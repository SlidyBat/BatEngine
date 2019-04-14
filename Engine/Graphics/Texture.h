#pragma once

#include "Colour.h"
#include "StringLib.h"
#include "IGPUDevice.h"
#include <memory>

namespace Bat
{
	class Texture
	{
	public:
		Texture() = default;
		Texture( const std::string& filename );
		Texture( const std::wstring& filename )
			:
			Texture( Bat::WideToString( filename ) )
		{}
		Texture( const char* pData, size_t size );
		Texture( const void* pPixels, size_t pitch, size_t width, size_t height, TexFormat format, GPUResourceUsage usage = USAGE_DEFAULT );

		operator ITexture*( );
		ITexture* operator->();
		const ITexture* operator->() const;

		static Texture FromColour( const Colour* pPixels, int width, int height, GPUResourceUsage usage = USAGE_DEFAULT );

		static size_t GetBPPForFormat( TexFormat fmt );

	private:
		std::unique_ptr<ITexture> m_pTexture = nullptr;
	};
}