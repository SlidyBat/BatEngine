#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

#include "IShader.h"
#include "Texture.h"
#include "VertexShader.h"
#include "PixelShader.h"

namespace Bat
{
	struct CB_Matrix
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX viewproj;
	};

	class TextureShaderParameters : public IShaderParameters
	{
	public:
		TextureShaderParameters( const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& viewproj, Texture* pTexture, UINT indexcount )
			:
			pTexture( pTexture ),
			indexcount( indexcount )
		{
			transform.world = world;
			transform.viewproj = viewproj;
		}
		CB_Matrix* GetTransformMatrix()
		{
			return &transform;
		}
		ID3D11ShaderResourceView* GetTextureView() const
		{
			return pTexture->GetTextureView();
		}
		UINT GetIndexCount() const
		{
			return indexcount;
		}
	private:
		CB_Matrix transform;
		Texture* pTexture;
		UINT indexcount;
	};

	class TextureShader : public IShader
	{
	public:
		TextureShader( ID3D11Device* pDevice, const std::wstring& vsFilename, const std::wstring& psFilename );

		void Render( ID3D11DeviceContext* pDeviceContext, IShaderParameters* pParameters ) override;
	private:
		VertexShader m_VertexShader;
		PixelShader m_PixelShader;
	};
}