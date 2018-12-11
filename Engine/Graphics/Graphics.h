#pragma once

#include "PCH.h"
#include "D3DClass.h"

#include "IGraphics.h"
#include "GDIPManager.h"
#include "Colour.h"
#include "Texture.h"
#include "VertexTypes.h"
#include "Camera.h"
#include "RenderTexture.h"

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Bat
{
	class Window;
	class Model;

	class Graphics : public IGraphics
	{
	public:
		Graphics( Window& wnd );
		Graphics( const Graphics& src ) = delete;
		Graphics& operator=( const Graphics& src ) = delete;
		Graphics( Graphics&& donor ) = delete;
		Graphics& operator=( Graphics&& donor ) = delete;

		~Graphics() override;

		void Resize( int width, int height );

		virtual int GetScreenWidth() const override;
		virtual int GetScreenHeight() const override;

		virtual IPipeline* GetPipeline( const std::string& name ) const override;

		virtual void AddPostProcess( std::unique_ptr<IPostProcess> pPostProcess ) override;
		virtual void SetSkybox( Texture* pCubemap ) override { m_pSkybox = pCubemap; }

		virtual bool IsDepthStencilEnabled() const override;
		virtual void SetDepthStencilEnabled( bool enable ) override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void DrawText( std::wstring text, const Vec2& pos, const DirectX::FXMVECTOR col = DirectX::Colors::White ) override;

		virtual DirectX::XMMATRIX GetOrthoMatrix() const;

		virtual ID3D11Device* GetDevice() const override;
		virtual ID3D11DeviceContext* GetDeviceContext() const override;
		virtual ID3D11RenderTargetView* GetRenderTargetView() const override;
		virtual ID3D11DepthStencilView* GetDepthStencilView() const override;
	private:
		void AddShader( const std::string& name, std::unique_ptr<IPipeline> pPipeline );
	private:
		D3DClass d3d;

		DirectX::XMMATRIX m_matOrtho;

		std::unordered_map<std::string, std::unique_ptr<IPipeline>> m_mapPipelines;
		std::vector<std::unique_ptr<IPostProcess>> m_PostProcesses;
		RenderTexture m_FrameBuffers[2];
		Texture* m_pSkybox = nullptr;

		int m_iScreenWidth = InitialScreenWidth;
		int m_iScreenHeight = InitialScreenHeight;

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