#include "BatWinAPI.h"
#include "Window.h"
#include "Graphics.h"
#include "MarioTestScene.h"
#include "ColourTestScene.h"
#include "ModelTestScene.h"
#include "COMException.h"
#include <string>
#include "FrameTimer.h"
#include "Globals.h"
#include "StringLib.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>

using namespace Bat;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow )
{
	try
	{
		COM_ERROR_IF_FAILED( CoInitialize( NULL ) );

		Window wnd( { 50, 50 }, Graphics::ScreenWidth, Graphics::ScreenHeight, "Bat Engine", Graphics::FullScreen );
		Graphics gfx( wnd );
		//IGraphics::RegisterGraphics( gfx );

		FrameTimer ft;
		auto sb = std::make_unique<DirectX::SpriteBatch>( gfx.GetDeviceContext() );
		auto font = std::make_unique<DirectX::SpriteFont>( gfx.GetDevice(), L"Assets/Fonts/consolas.spritefont" );

		IScene* pScene = new ModelTestScene( wnd );
		while( wnd.ProcessMessage() )
		{
			static int fpsCounter = 0;
			static float elapsedTime = 0.0f;
			static std::string fpsString = "FPS: 0";

			float dt = ft.Mark();
			elapsedTime += dt;
			fpsCounter += 1;
			if( elapsedTime > 1.0f )
			{
				fpsString = "FPS: " + std::to_string( fpsCounter );
				fpsCounter = 0;
				elapsedTime -= 1.0f;
			}
			g_pGlobals->elapsed_time += dt;

			pScene->OnUpdate( dt );

			gfx.BeginFrame();
			
			pScene->OnRender();

			sb->Begin();
			font->DrawString( sb.get(), Bat::StringToWide( fpsString ).c_str(), DirectX::XMFLOAT2{ 15.0f, 15.0f } );
			sb->End();

			gfx.EndFrame();
		}
		delete pScene;
	}
	catch( const std::exception& e )
	{
		MessageBox( NULL, e.what(), "Error", MB_ICONWARNING );
	}

	return 0;
}