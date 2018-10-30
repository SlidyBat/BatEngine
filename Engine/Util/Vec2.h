#pragma once

#include <cmath>

template <typename T>
class TVec2
{
public:
	TVec2() = default;
	TVec2( T x, T y )
		:
		x( x ),
		y( y )
	{}

	template <typename U>
	explicit TVec2( U src )
	{
		x = T( src.x );
		y = T( src.y );
	}
public:
	T LenSq() const
	{
		return ( x*x + y*y );
	}
	T Len() const
	{
		return (T)sqrt( LenSq() );
	}

	TVec2& operator=( const TVec2& rhs )
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}
	TVec2 operator+( const TVec2& rhs ) const
	{
		return TVec2( x + rhs.x, y + rhs.y );
	}
	TVec2& operator+=( const TVec2& rhs )
	{
		return ( *this = *this + rhs );
	}
	TVec2 operator-( const TVec2& rhs ) const
	{
		return TVec2( x - rhs.x, y - rhs.y );
	}
	TVec2& operator-=( const TVec2& rhs )
	{
		return ( *this = *this - rhs );
	}
	TVec2 operator*( const T rhs ) const
	{
		return TVec2( x*rhs, y*rhs );
	}
	TVec2& operator*=( const T rhs )
	{
		return ( *this = *this * rhs );
	}
	TVec2 operator/( const T rhs ) const
	{
		return TVec2( x / rhs, y / rhs );
	}
	TVec2& operator/=( const T rhs )
	{
		return ( *this = *this / rhs );
	}
	T operator*( const TVec2& rhs ) const
	{
		return x*rhs.x + y*rhs.y;
	}
	bool operator==( const TVec2& rhs ) const
	{
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=( const TVec2& rhs ) const
	{
		return !( *this == rhs );
	}
	TVec2 LerpTo( const TVec2& rhs, const float alpha ) const
	{
		return ( *this + ( rhs - *this )*alpha );
	}
public:
	T x;
	T y;
};

typedef TVec2<float> Vec2;
typedef TVec2<int> Vei2;
typedef TVec2<double> Ved2;