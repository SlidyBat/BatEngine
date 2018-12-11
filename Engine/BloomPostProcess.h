#pragma once

#include "GenericPostProcess.h"
#include "RenderTexture.h"
#include "ResourceManager.h"
 
namespace Bat
{
	class BloomPostProcess : public GenericPostProcess
	{
	public:
		BloomPostProcess();

		virtual void Render( RenderTexture& pRenderTexture ) override;
	private:
		RenderTexture m_BloomFrameBuffer;
		RenderTexture m_BloomFrameBuffer2;
		Resource<PixelShader> m_pBrightExtractPS;
		Resource<PixelShader> m_pGaussBlurHorPS;
		Resource<PixelShader> m_pGaussBlurVerPS;
		Resource<PixelShader> m_pBloomShader;
	};
}