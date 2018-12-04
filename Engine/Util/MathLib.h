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

		Vec2& operator=( const DirectX::XMFLOAT2& src )
		{
			x = src.x;
			y = src.y;

			return *this;
		}
		Vec2 operator*( float scalar ) const
		{
			Vec2 res;
			res.x = x * scalar;
			res.y = y * scalar;

			return res;
		}
		Vec2 operator-() const
		{
			Vec2 res;
			res.x = -x;
			res.y = -y;

			return res;
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

		Vec3& operator=( const DirectX::XMFLOAT3& src )
		{
			x = src.x;
			y = src.y;
			z = src.z;

			return *this;
		}
		Vec3 operator*( float scalar ) const
		{
			Vec3 res;
			res.x = x * scalar;
			res.y = y * scalar;
			res.z = z * scalar;

			return res;
		}
		Vec3 operator-() const
		{
			Vec3 res;
			res.x = -x;
			res.y = -y;
			res.z = -z;

			return res;
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
		Vec4( const Vec3& v3, const float vw )
		{
			x = v3.x;
			y = v3.y;
			z = v3.z;
			w = vw;
		}

		Vec4( const DirectX::XMFLOAT4& src )
		{
			x = src.x;
			y = src.y;
			z = src.z;
			w = src.w;
		}

		Vec4& operator=( const DirectX::XMFLOAT4& src )
		{
			x = src.x;
			y = src.y;
			z = src.z;
			w = src.w;

			return *this;
		}
		Vec4& operator=( const DirectX::XMFLOAT3& src )
		{
			x = src.x;
			y = src.y;
			z = src.z;
			w = 1.0f;

			return *this;
		}
		Vec4 operator*( float scalar ) const
		{
			Vec4 res;
			res.x = x * scalar;
			res.y = y * scalar;
			res.z = z * scalar;
			res.w = w * scalar;

			return res;
		}
		Vec4 operator-() const
		{
			Vec4 res;
			res.x = -x;
			res.y = -y;
			res.z = -z;
			res.w = -w;

			return res;
		}
	};
}