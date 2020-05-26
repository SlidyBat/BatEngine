#pragma once

#include "SceneRenderPass.h"
#include "../ShadowPipeline.h"
#include "../ShaderManager.h"

namespace Bat
{
	class ShadowPass : public BaseRenderPass
	{
	public:
		static constexpr int SHADOW_MAP_SIZE = 2048;
		static constexpr int NUM_CASCADES = 3;

		ShadowPass()
		{
			m_pShadowMaps = std::unique_ptr<IDepthStencil>( gpu->CreateDepthStencil( SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, TEX_FORMAT_R24G8_TYPELESS, MAX_SHADOW_SOURCES ) );
		}

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			m_pContext = pContext;

			ComparisonFunc original_comp_func = pContext->GetDepthComparisonFunc();

			pContext->SetRenderTarget( nullptr );
			pContext->SetDepthEnabled( true );
			pContext->SetDepthWriteEnabled( true );
			pContext->SetDepthComparisonFunc( ComparisonFunc::GREATER );
			pContext->SetDepthClipEnabled( false );
			pContext->SetCullMode( CullMode::NONE );
			pContext->SetBlendingEnabled( false );
			pContext->UnbindTextureSlot( PS_TEX_SHADOWMAPS );

			Viewport vp;
			vp.width = SHADOW_MAP_SIZE;
			vp.height = SHADOW_MAP_SIZE;
			vp.min_depth = 0.0f;
			vp.max_depth = 1.0f;
			vp.top_left = { 0.0f, 0.0f };
			pContext->PushViewport( vp );

			IDepthStencil* original_depth = pContext->GetDepthStencil();
			CB_ShadowMatrices transforms;
			size_t shadow_map_counter = 0;

			for( Entity e : world )
			{
				if( e.Has<LightComponent>() )
				{
					auto& t = e.Get<TransformComponent>();
					auto& light = e.Get<LightComponent>();
					light.SetShadowIndex( INVALID_SHADOW_MAP_INDEX );
					
					if( !light.IsEnabled() || light.GetType() == LightType::POINT )
					{
						continue;
					}
					
					if( !light.HasFlag( LightFlags::EMIT_SHADOWS ) )
					{
						continue;
					}

					switch( light.GetType() )
					{
					case LightType::DIRECTIONAL:
					{
						Mat4 rot = Mat4::RotateDeg( t.GetRotation() );
						Vec3 up = Mat4::TransformNormal( rot, { 0.0f, 1.0f, 0.0f } );
						Vec3 to = Mat4::TransformNormal( rot, { 0.0f, 0.0f, 1.0f } );

						Vec3 frustum_corners[8];
						camera.CalculateFrustumCorners( frustum_corners );

						light.SetShadowIndex( shadow_map_counter );

						float splits[NUM_CASCADES + 1] = {
							0.0f,
							0.01f,
							0.1f,
							1.0f
						};

						for( int cascade = 0; cascade < NUM_CASCADES; cascade++ )
						{
							Vec3 cascade_centre = { 0.0f, 0.0f, 0.0f };
							Vec3 cascade_corners[8];
							for( int i = 0; i < 8; i++ )
							{
								// First 4 corners are near plane, next 4 are the adjacent far plane corners
								// Lerp between the 2 corners to generate a split

								Vec3 near_corner = frustum_corners[i % 4];
								Vec3 far_corner = frustum_corners[(i % 4) + 4];

								cascade_corners[i] = Vec3::Lerp( near_corner, far_corner, splits[cascade + i/4] );
								cascade_centre += cascade_corners[i];
							}
							cascade_centre /= 8.0f;

							float radius = 0.0f;
							for( int i = 0; i < 8; i++ )
							{
								float dist = ( cascade_corners[i] - cascade_centre ).Length();
								radius = std::max( dist, radius );
							}

							Vec3 maxs = { radius, radius, radius };
							Vec3 mins = -maxs;

							// Snap to texel grid
							Vec3 extents = maxs - mins;
							Vec3 texel_size = extents / SHADOW_MAP_SIZE;
							mins = Vec3::Floor( mins / texel_size ) * texel_size;

							Vec3 cascade_camera_pos = cascade_centre - to * maxs.z;

							Mat4 view = Mat4::LookAt( cascade_camera_pos, cascade_centre, up );
							Mat4 proj = Mat4::OrthoOffCentre( mins.x, maxs.x, mins.y, maxs.y, extents.z, 0.0f );

							m_matLightViewProj = view * proj;
							m_bLightCull = false;
							transforms.shadow[shadow_map_counter++] = m_matLightViewProj;

							pContext->ClearDepthStencil( m_pShadowMaps.get(), CLEAR_FLAG_DEPTH, 0.0f, 0, light.GetShadowIndex() + cascade );
							pContext->SetDepthStencil( m_pShadowMaps.get(), light.GetShadowIndex() + cascade );

							Traverse();
						}
						
						break;
					}
					case LightType::SPOT:
					{
						Camera spot_cam( t.GetPosition(),
							t.GetRotation(),
							Math::RadToDeg( light.GetSpotlightAngle() * 2 ),
							1.0f,
							light.GetRange(),
							0.1f);

						Mat4 view = spot_cam.GetViewMatrix();
						Mat4 proj = spot_cam.GetProjectionMatrix();
						m_matLightViewProj = view * proj;

						// View frustum culling
						Vec3 light_corners[8];
						Frustum::CalculateCorners( m_matLightViewProj, light_corners );
						AABB light_aabb( light_corners, 8 );
						if( !camera.GetFrustum().IsBoxInside( light_aabb.mins, light_aabb.maxs ) )
						{
							continue;
						}

						m_LightFrustum = Frustum( m_matLightViewProj );
						m_bLightCull = true;

						light.SetShadowIndex( shadow_map_counter++ );
						transforms.shadow[light.GetShadowIndex()] = m_matLightViewProj;

						pContext->ClearDepthStencil( m_pShadowMaps.get(), CLEAR_FLAG_DEPTH, 0.0f, 0, light.GetShadowIndex() );
						pContext->SetDepthStencil( m_pShadowMaps.get(), light.GetShadowIndex() );

						Traverse();

						break;
					}
					}
				}
			}

			pContext->SetCullMode( CullMode::BACK );
			pContext->SetDepthClipEnabled( true );
			pContext->SetDepthComparisonFunc( original_comp_func );
			pContext->SetDepthStencil( original_depth );
			pContext->PopViewport();

			m_cbufShadowMatrices.Update( pContext, transforms );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufShadowMatrices, PS_CBUF_SHADOWMATRICES );
			pContext->BindTexture( m_pShadowMaps.get(), PS_TEX_SHADOWMAPS );
		}
	private:
		virtual void Visit( const Mat3x4& transform, Entity e ) override
		{
			if( e.Has<AnimationComponent>() )
			{
				auto& anim = e.Get<AnimationComponent>();

				Mat4* cbuf = m_cbufBones.Lock( m_pContext )->bone_transforms;

				GetMatrixPalette( cbuf, anim.bones, anim.current_pose );

				m_cbufBones.Unlock( m_pContext );

				m_pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufBones, VS_CBUF_BONES );
			}
			if( e.Has<ModelComponent>() )
			{
				auto& model = e.Get<ModelComponent>();

				if( model.HasRenderFlag( RenderFlags::NO_SHADOW ) )
				{
					return;
				}

				const auto w = Mat4( transform );

				auto& meshes = model.GetMeshes();
				for( auto& pMesh : meshes )
				{
					if( pMesh->HasRenderFlag( RenderFlags::NO_SHADOW ) )
					{
						continue;
					}

					if( pMesh->GetMaterial().GetAlphaMode() == AlphaMode::BLEND )
					{
						continue;
					}

					if( m_bLightCull && !MeshInLightFrustum( pMesh.get(), w ) )
					{
						continue;
					}

					auto pPipeline = ShaderManager::GetPipeline<ShadowPipeline>();
					pPipeline->Render( m_pContext, *pMesh, m_matLightViewProj, w );
				}
			}
		}

		void GetMatrixPalette( Mat4* out, const std::vector<BoneData>& bones, SkeletonPose pose ) const
		{
			ASSERT( bones.size() <= MAX_BONES, "Too many bones!" );

			auto model_pose = SkeletonPose::ToModelSpace( std::move( pose ) );
			SkeletonPose::ToMatrixPalette( model_pose, bones, out );
		}

		bool MeshInLightFrustum( Mesh* pMesh, Mat4 transform )
		{
			const AABB& aabb = pMesh->GetAABB();
			AABB world_aabb = aabb.Transform( transform );

			// View frustum culling on mesh level
			if( !m_LightFrustum.IsBoxInside( world_aabb.mins, world_aabb.maxs ) )
			{
				return false;
			}

			return true;
		}
	private:
		IGPUContext* m_pContext;
		Mat4 m_matLightViewProj;
		Frustum m_LightFrustum;
		bool m_bLightCull = false;
		struct CB_Bones
		{
			Mat4 bone_transforms[MAX_BONES];
		};
		ConstantBuffer<CB_Bones> m_cbufBones;
		struct CB_ShadowMatrices
		{
			Mat4 shadow[MAX_SHADOW_SOURCES];
		};
		ConstantBuffer<CB_ShadowMatrices> m_cbufShadowMatrices;
		std::unique_ptr<IDepthStencil> m_pShadowMaps;
	};
}