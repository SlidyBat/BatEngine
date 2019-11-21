#pragma once

#define DECLARE_CLASS( classname, baseclass ) typedef baseclass BaseClass; typedef classname ThisClass

#define BIND_MEM_FN( fn ) std::bind( &fn, this, std::placeholders::_1 )

#define BAT_DEBUG_BREAK() __debugbreak();

#define STRINGIFY_( s ) #s
#define STRINGIFY( s )  STRINGIFY_( s )

// Defines |/&/^ operators for a given type, intended use is for enum classes
#define BAT_ENUM_OPERATORS( T )                                                                             \
	inline T operator|( T lhs, T rhs )                                                                      \
	{                                                                                                       \
		using U = std::underlying_type_t<T>;                                                                \
		return static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));                                   \
	}                                                                                                       \
	inline T& operator|=( T& lhs, T rhs )                                                                   \
	{                                                                                                       \
		lhs = lhs | rhs;                                                                                    \
		return lhs;                                                                                         \
	}                                                                                                       \
	inline T operator&( T lhs, T rhs )                                                                      \
	{                                                                                                       \
		using U = std::underlying_type_t<T>;                                                                \
		return static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));                                   \
	}                                                                                                       \
	inline T& operator&=( T& lhs, T rhs )                                                                   \
	{                                                                                                       \
		lhs = lhs & rhs;                                                                                    \
		return lhs;                                                                                         \
	}                                                                                                       \
	inline T operator^( T lhs, T rhs )                                                                      \
	{                                                                                                       \
		using U = std::underlying_type_t<T>;                                                                \
		return static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));                                   \
	}                                                                                                       \
	inline T& operator^=( T& lhs, T rhs )                                                                   \
	{                                                                                                       \
		lhs = lhs ^ rhs;                                                                                    \
		return lhs;                                                                                         \
	}                                                                                                       \
	inline T operator~( T val )                                                                             \
	{                                                                                                       \
		using U = std::underlying_type_t<T>;                                                                \
		return static_cast<T>(~static_cast<U>(val));                                                        \
	}