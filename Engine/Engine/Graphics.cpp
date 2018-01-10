#include "Graphics.h"

bool Graphics::Initialize( const int screenWidth, const int screenHeight, HWND hWnd )
{
	return true;
}

bool Graphics::Frame()
{
	if( !Render() )
	{
		return false;
	}

	return true;
}

bool Graphics::Render()
{
	return true;
}
