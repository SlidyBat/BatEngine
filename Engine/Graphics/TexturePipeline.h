#pragma once

#include "IPipeline.h"
#include "Texture.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

namespace Bat
{
	struct CB_TexturePipelineMatrix
	{
		DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX viewproj = DirectX::XMMatrixIdentity();
	};

	class TexturePipeline : public IPipeline
	{
	public:
		void Render( IGPUContext* pContext, const Mesh& mesh, const Camera& camera, ITexture* pTexture, const DirectX::XMMATRIX& world_transform );
	private:
		ConstantBuffer<CB_TexturePipelineMatrix> m_cbufTransform;
	};
}