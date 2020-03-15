#include "PCH.h"
#include "DebugDraw.h"

#include "IGPUDevice.h"
#include "MeshBuilder.h"
#include "ShaderManager.h"
#include "ColourPipeline.h"
#include "SpriteFont.h"

namespace Bat
{
	struct DebugRect
	{
		Vei2 a;
		Vei2 b;
		Colour col;
		float thickness;
	};
	static std::vector<DebugRect> g_DebugRects;

	struct DebugText
	{
		std::string str;
		Vei2 pos;
		Colour col;
	};
	static std::vector<DebugText> g_DebugTexts;

	struct DebugBox
	{
		Vec3 a;
		Vec3 b;
		Colour col;
	};
	static std::vector<DebugBox> g_DebugBoxes;

	struct DebugLine
	{
		Vec3 a;
		Vec3 b;
		Colour col;
	};
	static std::vector<DebugLine> g_DebugLines;

	static void InitDebugDrawState( IGPUContext* pContext )
	{
		pContext->SetDepthEnabled( false );
		pContext->SetDepthWriteEnabled( false );
		pContext->SetCullMode( CullMode::NONE );
		pContext->SetRenderTarget( gpu->GetBackbuffer() );
	}
	static void DrawRectangle( const DebugRect& rect )
	{
		IGPUContext* pContext = gpu->GetContext();

		const auto left = (float)std::min( rect.a.x, rect.b.x );
		const auto right = (float)std::max( rect.a.x, rect.b.x );
		const auto top = (float)std::min( rect.a.y, rect.b.y );
		const auto bottom = (float)std::max( rect.a.y, rect.b.y );
		const Vec4 col_vec = rect.col.AsVector();
		const float thickness = rect.thickness;

		// 4 sides, each with 2 triangles makes 8 triangles total
		const size_t triangle_count = 8;
		MeshBuilder builder( triangle_count );
		// Inside corners
		builder.Position( left, top );     builder.Colour( col_vec );
		builder.Position( right, top );    builder.Colour( col_vec );
		builder.Position( right, bottom ); builder.Colour( col_vec );
		builder.Position( left, bottom );  builder.Colour( col_vec );
		// Outside corners
		builder.Position( left - thickness, top - thickness );     builder.Colour( col_vec );
		builder.Position( right + thickness, top - thickness );    builder.Colour( col_vec );
		builder.Position( right + thickness, bottom + thickness ); builder.Colour( col_vec );
		builder.Position( left - thickness, bottom + thickness );  builder.Colour( col_vec );

		builder.Triangle( 0, 4, 5 );
		builder.Triangle( 0, 5, 1 );
		builder.Triangle( 1, 5, 6 );
		builder.Triangle( 1, 6, 2 );
		builder.Triangle( 2, 6, 7 );
		builder.Triangle( 2, 7, 3 );
		builder.Triangle( 3, 7, 4 );
		builder.Triangle( 3, 4, 0 );

		auto pPipeline = ShaderManager::GetPipeline<ColourPipeline>();
		pPipeline->Render( pContext, builder.Build(), Camera::ScreenOrtho(), Mat4::Identity() );
	}
	static void DrawBox( const Camera& camera, const DebugBox& box )
	{
		AABB aabb( box.a, box.b );
		Vec3 box_points[8];
		aabb.GetPoints( box_points );
		Vec4 colour = box.col.AsVector();

		const size_t vertex_count = 8;
		const size_t index_count = 8 * 3;
		MeshBuilder builder( vertex_count, index_count, PrimitiveTopology::LINELIST );
		for( int i = 0; i < 8; i++ )
		{
			builder.Position( box_points[i] );
			builder.Colour( colour );
		}

		builder.Line( 0, 1 );
		builder.Line( 1, 2 );
		builder.Line( 2, 3 );
		builder.Line( 3, 0 );
		builder.Line( 4, 5 );
		builder.Line( 5, 6 );
		builder.Line( 6, 7 );
		builder.Line( 7, 4 );
		builder.Line( 4, 2 );
		builder.Line( 5, 1 );
		builder.Line( 6, 0 );
		builder.Line( 7, 3 );

		IGPUContext* pContext = gpu->GetContext();
		auto pPipeline = ShaderManager::GetPipeline<ColourPipeline>();
		pPipeline->Render( pContext, builder.Build(), camera, Mat4::Identity() );
	}
	static void DrawLines( const Camera& camera, const std::vector<DebugLine>& lines )
	{
		const size_t vertex_count = lines.size() * 2;
		const size_t index_count = 0;
		MeshBuilder builder( vertex_count, index_count, PrimitiveTopology::LINELIST );
		for( const DebugLine& line : lines  )
		{
			Vec4 col = line.col.AsVector();
			builder.Position( line.a ); builder.Colour( col );
			builder.Position( line.b ); builder.Colour( col );
		}

		IGPUContext* pContext = gpu->GetContext();
		auto pPipeline = ShaderManager::GetPipeline<ColourPipeline>();
		pPipeline->Render( pContext, builder.Build(), camera, Mat4::Identity() );
	}

	void DebugDraw::Rectangle( const Vei2& a, const Vei2& b, const Colour& col, float thickness )
	{
		DebugRect rect;
		rect.a = a;
		rect.b = b;
		rect.col = col;
		rect.thickness = thickness;
		g_DebugRects.push_back( rect );
	}
	void DebugDraw::Box( const Vec3& a, const Vec3& b, const Colour& col )
	{
		DebugBox box;
		box.a = a;
		box.b = b;
		box.col = col;
		g_DebugBoxes.push_back( box );
	}
	void DebugDraw::Line( const Vec3& a, const Vec3& b, const Colour& col )
	{
		DebugLine line;
		line.a = a;
		line.b = b;
		line.col = col;
		g_DebugLines.push_back( line );
	}
	void DebugDraw::Text( const std::string& str, const Vei2& pos, const Colour& col )
	{
		DebugText text;
		text.str = str;
		text.pos = pos;
		text.col = col;
		g_DebugTexts.push_back( std::move( text ) );
	}
	void DebugDraw::Flush( const Camera& camera )
	{
		IGPUContext* pContext = gpu->GetContext();
		pContext->BeginEvent( "debug_draw" );

		InitDebugDrawState( pContext );

		for( const DebugRect& rect : g_DebugRects )
		{
			DrawRectangle( rect );
		}
		g_DebugRects.clear();

		for( const DebugBox& box : g_DebugBoxes )
		{
			DrawBox( camera, box );
		}
		g_DebugBoxes.clear();

		if( !g_DebugLines.empty() )
		{
			DrawLines( camera, g_DebugLines );
			g_DebugLines.clear();
		}

		static DirectX::SpriteFont font( (ID3D11Device*)gpu->GetImpl(), L"Assets/Fonts/consolas.spritefont" );
		if( !g_DebugTexts.empty() )
		{
			static DirectX::SpriteBatch batch( (ID3D11DeviceContext*)gpu->GetContext()->GetImpl() );
			batch.Begin();
			for( const DebugText& text : g_DebugTexts )
			{
				Vec2 pos = { (float)text.pos.x, (float)text.pos.y };

				wchar_t wstr[256];
				Bat::StringToWide( text.str.c_str(), wstr, sizeof( wstr ) );
				font.DrawString( &batch, wstr, pos, text.col.AsVector() );
			}
			batch.End();

			g_DebugTexts.clear();
		}

		pContext->EndEvent();
	}
}
