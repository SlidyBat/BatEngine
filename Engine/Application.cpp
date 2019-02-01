#include "PCH.h"
#include "Application.h"

#include "Common.h"
#include "Graphics.h"
#include "Window.h"
#include "SceneLoader.h"
#include "FileWatchdog.h"

#include "WindowEvents.h"
#include "KeyboardEvents.h"
#include "MouseEvents.h"
#include "TexturePipeline.h"
#include "Globals.h"

#include <imgui.h>

namespace Bat
{
	Application::Application( Graphics& gfx, Window& wnd )
		:
		gfx( gfx ),
		wnd( wnd )
	{
		screentex = Texture::FromColour( nullptr, wnd.GetWidth(), wnd.GetHeight(), D3D11_USAGE_DYNAMIC );
		pixels = new Colour[wnd.GetWidth() * wnd.GetHeight()];

		wnd.OnEventDispatched<WindowResizeEvent>( [this]( const WindowResizeEvent& e )
		{
			this->screentex = Texture::FromColour( nullptr, e.width, e.height, D3D11_USAGE_DYNAMIC );
			delete[] pixels;
			pixels = new Colour[e.width * e.height];
		} );

		const std::vector<Vec3> vertices = {
			{ -1.0f,  1.0f, 0.5f },
			{  1.0f,  1.0f, 0.5f },
			{  1.0f, -1.0f, 0.5f },
			{ -1.0f, -1.0f, 0.5f },
		};

		const std::vector<Vec2> uvs = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};

		const std::vector<int> indices = { 0, 1, 2, 2, 3, 0 };

		MeshParameters meshparams;
		meshparams.position = vertices;
		meshparams.uv = uvs;
		screenquad.SetData( meshparams );
		screenquad.SetIndices( indices );

		// set screen colour
		for( size_t y = 0; y < screentex.GetHeight(); y++ )
		{
			for( size_t x = 0; x < screentex.GetWidth(); x++ )
			{
				pixels[y*screentex.GetWidth() + x] = Colour( 0, 255, 0 );
			}
		}

		wnd.input.OnEventDispatched<KeyPressedEvent>( []( const KeyPressedEvent& e )
		{
			if( e.key == VK_OEM_3 )
			{
				BAT_LOG( "Toggling console" );
				g_Console.SetVisible( !g_Console.IsVisible() );
			}
		} );

		wnd.input.OnEventDispatched<MouseScrolledEvent>( []( const MouseScrolledEvent& e )
		{
			static float accum = 0.0f;
			accum += e.delta;
			BAT_LOG( "Accumulated scroll: {}", accum );
		} );

		g_Console.AddCommand( "test_command", []( const CommandArgs_t& args )
		{
			BAT_LOG( "You wrote: '{}'", Bat::JoinStrings( args ) );
		} );
	}

	Application::~Application()
	{
		delete[] pixels;
	}

	void Application::OnUpdate( float deltatime )
	{
		elapsed_time += deltatime;
		fps_counter += 1;
		if( elapsed_time > 1.0f )
		{
			fps_string = "FPS: " + std::to_string( fps_counter );
			fps_counter = 0;
			elapsed_time -= 1.0f;
		}

		screentex.UpdatePixels( pixels, screentex.GetWidth() * sizeof( Colour ) );
	}

	void Application::OnRender()
	{
		gfx.DisableDepthStencil();

		TexturePipelineParameters params( DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), screentex.GetTextureView() );

		auto texture_pipeline = gfx.GetPipeline( "texture" );
		screenquad.Bind( texture_pipeline );
		texture_pipeline->BindParameters( params );
		texture_pipeline->RenderIndexed( (UINT)screenquad.GetIndexCount() );

		gfx.DrawText( Bat::StringToWide( fps_string ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );
	}
}
