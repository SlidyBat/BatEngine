#pragma once

#include <string>
#include <unordered_map>

namespace Bat
{
	class Texture;
	class RenderTexture;

	class RenderData
	{
		friend class RenderGraph;
	public:
		Texture* GetTexture( const std::string& name );
		RenderTexture* GetRenderTexture( const std::string& name );

		void AddTexture( const std::string& name, Texture* pTexture );
		void AddRenderTexture( const std::string& name, RenderTexture* pRenderTexture );
	private:
		std::unordered_map<std::string, Texture*> m_mapTextures;
		std::unordered_map<std::string, RenderTexture*> m_mapRenderTextures;
	};
}