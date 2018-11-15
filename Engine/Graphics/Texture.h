#pragma once

#include <d3d11.h>
#include <string>
#include <wrl.h>
#include "Colour.h"

namespace Bat
{
	class Texture
	{
	public:
		Texture() = default;
		Texture( const std::wstring& filename );
		Texture( const uint8_t* pData, size_t size );
		Texture( const Colour* pPixels, int width, int height );

		ID3D11ShaderResourceView* GetTextureView() const;
	private:
		Microsoft::WRL::ComPtr<ID3D11Resource>				m_pTexture = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pTextureView = nullptr;
	};
}