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
		GenericPostProcess( int width, int height, const std::string& filename );

		virtual void Render( RenderTexture& pRenderTexture ) override;
	protected:
		VertexShader m_VertexShader;
		PixelShader m_PixelShader;
		VertexBuffer<Vec4> m_bufPosition;
		VertexBuffer<Vec2> m_bufUV;
		IndexBuffer m_bufIndices;
	};
}