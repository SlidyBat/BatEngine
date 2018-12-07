#pragma once

#include <dxgitype.h>
#include <wrl.h>
#include "Colour.h"

struct ID3D11Resource;
struct ID3D11ShaderResourceView;

namespace Bat
{
	class Texture
	{
	public:
		Texture() = default;
		Texture( const std::wstring& filename );
		Texture( const uint8_t* pData, size_t size );
		Texture( const Colour* pPixels, int width, int height );
		Texture( const D3DCOLORVALUE* pPixels, int width, int height );

		static Texture FromDDS( const std::wstring& filename );

		ID3D11ShaderResourceView* GetTextureView() const;
	private:
		Microsoft::WRL::ComPtr<ID3D11Resource>				m_pTexture = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pTextureView = nullptr;
	};
}