#pragma once

#define DECLARE_CLASS(classname, baseclass) typedef baseclass BaseClass; typedef classname ThisClass

#define BIND_MEM_FN( fn ) std::bind( &fn, this, std::placeholders::_1 )

#define STRINGIFY_( s ) #s
#define STRINGIFY( s )  STRINGIFY_( s )