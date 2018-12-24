#pragma once

#define BIND_MEM_FN( fn ) std::bind( &fn, this, std::placeholders::_1 )