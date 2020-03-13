#include "PCH.h"
#include "NavMesh.h"

#include <memory>
#include <Recast/Recast.h>
#include <Detour/DetourNavMesh.h>
#include <Detour/DetourNavMeshBuilder.h>
#include <Detour/DetourNavMeshQuery.h>
#include "Model.h"
#include "CoreEntityComponents.h"
#include "DebugDraw.h"

namespace Bat
{
#define MAKE_AUTO_TYPE( type, alloc_func, free_func ) \
	class type##Auto : public std::unique_ptr<type, decltype(&free_func)> \
	{ \
	public: \
		type##Auto() : std::unique_ptr<type, decltype(&free_func)>( nullptr, free_func ) {} \
		type##Auto( type* p ) : std::unique_ptr<type, decltype(&free_func)>( p, free_func ) {} \
	}; \
	type##Auto type##Make() { return type##Auto( alloc_func() ); }
	
	MAKE_AUTO_TYPE( rcHeightfield, rcAllocHeightfield, rcFreeHeightField );
	MAKE_AUTO_TYPE( rcCompactHeightfield, rcAllocCompactHeightfield, rcFreeCompactHeightfield );
	MAKE_AUTO_TYPE( rcContourSet, rcAllocContourSet, rcFreeContourSet );
	MAKE_AUTO_TYPE( rcPolyMesh, rcAllocPolyMesh, rcFreePolyMesh );
	MAKE_AUTO_TYPE( rcPolyMeshDetail, rcAllocPolyMeshDetail, rcFreePolyMeshDetail );
	MAKE_AUTO_TYPE( dtNavMesh, dtAllocNavMesh, dtFreeNavMesh );
	MAKE_AUTO_TYPE( dtNavMeshQuery, dtAllocNavMeshQuery, dtFreeNavMeshQuery );

	class BuildContext : public rcContext
	{
	public:
		virtual void doLog( const rcLogCategory category, const char* msg, const int len ) override
		{
			switch( category )
			{
			case RC_LOG_PROGRESS:
				BAT_TRACE( msg );
				break;
			case RC_LOG_WARNING:
				BAT_LOG( msg );
				break;
			case RC_LOG_ERROR:
				BAT_WARN( msg );
				break;
			}
		}
	};

	class NavMesh
	{
	public:
		NavMesh()
		{
			std::vector<Mesh*> meshes;
			std::vector<Mat3x4> transforms;
			for( Entity e : world )
			{
				if( e.Has<ModelComponent>() )
				{
					auto model = e.Get<ModelComponent>();
					auto t = e.Get<TransformComponent>();
					for( auto& mesh : model.GetMeshes() )
					{
						meshes.push_back( mesh.get() );
						transforms.push_back( t.LocalToWorldMatrix() );
					}
				}
			}

			BuildContext ctx;

			float agent_height = 0.7f;
			float agent_radius = 0.1f;
			float agent_max_climb = 0.1f;

			rcConfig cfg;
			cfg.cs = 0.1f;
			cfg.ch = 0.05f;
			cfg.borderSize = 0;
			cfg.walkableSlopeAngle = 45.0f;
			cfg.walkableHeight = (int)ceilf( agent_height / cfg.ch );
			cfg.walkableClimb = (int)floorf( agent_max_climb / cfg.ch );
			cfg.walkableRadius = (int)ceilf( agent_radius / cfg.cs );
			cfg.maxEdgeLen = (int)( 12.0f / cfg.cs );
			cfg.maxSimplificationError = 1.3f;
			cfg.minRegionArea = 2 * 2;
			cfg.mergeRegionArea = 20;
			cfg.maxVertsPerPoly = 6;
			cfg.detailSampleDist = 6.0f * cfg.cs;
			cfg.detailSampleMaxError = 1.0f * cfg.ch;

			cfg.bmin[0] = -100.0f; cfg.bmin[1] = -100.0f; cfg.bmin[2] = -100.0f;
			cfg.bmax[0] = 100.0f; cfg.bmax[1] = 100.0f; cfg.bmax[2] = 100.0f;
			rcCalcGridSize( cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height );

			rcHeightfieldAuto solid = rcHeightfieldMake();
			if( !rcCreateHeightfield( &ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch ) )
			{
				BAT_ERROR( "Could not create solid heightfield" );
				return;
			}

			std::vector<unsigned char> triareas;
			std::vector<float> verts;
			std::vector<int> tris;
			for( size_t mesh_index = 0; mesh_index < meshes.size(); mesh_index++ )
			{
				Mesh* mesh = meshes[mesh_index];
				const Mat3x4& transform = transforms[mesh_index];

				const auto* orig_verts = mesh->GetVertexData();
				const auto* orig_tris = mesh->GetIndexData();
				auto nv = (int)mesh->GetVertexCount();
				auto nt = (int)mesh->GetIndexCount() / 3;

				verts.clear();
				verts.reserve( nv * 3 );
				for( int i = 0; i < nv; i++ )
				{
					Vec3 vert = transform * orig_verts[i];
					verts.push_back( vert.x );
					verts.push_back( vert.y );
					verts.push_back( vert.z );
				}
				tris.clear();
				tris.reserve( nt * 3 );
				for( int i = 0; i < nt * 3; i++ )
				{
					tris.push_back( (int)orig_tris[i] );
				}

				triareas.clear();
				triareas.resize( nt );

				rcMarkWalkableTriangles( &ctx, cfg.walkableSlopeAngle, verts.data(), nv, tris.data(), nt, triareas.data() );
				if( !rcRasterizeTriangles( &ctx, verts.data(), nv, tris.data(), triareas.data(), nt, *solid ) )
				{
					BAT_ERROR( "Could not rasterize triangles" );
					return;
				}
			}

			rcFilterLowHangingWalkableObstacles( &ctx, cfg.walkableClimb, *solid );
			rcFilterLedgeSpans( &ctx, cfg.walkableHeight, cfg.walkableClimb, *solid );
			rcFilterWalkableLowHeightSpans( &ctx, cfg.walkableHeight, *solid );

			rcCompactHeightfieldAuto chf = rcCompactHeightfieldMake();
			if( !rcBuildCompactHeightfield( &ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf ) )
			{
				BAT_ERROR( "Could not build compact data" );
				return;
			}

			if( !rcErodeWalkableArea( &ctx, cfg.walkableRadius, *chf ) )
			{
				BAT_ERROR( "Could not erode" );
				return;
			}

			if( !rcBuildDistanceField( &ctx, *chf ) )
			{
				BAT_ERROR( "Could not build distance field" );
				return;
			}

			if( !rcBuildRegions( &ctx, *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea ) )
			{
				BAT_ERROR( "Could not build watershed regions" );
				return;
			}

			rcContourSetAuto cset = rcContourSetMake();
			if( !rcBuildContours( &ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset ) )
			{
				BAT_ERROR( "Could not create contours" );
				return;
			}

			m_PolyMesh = rcPolyMeshMake();
			if( !rcBuildPolyMesh( &ctx, *cset, cfg.maxVertsPerPoly, *m_PolyMesh ) )
			{
				BAT_ERROR( "Could not triangulate contours" );
				return;
			}

			m_PolyMeshDetail = rcPolyMeshDetailMake();
			if( !rcBuildPolyMeshDetail( &ctx, *m_PolyMesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *m_PolyMeshDetail ) )
			{
				BAT_ERROR( "Could not build detail mesh" );
				return;
			}

			for( int i = 0; i < m_PolyMesh->npolys; i++ )
			{
				if( m_PolyMesh->areas[i] == RC_WALKABLE_AREA )
				{
					m_PolyMesh->flags[i] = 1; // TODO: Update to proper flag
				}
			}

			dtNavMeshCreateParams params;
			params.verts = m_PolyMesh->verts;
			params.vertCount = m_PolyMesh->nverts;
			params.polys = m_PolyMesh->polys;
			params.polyFlags = m_PolyMesh->flags;
			params.polyAreas = m_PolyMesh->areas;
			params.polyCount = m_PolyMesh->npolys;
			params.nvp = m_PolyMesh->nvp;
			params.detailMeshes = m_PolyMeshDetail->meshes;
			params.detailVerts = m_PolyMeshDetail->verts;
			params.detailVertsCount = m_PolyMeshDetail->nverts;
			params.detailTris = m_PolyMeshDetail->tris;
			params.detailTriCount = m_PolyMeshDetail->ntris;
			params.offMeshConVerts = nullptr;
			params.offMeshConRad = nullptr;
			params.offMeshConFlags = nullptr;
			params.offMeshConAreas = nullptr;
			params.offMeshConDir = nullptr;
			params.offMeshConUserID = nullptr;
			params.offMeshConCount = 0;
			params.userId = 0;
			params.tileX = 0;
			params.tileY = 0;
			params.tileLayer = 0;
			rcVcopy( params.bmin, m_PolyMesh->bmin );
			rcVcopy( params.bmax, m_PolyMesh->bmax );
			params.walkableHeight = agent_height;
			params.walkableRadius = agent_radius;
			params.walkableClimb = agent_max_climb;
			params.cs = cfg.cs;
			params.ch = cfg.ch;
			params.buildBvTree = false;

			unsigned char* navmesh_data;
			int navmesh_data_size;
			if( !dtCreateNavMeshData( &params, &navmesh_data, &navmesh_data_size ) )
			{
				BAT_ERROR( "Could not build nav mesh data" );
				return;
			}

			m_NavMesh = dtNavMeshMake();
			dtStatus status = m_NavMesh->init( navmesh_data, navmesh_data_size, DT_TILE_FREE_DATA );
			if( dtStatusFailed( status ) )
			{
				dtFree( navmesh_data );
				BAT_ERROR( "Could not initialize nav mesh" );
				return;
			}

			m_Query = dtNavMeshQueryMake();
			m_Query->init( m_NavMesh.get(), 4096 );
		}

		std::vector<Vec3> GetPath( const Vec3& start, const Vec3& end )
		{
			auto* s = reinterpret_cast<const float*>( &start );
			auto* e = reinterpret_cast<const float*>( &end );
			float poly_ext[3] = { 2.0f, 4.0f, 2.0f };

			dtQueryFilter filter;
			filter.setIncludeFlags( 0xFFFF );
			filter.setExcludeFlags( 0 );
			filter.setAreaCost( RC_WALKABLE_AREA, 1.0f );

			dtPolyRef pstart, pend;
			m_Query->findNearestPoly( s, poly_ext, &filter, &pstart, nullptr );
			m_Query->findNearestPoly( e, poly_ext, &filter, &pend, nullptr );

			dtPolyRef poly_path[256];
			int poly_path_count;
			m_Query->findPath( pstart, pend, s, e, &filter, poly_path, &poly_path_count, 256 );

			Vec3 steer_path[256];
			int steer_path_count;
			m_Query->findStraightPath( s, e, poly_path, poly_path_count, reinterpret_cast<float*>( steer_path ), nullptr, nullptr, &steer_path_count, 256 );

			std::vector<Vec3> path;
			path.reserve( steer_path_count );
			for( int i = 0; i < steer_path_count; i++ )
			{
				path.push_back( steer_path[i] );
			}

			return path;
		}

		Vec3 MoveAlongSurface( const Vec3& start, const Vec3& end )
		{
			auto* s = reinterpret_cast<const float*>( &start );
			auto* e = reinterpret_cast<const float*>( &end );
			float poly_ext[3] = { 2.0f, 4.0f, 2.0f };

			dtQueryFilter filter;
			filter.setIncludeFlags( RC_WALKABLE_AREA );
			filter.setExcludeFlags( 0 );
			filter.setAreaCost( RC_WALKABLE_AREA, 1.0f );

			dtPolyRef pstart;
			m_Query->findNearestPoly( s, poly_ext, &filter, &pstart, nullptr );

			float result[3];
			dtPolyRef visited[16];
			int visited_count;
			m_Query->moveAlongSurface( pstart, s, e, &filter, result, visited, &visited_count, 16 );

			return { result[0], result[1], result[2] };
		}

		void Draw()
		{
			rcPolyMesh& mesh = *m_PolyMesh;

			const int nvp = mesh.nvp;
			const float cs = mesh.cs;
			const float ch = mesh.ch;
			const Vec3 orig = { mesh.bmin[0], mesh.bmin[1], mesh.bmin[2] };

			for( int i = 0; i < mesh.npolys; ++i )
			{
				const unsigned short* p = &mesh.polys[i * nvp * 2];
				for( int j = 0; j < nvp; ++j )
				{
					if( p[j] == RC_MESH_NULL_IDX ) break;
					if( p[nvp + j] & 0x8000 ) continue;
					const int nj = ( j + 1 >= nvp || p[j + 1] == RC_MESH_NULL_IDX ) ? 0 : j + 1;
					const int vi[2] = { p[j], p[nj] };

					const unsigned short* t0 = &mesh.verts[p[j] * 3];
					Vec3 v0 = orig + Vec3{ t0[0] * cs, ( t0[1] + 1 ) * ch + 0.1f, t0[2] * cs };
					const unsigned short* t1 = &mesh.verts[p[nj] * 3];
					Vec3 v1 = orig + Vec3{ t1[0] * cs, ( t1[1] + 1 ) * ch + 0.1f, t1[2] * cs };

					DebugDraw::Line( v0, v1, Colours::Green );
				}
			}

			for( int i = 0; i < mesh.npolys; ++i )
			{
				const unsigned short* p = &mesh.polys[i * nvp * 2];
				for( int j = 0; j < nvp; ++j )
				{
					if( p[j] == RC_MESH_NULL_IDX ) break;
					if( ( p[nvp + j] & 0x8000 ) == 0 ) continue;
					const int nj = ( j + 1 >= nvp || p[j + 1] == RC_MESH_NULL_IDX ) ? 0 : j + 1;

					Colour col = Colours::Blue;
					if( ( p[nvp + j] & 0xf ) != 0xf )
					{
						col = Colours::White;
					}

					const unsigned short* t0 = &mesh.verts[p[j] * 3];
					Vec3 v0 = orig + Vec3{ t0[0] * cs, ( t0[1] + 1 ) * ch + 0.1f, t0[2] * cs };
					const unsigned short* t1 = &mesh.verts[p[nj] * 3];
					Vec3 v1 = orig + Vec3{ t1[0] * cs, ( t1[1] + 1 ) * ch + 0.1f, t1[2] * cs };

					DebugDraw::Line( v0, v1, col );
				}
			}
		}
	private:
		rcPolyMeshAuto m_PolyMesh;
		rcPolyMeshDetailAuto m_PolyMeshDetail;
		dtNavMeshAuto m_NavMesh;
		dtNavMeshQueryAuto m_Query;
	};

	NavMeshSystem::NavMeshSystem()
	{
	}

	NavMeshSystem::~NavMeshSystem()
	{
	}

	NavMeshHandle_t NavMeshSystem::Bake()
	{
		auto handle = (NavMeshHandle_t)m_NavMeshes.size();
		m_NavMeshes.emplace_back();
		return handle;
	}

	void NavMeshSystem::Update( float dt )
	{
		for( Entity e : world )
		{
			if( e.Has<NavMeshAgent>() )
			{
				auto& agent = e.Get<NavMeshAgent>();
				auto& t = e.Get<TransformComponent>();

				NavMesh& navmesh = m_NavMeshes[agent.navmesh];

				if( agent.go_to_target )
				{
					if( agent.path.empty() )
					{
						agent.path = navmesh.GetPath( t.GetPosition(), agent.target );
					}

					if( agent.path.empty() )
					{
						continue;
					}

					Vec3 start = t.GetPosition();
					Vec3 end = agent.path[agent.next_path_point];
					Vec3 delta = end - start;
					float len = delta.Length();

					if( len <= 0.01f )
					{
						agent.next_path_point++;
						if( agent.next_path_point == agent.path.size() )
						{
							agent.go_to_target = false;
							agent.path.clear();
						}
					}
					else
					{
						delta /= len;
					}

					Vec3 result = navmesh.MoveAlongSurface( start, start + delta * std::min( len, 0.05f ) );
					t.SetPosition( result );
				}
			}
		}

		if( !m_NavMeshes.empty() )
		{
			NavMesh& navmesh = m_NavMeshes.back();
			//navmesh.Draw();
		}
	}
}
