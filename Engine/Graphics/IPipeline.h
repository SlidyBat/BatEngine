#pragma once

#include "VertexShader.h"
#include "PixelShader.h"

namespace Bat
{
	class IPipelineParameters
	{
	public:
		virtual ~IPipelineParameters() = default;
	};

	class IPipeline
	{
	public:
		IPipeline( const std::wstring& vsFilename, const std::wstring& psFilename )
			:
			m_VertexShader( vsFilename ),
			m_PixelShader( psFilename )
		{}
		virtual ~IPipeline() = default;

		virtual bool RequiresVertexAttribute( const VertexAttribute attribute )
		{
			return m_VertexShader.RequiresVertexAttribute( attribute );
		}

		virtual void BindParameters( IPipelineParameters* pParameters ) = 0;
		virtual void Render( UINT vertexcount ) = 0;
		virtual void RenderIndexed( UINT indexcount ) = 0;
	protected:
		VertexShader m_VertexShader;
		PixelShader m_PixelShader;
	};
}