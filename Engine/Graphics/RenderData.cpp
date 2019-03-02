#include "PCH.h"
#include "RenderData.h"

namespace Bat
{
	Texture* RenderData::GetTexture( const std::string& name )
	{
		auto it = m_mapTextures.find( name );
		if( it != m_mapTextures.end() )
		{
			return it->second;
		}

		return nullptr;
	}

	RenderTexture* RenderData::GetRenderTexture( const std::string& name )
	{
		auto it = m_mapRenderTextures.find( name );
		if( it != m_mapRenderTextures.end() )
		{
			return it->second;
		}

		return nullptr;
	}

	void RenderData::AddTexture( const std::string& name, Texture* pTexture )
	{
		m_mapTextures[name] = pTexture;
	}

	void RenderData::AddRenderTexture( const std::string& name, RenderTexture* pRenderTexture )
	{
		m_mapRenderTextures[name] = pRenderTexture;
	}
}
