#pragma once

#include <d3d11.h>
#include <string>
#include <wrl.h>
#include "Colour.h"

class Texture
{
public:
	Texture( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::wstring& filename );
	Texture( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Colour* pPixels, int width, int height );

	ID3D11ShaderResourceView* GetTextureView() const;
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_pTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pTextureView;
};