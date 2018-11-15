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
	class Model;
	class IShader;

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

		IShader* GetShader( const std::string& name )
		{
			auto it = m_mapShaders.find( name );
			if( it == m_mapShaders.end() )
			{
				return nullptr;
			}

			return it->second;
		}

		virtual Texture CreateTexture( const std::wstring& filename ) = 0;
		virtual Texture CreateTexture( const Bat::Colour* pPixels, int width, int height ) = 0;
		virtual Model* CreateTexturedModel( const std::vector<TexVertex>& vertices, const std::vector<int>& indices, Texture& tex ) = 0;

		Camera* GetCamera() const
		{
			return m_pCamera;
		}
		void SetCamera(Camera* pCamera)
		{
			m_pCamera = pCamera;
		}
	protected:
		void AddShader( const std::string& name, IShader* pShader )
		{
			m_mapShaders[name] = pShader;
		}
	protected:
		Camera* m_pCamera;
		std::unordered_map<std::string, IShader*> m_mapShaders;
	};
}