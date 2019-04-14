#pragma once

#include "IGPUDevice.h"
#include "ResourceManager.h"

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
		IPipeline( const std::string& vs_filename, const std::string& ps_filename )
			:
			m_pVertexShader( ResourceManager::GetVertexShader( vs_filename ) ),
			m_pPixelShader( ResourceManager::GetPixelShader( ps_filename ) )
		{}
		virtual ~IPipeline() = default;

		virtual bool RequiresVertexAttribute( VertexAttribute attribute )
		{
			return m_pVertexShader->RequiresVertexAttribute( attribute );
		}

		virtual void BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters ) = 0;
		virtual void Render( IGPUContext* pContext, size_t vertexcount ) = 0;
		virtual void RenderIndexed( IGPUContext* pContext, size_t indexcount ) = 0;
	protected:
		Resource<IVertexShader> m_pVertexShader;
		Resource<IPixelShader> m_pPixelShader;
	};
}