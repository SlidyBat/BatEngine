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

		virtual Texture* CreateTexture( const std::wstring& filename ) = 0;
		virtual Texture* CreateTexture( const Bat::Colour* pPixels, int width, int height ) = 0;
		virtual IModel* CreateColouredModel( const std::vector<ColourVertex>& vertices, const std::vector<int>& indices ) = 0;
		virtual IModel* CreateTexturedModel( const std::vector<TexVertex>& vertices, const std::vector<int>& indices, Texture& tex ) = 0;
		virtual IModel* CreateModel( const std::vector<Vertex>& vertices, const std::vector<int>& indices, Texture& tex ) = 0;

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