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

class TextureShaderParameters : public IShaderParameters
{
public:
	TextureShaderParameters( const DirectX::XMMATRIX& wvp, Texture* pTexture, UINT indexcount )
		:
		wvp(wvp),
		pTexture(pTexture),
		indexcount(indexcount)
	{}
	DirectX::XMMATRIX* GetTransformMatrix()
	{
		return &wvp;
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
	DirectX::XMMATRIX wvp;
	Texture* pTexture;
	UINT indexcount;
};

class TextureShader : public IShader
{
private:
	struct CB_Matrix
	{
		DirectX::XMMATRIX mat;
	};
public:
	TextureShader( ID3D11Device* pDevice, const std::wstring& vsFilename, const std::wstring& psFilename );

	void Render( ID3D11DeviceContext* pDeviceContext, IShaderParameters* pParameters ) override;
private:
	VertexShader m_VertexShader;
	PixelShader m_PixelShader;
};