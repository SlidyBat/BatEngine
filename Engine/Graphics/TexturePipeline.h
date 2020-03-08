#pragma once

#include "IPipeline.h"
#include "Texture.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

namespace Bat
{
	struct CB_TexturePipelineMatrix
	{
		Mat4 world = Mat4::Identity();
		Mat4 viewproj = Mat4::Identity();
	};

	class TexturePipeline : public IPipeline
	{
	public:
		void Render( IGPUContext* pContext, const Mesh& mesh, const Camera& camera, ITexture* pTexture, const Mat4& world_transform );
	private:
		ConstantBuffer<CB_TexturePipelineMatrix> m_cbufTransform;
	};
}