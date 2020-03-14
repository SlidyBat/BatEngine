#pragma once

#include "PCH.h"

#include "IGPUDevice.h"
#include "IPipeline.h"
#include "Texture.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

namespace Bat
{
	class SkyboxPipeline : public IPipeline
	{
	public:
		SkyboxPipeline();

		void Render( IGPUContext* pContext, const Camera& camera, ITexture* pTexture );
	private:
		VertexBuffer<Vec4> m_bufPositions;
		IndexBuffer m_bufIndices;

		struct CB_SkyboxPipelineMatrix
		{
			Mat4 viewproj;
		};
		ConstantBuffer<CB_SkyboxPipelineMatrix> m_cbufTransform;
	};
}