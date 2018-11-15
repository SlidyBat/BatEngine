#pragma once

#include "BatWinAPI.h"
#include "D3DClass.h"

#include "IGraphics.h"
#include "Window.h"
#include "GDIPManager.h"
#include "Colour.h"
#include "Texture.h"
#include "VertexTypes.h"
#include "Model.h"
#include "Camera.h"
#include "TextureShader.h"
//#include "ColourShader.h"


#include <memory>

namespace Bat
{
	class Graphics : public IGraphics
	{
	public:
		Graphics( Window& wnd );
		Graphics( const Graphics& src ) = delete;
		Graphics& operator=( const Graphics& src ) = delete;
		Graphics( Graphics&& donor ) = delete;
		Graphics& operator=( Graphics&& donor ) = delete;

		void Resize( int width = 0, int height = 0 )
		{
			d3d.Resize( width, height );
		}

		Model* CreateTexturedModel( const std::vector<TexVertex>& vertices, const std::vector<int>& indices, Texture& tex ) override;

		Texture CreateTexture( const std::wstring& filename ) override;
		Texture CreateTexture( const Bat::Colour* pPixels, int width, int height ) override;

		void BeginFrame();
		void EndFrame();

		ID3D11Device* GetDevice() override
		{
			return d3d.GetDevice();
		}
		ID3D11DeviceContext* GetDeviceContext() override
		{
			return d3d.GetDeviceContext();
		}
	private:
		D3DClass d3d;

		GDIPManager gdip;
	public:
		static constexpr bool	FullScreen = false;
		static constexpr int	VSyncEnabled = false;
		static constexpr float	ScreenFar = 1000.0f;
		static constexpr float	ScreenNear = 0.1f;
		static constexpr float	FOV = 90.0f;
		static constexpr float  FOVRadians = FOV * (DirectX::XM_PI / 180.0f);

		// only used when not in fullscreen
		static constexpr int	ScreenWidth = 800;
		static constexpr int	ScreenHeight = 600;
	};
}