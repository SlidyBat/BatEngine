#pragma once

#include "SlidyWin.h"

class GDIPManager
{
public:
	GDIPManager();
	~GDIPManager() noexcept;
private:
	static int nInstances;
	static ULONG_PTR token;
};