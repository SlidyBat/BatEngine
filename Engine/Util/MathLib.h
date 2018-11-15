#pragma once

#include <DirectXMath.h>
#include <intrin.h>

namespace Bat
{
	namespace Math
	{
		constexpr float PI = DirectX::XM_PI;

		constexpr inline float DegToRad( const float deg )
		{
			return deg * (PI / 180.0f);
		}
		constexpr inline float RadToDeg( const float rad )
		{
			return rad * (180.0f / PI);
		}

		__m128 Abs( __m128 m );
		__m128 Sin( __m128 m_x );
		float Sin( float x );
		float Cos( float x );
		void SinCos( float x, float* s, float* c );

		void AngleVectors( const DirectX::XMFLOAT3& angles, DirectX::XMFLOAT3* forward = nullptr, DirectX::XMFLOAT3* right = nullptr, DirectX::XMFLOAT3* up = nullptr );
	}

	template <typename T>
	class _Vec2
	{
	public:
		T x;
		T y;
	};

	using Vei2 = _Vec2<int>;

	// Vec2/Vec3 aren't templated so that they can be used with the XMMath functions

	class Vec2 : public DirectX::XMFLOAT2
	{
	public:
		Vec2() = default;
		Vec2( const float x, const float y )
			:
			DirectX::XMFLOAT2( x, y )
		{}
		Vec2( const DirectX::XMFLOAT2& src )
		{
			x = src.x;
			y = src.y;
		}

		Vec2 operator=( const DirectX::XMFLOAT2& src )
		{
			x = src.x;
			y = src.y;

			return *this;
		}
		Vec2& operator*( float scalar )
		{
			x *= scalar;
			y *= scalar;

			return *this;
		}
		Vec2& operator-()
		{
			x = -x;
			y = -y;

			return *this;
		}
	};

	class Vec3 : public DirectX::XMFLOAT3
	{
	public:
		Vec3() = default;
		Vec3( const float x, const float y, const float z )
			:
			DirectX::XMFLOAT3( x, y, z )
		{}
		Vec3( const DirectX::XMFLOAT3& src )
		{
			x = src.x;
			y = src.y;
			z = src.z;
		}

		Vec3 operator=( const DirectX::XMFLOAT3& src )
		{
			x = src.x;
			y = src.y;
			z = src.z;

			return *this;
		}
		Vec3& operator*( float scalar )
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;

			return *this;
		}
		Vec3& operator-()
		{
			x = -x;
			y = -y;
			z = -z;

			return *this;
		}
	};

	class Vec4 : public DirectX::XMFLOAT4
	{
	public:
		Vec4() = default;
		Vec4( const float x, const float y, const float z, const float w )
			:
			DirectX::XMFLOAT4( x, y, z, w )
		{}
		Vec4( const DirectX::XMFLOAT4& src )
		{
			x = src.x;
			y = src.y;
			z = src.z;
		}

		Vec4 operator=( const DirectX::XMFLOAT4& src )
		{
			x = src.x;
			y = src.y;
			z = src.z;

			return *this;
		}
		Vec4& operator*( float scalar )
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;

			return *this;
		}
		Vec4& operator-()
		{
			x = -x;
			y = -y;
			z = -z;

			return *this;
		}
	};
}