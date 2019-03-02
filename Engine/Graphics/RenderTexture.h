#pragma once

#include <wrl.h>

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;

namespace Bat
{
	class RenderTexture
	{
	public:
		static void BindMultiple( const RenderTexture* pRenderTargets, const size_t count );
		static RenderTexture Backbuffer();
	public:
		RenderTexture() = default;
		RenderTexture( int width, int height );

		void Bind();
		void Clear( const float red, const float green, const float blue, const float alpha );
		void Resize( int width, int height );
		ID3D11ShaderResourceView* GetTextureView() const { return m_pShaderResourceView.Get(); }
		int GetTextureWidth() const { return m_iWidth; }
		int GetTextureHeight() const { return m_iHeight; }
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pRenderTargetTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;

		int m_iWidth = 0;
		int m_iHeight = 0;
	};
}