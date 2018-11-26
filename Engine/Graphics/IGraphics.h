#pragma once

#include <d3d11.h>
#include <unordered_map>
#include <string>
#include "Texture.h"
#include "VertexTypes.h"

namespace Bat
{
	class Camera;
	class IModel;
	class Material;
	class IPipeline;

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

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual ID3D11Device* GetDevice() = 0;
		virtual ID3D11DeviceContext* GetDeviceContext() = 0;

		virtual IPipeline* GetPipeline( const std::string& name ) const = 0;

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