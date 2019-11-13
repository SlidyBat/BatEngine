#pragma once

#include "IPipeline.h"
#include "Colour.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

namespace Bat
{
	class ColourPipeline : public IPipeline
	{
	public:
		void Render( IGPUContext* pContext, const Mesh& mesh, const Camera& camera, const DirectX::XMMATRIX& world_transform );
	private:
		struct CB_ColourPipelineMatrix
		{
			DirectX::XMMATRIX world;
			DirectX::XMMATRIX viewproj;
		};
		ConstantBuffer<CB_ColourPipelineMatrix> m_cbufTransform;
	};
}