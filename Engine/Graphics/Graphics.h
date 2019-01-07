#pragma once

#include "D3DClass.h"

#include <DirectXColors.h>
#include "IPipeline.h"
#include "RenderTexture.h"
#include "ResourceManager.h"
#include "UI/BatUI.h"

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Bat
{
	class Window;
	class SceneGraph;
	class Camera;
	class IPostProcess;

	class Graphics
	{
	public:
		Graphics( Window& wnd );
		Graphics( const Graphics& src ) = delete;
		Graphics& operator=( const Graphics& src ) = delete;
		Graphics( Graphics&& donor ) = delete;
		Graphics& operator=( Graphics&& donor ) = delete;

		~Graphics();

		void Resize( int width, int height );

		int GetScreenWidth() const;
		int GetScreenHeight() const;

		Camera* GetActiveCamera() const { return m_pCamera; }
		void SetActiveCamera( Camera* pCamera ) { m_pCamera = pCamera; }

		SceneGraph* GetActiveScene() const { return m_pSceneGraph; }
		void SetActiveScene( SceneGraph* pSceneGraph ) { m_pSceneGraph = pSceneGraph; }

		BatUI& UI() { return m_UI; }
		const BatUI& UI() const { return m_UI; }

		IPipeline* GetPipeline( const std::string& name ) const;

		bool IsBloomEnabled() const { return m_bBloomEnabled; }
		void SetBloomEnabled( const bool enable ) { m_bBloomEnabled = enable; }

		void AddPostProcess( std::unique_ptr<IPostProcess> pPostProcess );
		void SetSkybox( Resource<Texture> pCubemap ) { m_pSkybox = pCubemap; }

		bool IsDepthStencilEnabled() const;
		void SetDepthStencilEnabled( bool enable );
		void EnableDepthStencil() { SetDepthStencilEnabled( true ); }
		void DisableDepthStencil() { SetDepthStencilEnabled( true ); }

		void BeginFrame();
		void EndFrame();

		void DrawText( std::wstring text, const Vec2& pos, const DirectX::FXMVECTOR col = DirectX::Colors::White );

		DirectX::XMMATRIX GetOrthoMatrix() const;

		ID3D11RenderTargetView* GetRenderTargetView() const;
		ID3D11DepthStencilView* GetDepthStencilView() const;
	private:
		ID3D11Device* GetDevice() const;
		ID3D11DeviceContext* GetDeviceContext() const;

		void RenderScene();

		void AddShader( const std::string& name, std::unique_ptr<IPipeline> pPipeline );
	private:
		D3DClass d3d;

		DirectX::XMMATRIX m_matOrtho;

		std::unordered_map<std::string, std::unique_ptr<IPipeline>> m_mapPipelines;
		std::vector<std::unique_ptr<IPostProcess>> m_PostProcesses;
		RenderTexture m_FrameBuffers[2];
		Resource<Texture> m_pSkybox = nullptr;

		int m_iScreenWidth = InitialScreenWidth;
		int m_iScreenHeight = InitialScreenHeight;

		SceneGraph* m_pSceneGraph = nullptr;
		Camera* m_pCamera = nullptr;
		
		bool m_bBloomEnabled = false;
		std::unique_ptr<IPostProcess> m_pBloomProcess;
	private:
		struct TextDrawCommand
		{
			TextDrawCommand( std::wstring text, Vec2 pos, DirectX::FXMVECTOR col )
				:
				text( std::move( text ) ),
				pos( pos ),
				col( col )
			{}

			std::wstring text;
			Vec2 pos;
			DirectX::FXMVECTOR col;
		};

		std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
		std::unique_ptr<DirectX::SpriteFont> m_pFont;
		std::vector<TextDrawCommand> m_TextDrawCommands;
	private: // UI
		BatUI m_UI;
	public:
		static constexpr bool	FullScreen = false;
		static constexpr int	VSyncEnabled = false;
		static constexpr float	ScreenFar = 1000.0f;
		static constexpr float	ScreenNear = 0.1f;
		static constexpr float	FOV = 90.0f;
		static constexpr float  FOVRadians = FOV * (DirectX::XM_PI / 180.0f);

		// only used when not in fullscreen
		static constexpr int	InitialScreenWidth = 800;
		static constexpr int	InitialScreenHeight = 600;
	};
}