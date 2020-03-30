#pragma once

namespace Bat
{
	class IGPUDevice;
	class FileSystem;

	struct GlobalValues
	{
		float elapsed_time;
		float deltatime;
	};

	extern GlobalValues* g_pGlobals;

	extern IGPUDevice* gpu;
	extern FileSystem* filesystem;
}
