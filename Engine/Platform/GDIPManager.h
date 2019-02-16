#pragma once

#include "BatWinAPI.h"

class GDIPManager
{
public:
	GDIPManager();
	~GDIPManager() noexcept;
private:
	static int nInstances;
	static ULONG_PTR token;
};