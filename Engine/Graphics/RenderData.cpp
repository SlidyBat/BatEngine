#include "PCH.h"
#include "RenderData.h"

namespace Bat
{
	ITexture* RenderData::GetTexture( const std::string& name )
	{
		auto it = m_mapTextures.find( name );
		if( it != m_mapTextures.end() )
		{
			return it->second;
		}

		return nullptr;
	}

	IRenderTarget* RenderData::GetRenderTarget( const std::string& name )
	{
		auto it = m_mapRenderTextures.find( name );
		if( it != m_mapRenderTextures.end() )
		{
			return it->second;
		}

		return nullptr;
	}

	IDepthStencil* RenderData::GetDepthStencil( const std::string& name )
	{
		auto it = m_mapDepthStencils.find( name );
		if( it != m_mapDepthStencils.end() )
		{
			return it->second;
		}

		return nullptr;
	}

	void RenderData::AddTexture( const std::string& name, ITexture* pTexture )
	{
		m_mapTextures[name] = pTexture;
	}

	void RenderData::AddRenderTarget( const std::string& name, IRenderTarget* pRenderTexture )
	{
		m_mapRenderTextures[name] = pRenderTexture;
	}

	void RenderData::AddDepthStencil( const std::string& name, IDepthStencil* pDepthStencil )
	{
		m_mapDepthStencils[name] = pDepthStencil;
	}
}
