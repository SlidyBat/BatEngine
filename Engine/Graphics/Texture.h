#pragma once

#include <d3d11.h>
#include <wrl.h>
#include "Colour.h"
#include "StringLib.h"

namespace Bat
{
	class Texture
	{
	public:
		Texture() = default;
		Texture( const std::string& filename )
			:
			Texture( Bat::StringToWide( filename ) )
		{}
		Texture( const std::wstring& filename );
		Texture( const uint8_t* pData, size_t size );
		Texture( const void* pPixels, size_t pitch, int width, int height, DXGI_FORMAT format, D3D11_USAGE usage = D3D11_USAGE_DEFAULT );

		size_t GetWidth() const { return m_iWidth; }
		size_t GetHeight() const { return m_iHeight; }

		void UpdatePixels( const void* pPixels, size_t pitch );
		ID3D11ShaderResourceView* GetTextureView() const;

		static Texture FromDDS( const std::wstring& filename );
		static Texture FromColour( const Colour* pPixels, int width, int height, D3D11_USAGE usage = D3D11_USAGE_DEFAULT );
		static Texture FromD3DCOLORVALUE( const D3DCOLORVALUE* pPixels, int width, int height, D3D11_USAGE usage = D3D11_USAGE_DEFAULT );

		static size_t GetBPPForFormat( DXGI_FORMAT fmt );

	private:
		Microsoft::WRL::ComPtr<ID3D11Resource>				m_pTexture = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pTextureView = nullptr;

		size_t m_iWidth = 0;
		size_t m_iHeight = 0;
	};
}