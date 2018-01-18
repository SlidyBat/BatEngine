#pragma once

#include "SlidyWin.h"

class GDIPManager
{
public:
	GDIPManager();
	~GDIPManager();
private:
	static int nInstances;
	static ULONG_PTR token;
};