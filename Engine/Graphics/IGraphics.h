#pragma once

#include <d3d11.h>
#include <unordered_map>
#include <string>
#include "Texture.h"
#include "VertexTypes.h"
#include <DirectXColors.h>

namespace Bat
{
	class Camera;
	class IModel;
	class Material;
	class IPipeline;
	class IPostProcess;
	class Vec2;

	class IGraphics;
	extern IGraphics* g_pGfx;

	class IGraphics
	{
	public:
		static void RegisterGraphics( IGraphics* pGraphics )
		{
			g_pGfx = pGraphics;
		}
	public:
		virtual ~IGraphics() = default;

		virtual int GetScreenWidth() const = 0;
		virtual int GetScreenHeight() const = 0;

		virtual void SetSkybox( Texture* pCubemap ) = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		
		virtual void DrawText( std::wstring text, const Vec2& pos, const DirectX::FXMVECTOR col = DirectX::Colors::White ) = 0;

		virtual ID3D11Device* GetDevice() const = 0;
		virtual ID3D11DeviceContext* GetDeviceContext() const = 0;
		virtual ID3D11RenderTargetView* GetRenderTargetView() const = 0;
		virtual ID3D11DepthStencilView* GetDepthStencilView() const = 0;

		virtual DirectX::XMMATRIX GetOrthoMatrix() const = 0;

		virtual IPipeline* GetPipeline( const std::string& name ) const = 0;

		virtual void AddPostProcess( std::unique_ptr<IPostProcess> pPostProcess ) = 0;

		virtual bool IsDepthStencilEnabled() const = 0;
		virtual void SetDepthStencilEnabled( bool enable ) = 0;
		void EnableDepthStencil() { SetDepthStencilEnabled( true ); }
		void DisableDepthStencil() { SetDepthStencilEnabled( true ); }

		Camera* GetCamera() const
		{
			return m_pCamera;
		}
		void SetCamera(Camera* pCamera)
		{
			m_pCamera = pCamera;
		}
	protected:
		Camera* m_pCamera = nullptr;
	};
}