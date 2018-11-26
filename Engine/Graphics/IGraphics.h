#pragma once

#include <d3d11.h>
#include <unordered_map>
#include <string>
#include "Texture.h"
#include "VertexTypes.h"

namespace Bat
{
	class IGraphics;
	extern IGraphics* g_pGfx;

	class Camera;
	class IModel;
	class IPipeline;
	class Material;

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

		IPipeline* GetPipeline( const std::string& name )
		{
			auto it = m_mapPipelines.find( name );
			if( it == m_mapPipelines.end() )
			{
				return nullptr;
			}

			return it->second;
		}

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
		void AddShader( const std::string& name, IPipeline* pPipeline )
		{
			m_mapPipelines[name] = pPipeline;
		}
	protected:
		Camera* m_pCamera = nullptr;
		std::unordered_map<std::string, IPipeline*> m_mapPipelines;
	};
}