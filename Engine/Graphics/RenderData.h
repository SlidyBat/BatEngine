#pragma once

#include <string>
#include <unordered_map>
#include "IGPUDevice.h"

namespace Bat
{
	class Texture;
	class RenderTarget;

	class RenderData
	{
		friend class RenderGraph;
	public:
		ITexture* GetTexture( const std::string& name );
		IRenderTarget* GetRenderTarget( const std::string& name );
		IDepthStencil* GetDepthStencil( const std::string& name );

		void AddTexture( const std::string& name, ITexture* pTexture );
		void AddRenderTarget( const std::string& name, IRenderTarget* pRenderTexture );
		void AddDepthStencil( const std::string& name, IDepthStencil* pDepthStencil );
	private:
		std::unordered_map<std::string, ITexture*> m_mapTextures;
		std::unordered_map<std::string, IRenderTarget*> m_mapRenderTextures;
		std::unordered_map<std::string, IDepthStencil*> m_mapDepthStencils;
	};
}