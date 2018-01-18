#include "Texture.h"
#include <cassert>
#include "Colour.h"

#define FULL_WINTARD
#include "SlidyWin.h"

#include <algorithm>
namespace Gdiplus
{
	using std::min;
	using std::max;
}
#include <gdiplus.h>

Texture::Texture( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::wstring& filename )
{
	Gdiplus::Bitmap bmp( filename.c_str() );
	assert( bmp.GetPixelFormat() == PixelFormat32bppARGB );

	Colour* pPixels = new Colour[bmp.GetWidth()*bmp.GetHeight()];
	for( size_t y = 0; y < bmp.GetHeight(); y++ )
	{
		for( size_t x = 0; x < bmp.GetWidth(); x++ )
		{
			Gdiplus::Color c;
			bmp.GetPixel( (int)x, (int)y, &c );
			pPixels[x + y*bmp.GetWidth()].SetValue( c.GetValue() );
		}
	}

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = bmp.GetWidth();
	textureDesc.Height = bmp.GetHeight();
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	pDevice->CreateTexture2D( &textureDesc, NULL, &m_pTexture ); // should add error checking
	pDeviceContext->UpdateSubresource( m_pTexture.Get(), 0, NULL, pPixels, bmp.GetWidth()*4, 0 );

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView( m_pTexture.Get(), &srvDesc, &m_pTextureView );

	pDeviceContext->GenerateMips( m_pTextureView.Get() );

	delete[] pPixels;
}

ID3D11ShaderResourceView* Texture::GetTextureView()
{
	return m_pTextureView.Get();
}
