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
		void Render( IGPUContext* pContext, const Mesh& mesh, const Camera& camera, const Mat4& world_transform );
	private:
		struct CB_ColourPipelineMatrix
		{
			Mat4 world;
			Mat4 viewproj;
		};
		ConstantBuffer<CB_ColourPipelineMatrix> m_cbufTransform;
	};
}