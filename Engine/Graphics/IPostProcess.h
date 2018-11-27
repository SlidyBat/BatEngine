#pragma once

#include "RenderTexture.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include <string>

namespace Bat
{
	class IPostProcess
	{
	public:
		virtual ~IPostProcess() = default;

		virtual void BeginFrame( int width, int height )
		{
			if( m_Texture.GetTextureWidth() != width || m_Texture.GetTextureHeight() != height )
			{
				m_Texture.Resize( width, height );
			}
			m_Texture.Clear( 1.0f, 1.0f, 1.0f, 1.0f );
			m_Texture.Bind();
		}
		virtual void Render( ID3D11ShaderResourceView* pTexture ) = 0;
		virtual void EndFrame() {}

		ID3D11ShaderResourceView* GetTextureView()
		{
			return m_Texture.GetTextureView();
		}
	protected:
		RenderTexture m_Texture;
	};
}