#pragma once

#include <d3d11.h>
#include <string>

class Texture
{
public:
	Texture( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::wstring& filename );
	~Texture();

	ID3D11ShaderResourceView* GetTextureView();
private:
	ID3D11Texture2D*			m_pTexture;
	ID3D11ShaderResourceView*	m_pTextureView;
};