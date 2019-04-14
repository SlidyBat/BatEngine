#pragma once

namespace Bat
{
	class IGPUDevice;
	class Window;
	IGPUDevice* CreateD3DGPUDevice( Window& wnd, bool vsync_enabled, float screen_depth, float screen_near );
}