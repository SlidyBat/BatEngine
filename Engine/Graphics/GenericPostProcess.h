#pragma once

#include "IPostProcess.h"
#include "MathLib.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Bat
{
	class GenericPostProcess : public IPostProcess
	{
	public:
		GenericPostProcess( const std::wstring& filename );

		virtual void Render( ID3D11ShaderResourceView* pTexture ) override;
	protected:
		VertexShader m_VertexShader;
		PixelShader m_PixelShader;
		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec2> m_bufUV;
		IndexBuffer m_bufIndices;
	};
}