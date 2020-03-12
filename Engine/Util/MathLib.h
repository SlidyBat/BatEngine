#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <intrin.h>

namespace Bat
{
	class Vec2;
	class Vec3;
	class Vec4;

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
		Vec3 DegToRad( const Vec3& deg );
		Vec3 RadToDeg( const Vec3& rad );

		Vec3 QuaternionToEulerRad( const Vec4& quat );
		Vec4 EulerToQuaternionRad( const Vec3& euler );
		Vec3 QuaternionToEulerDeg( const Vec4& quat );
		Vec4 EulerToQuaternionDeg( const Vec3& euler );
		// Normalizes an angle so that it becomes in the range [-180.0f, 180.0f)
		float NormalizeAngleDeg( float ang );
		float NormalizeAngleRad( float ang );
		Vec3 NormalizeAngleDeg( const Vec3& ang );
		Vec3 NormalizeAngleRad( const Vec3& ang );

		__m128 Abs( __m128 m );
		__m128 Sin( __m128 m_x );
		float Sin( float x );
		float Cos( float x );
		void SinCos( float x, float* s, float* c );

		void AngleVectors( const Vec3& angles, Vec3* forward );
		void AngleVectors( const Vec3& angles, Vec3* forward, Vec3* right, Vec3* up );

		// Returns a random int in the range [min, max]
		int GetRandomInt( int min, int max );
		// Returns a random float in the range [min, max)
		float GetRandomFloat( float min, float max );

		bool CloseEnough( float a, float b, float epsilon = 0.001f );

		float Lerp( float a, float b, float t );
	}

	template <typename T>
	class _Vec2
	{
	public:
		_Vec2 operator-( const _Vec2& rhs ) const
		{
			return { x - rhs.x, y - rhs.y };
		}
		_Vec2 operator+( const _Vec2& rhs ) const
		{
			return { x + rhs.x, y + rhs.y };
		}

		_Vec2& operator-=( const _Vec2& rhs )
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}
		_Vec2& operator+=( const _Vec2& rhs )
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}
	public:
		T x;
		T y;
	};

	using Vei2 = _Vec2<int>;
	using Veu2 = _Vec2<unsigned>;

	template <typename T>
	class _Vec3
	{
	public:
		_Vec3 operator-( const _Vec3& rhs ) const
		{
			return { x - rhs.x, y - rhs.y, z - rhs.z };
		}
		_Vec3 operator+( const _Vec3& rhs ) const
		{
			return { x + rhs.x, y + rhs.y, z + rhs.z };
		}

		_Vec3& operator-=( const _Vec3& rhs )
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}
		_Vec3& operator+=( const _Vec3& rhs )
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}
	public:
		T x;
		T y;
		T z;
	};

	using Vei3 = _Vec3<int>;
	using Veu3 = _Vec3<unsigned>;

	template <typename T>
	class _Vec4
	{
	public:
		_Vec4( T x, T y, T z, T w )
			:
			x( x ),
			y( y ),
			z( z ),
			w( w )
		{}

		_Vec4 operator-( const _Vec4& rhs ) const
		{
			return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
		}
		_Vec4 operator+( const _Vec4& rhs ) const
		{
			return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
		}

		_Vec4& operator-=( const _Vec4& rhs )
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
			return *this;
		}
		_Vec4& operator+=( const _Vec4& rhs )
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}
	public:
		T x;
		T y;
		T z;
		T w;
	};

	using Vei4 = _Vec4<int>;
	using Veu4 = _Vec4<unsigned>;

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
		Vec2( const DirectX::XMVECTOR v )
		{
			DirectX::XMStoreFloat2( this, v );
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
		Vec2 operator+( const Vec2& rhs ) const
		{
			Vec2 res;
			res.x = x + rhs.x;
			res.y = y + rhs.y;

			return res;
		}
		Vec2 operator-( const Vec2& rhs ) const
		{
			Vec2 res;
			res.x = x - rhs.x;
			res.y = y - rhs.y;

			return res;
		}

		operator DirectX::XMVECTOR() const
		{
			return DirectX::XMLoadFloat2( this );
		}

		static Vec2 Lerp( const Vec2& a, const Vec2& b, float t )
		{
			return Vec2( Math::Lerp( a.x, b.x, t ),
				Math::Lerp( a.y, b.y, t ) );
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
		Vec3( const DirectX::XMVECTOR v )
		{
			DirectX::XMStoreFloat3( this, v );
		}

		bool operator==( const Vec3& rhs ) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z;
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
		Vec3 operator+( const Vec3& rhs ) const
		{
			Vec3 res;
			res.x = x + rhs.x;
			res.y = y + rhs.y;
			res.z = z + rhs.z;

			return res;
		}
		Vec3 operator-( const Vec3& rhs ) const
		{
			Vec3 res;
			res.x = x - rhs.x;
			res.y = y - rhs.y;
			res.z = z - rhs.z;

			return res;
		}
		Vec3& operator+=( const Vec3& rhs )
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;

			return *this;
		}
		Vec3& operator-=( const Vec3& rhs )
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;

			return *this;
		}
		Vec3& operator*=( float scalar )
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;

			return *this;
		}
		Vec3& operator/=( float scalar )
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;

			return *this;
		}

		float LengthSq() const
		{
			return x*x + y*y + z*z;
		}
		
		float Length() const
		{
			return sqrt( LengthSq() );
		}

		Vec3& Normalize()
		{
			float length = Length();
			x /= length;
			y /= length;
			z /= length;

			return *this;
		}

		Vec3 Normalized() const
		{
			float length = Length();

			return { x / length, y / length, z / length };
		}

		operator DirectX::XMVECTOR() const
		{
			return DirectX::XMLoadFloat3( this );
		}

		static float Dot( const Vec3& a, const Vec3& b )
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		static Vec3 Cross( const Vec3& a, const Vec3& b )
		{
			return {
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x
			};
		}

		static Vec3 Lerp( const Vec3& a, const Vec3& b, float t )
		{
			return Vec3( Math::Lerp( a.x, b.x, t ),
				Math::Lerp( a.y, b.y, t ),
				Math::Lerp( a.z, b.z, t ) );
		}

		static Vec3 Floor( const Vec3& v )
		{
			return {
				floorf( v.x ),
				floorf( v.y ),
				floorf( v.z )
			};
		}

		static Vec3 Reciprocal( const Vec3& v )
		{
			return { 1.0f / v.x, 1.0f / v.y, 1.0f / v.z };
		}

		static bool CloseEnough( const Vec3& a, const Vec3& b, float epsilon = 0.001f )
		{
			return Math::CloseEnough( a.x, b.x ) && Math::CloseEnough( a.y, b.y ) && Math::CloseEnough( a.z, b.z );
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
		Vec4( const DirectX::XMVECTOR v )
		{
			DirectX::XMStoreFloat4( this, v );
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
		Vec4 operator/( float scalar ) const
		{
			Vec4 res;
			res.x = x / scalar;
			res.y = y / scalar;
			res.z = z / scalar;
			res.w = w / scalar;

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

		Vec4& operator*=( float scalar )
		{
			x = x * scalar;
			y = y * scalar;
			z = z * scalar;
			w = w * scalar;

			return *this;
		}
		Vec4& operator/=( float scalar )
		{
			x = x / scalar;
			y = y / scalar;
			z = z / scalar;
			w = w / scalar;

			return *this;
		}

		float LengthSq() const
		{
			return x * x + y * y + z * z + w * w;
		}

		float Length() const
		{
			return sqrt( LengthSq() );
		}

		Vec4& Normalize()
		{
			float length = Length();
			x /= length;
			y /= length;
			z /= length;
			w /= length;

			return *this;
		}

		Vec4 Normalized() const
		{
			float length = Length();

			return { x / length, y / length, z / length, w / length };
		}

		operator DirectX::XMVECTOR() const
		{
			return DirectX::XMLoadFloat4( this );
		}

		static float Dot( const Vec4& a, const Vec4& b )
		{
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		static Vec4 Lerp( const Vec4& a, const Vec4& b, float t )
		{
			return Vec4( Math::Lerp( a.x, b.x, t ),
				Math::Lerp( a.y, b.y, t ),
				Math::Lerp( a.z, b.z, t ),
				Math::Lerp( a.w, b.w, t ) );
		}
	};

	struct Plane
	{
		Vec3 n; // Plane normal
		float d; // Distance from origin

		Plane() = default;
		explicit Plane( const Vec4& v )
			:
			n( v.x, v.y, v.z ),
			d( v.w )
		{}

		static Plane Normalize( const Plane& plane )
		{
			Plane normalized;

			float inv_l = 1.0f / plane.n.Length();
			normalized.n = plane.n * inv_l;
			normalized.d = plane.d * inv_l;

			return normalized;
		}
		static Plane Normalize( const Vec4& plane )
		{
			return Normalize( Plane( plane ) );
		}
	};

	class Mat4
	{
	public:
		Mat4();
		Mat4( const float values[4 * 4] );
		Mat4( float _00, float _01, float _02, float _03,
			float _10, float _11, float _12, float _13,
			float _20, float _21, float _22, float _23,
			float _30, float _31, float _32, float _33 );
		Mat4( const Mat4& copy );
		Mat4& operator=( const Mat4& rhs );

		Vec4 GetCol( size_t col ) const;
		void SetCol( size_t col, const Vec4& val );
		Vec4 GetRow( size_t row ) const;
		void SetRow( size_t row, const Vec4& val );

		// Basis vectors
		Vec3 GetForwardVector() const;
		Vec3 GetRightVector() const;
		Vec3 GetUpVector();

		Vec3 GetTranslation() const;
		void SetTranslation( const Vec3& val );
		Vec3 GetRotationRad() const;
		Vec3 GetRotationDeg() const;
		Vec4 GetRotationQuat() const;
		Vec3 GetScale() const;
		void DecomposeDeg( Vec3* out_pos, Vec3* out_rot, float* out_uniform_scale ) const;
		void DecomposeNonUniformScaleDeg( Vec3* out_pos, Vec3* out_rot, Vec3* out_scale ) const;

		static Mat4 Identity();
		static Mat4 Transpose( const Mat4& mat );

		// Fast inverse for matrices with only translation/rotation
		static Mat4 InverseTransRot( const Mat4& mat );
		static Mat4 Inverse( const Mat4& mat );

		static Mat4 RotateDeg( const Vec3& euler_deg );
		static Mat4 RotateDeg( float pitch, float yaw, float roll );
		static Mat4 RotateRad( const Vec3& euler_rad );
		static Mat4 RotateRad( float pitch, float yaw, float roll );
		static Mat4 RotateQuat( const Vec4& q );
		static Mat4 Translate( const Vec3& translation ) { return Translate( translation.x, translation.y, translation.z ); }
		static Mat4 Translate( float x, float y, float z );
		static Mat4 Scale( float uniform_scale );
		static Mat4 Scale( const Vec3& scale );
		static Mat4 Scale( float x, float y, float z );

		static Mat4 Ortho( float width, float height, float near_z, float far_z );
		static Mat4 OrthoOffCentre( float left, float right, float bottom, float top, float near_z, float far_z );
		static Mat4 Perspective( float width, float height, float near_z, float far_z );
		static Mat4 PerspectiveOffCentre( float left, float right, float bottom, float top, float near_z, float far_z );
		static Mat4 PerspectiveFov( float fov, float ar, float near_z, float far_z );
		static Mat4 LookTo( const Vec3& eye_pos, const Vec3& eye_dir, const Vec3& up );
		static Mat4 LookAt( const Vec3& eye_pos, const Vec3& target, const Vec3& up );

		static Vec3 Transform( const Mat4& mat, const Vec3& vec );
		static Vec3 TransformNormal( const Mat4& mat, const Vec3& vec );

		Mat4 operator*( const Mat4& r ) const;
		Vec4 operator*( const Vec4& rhs ) const;
		Vec3 operator*( const Vec3& rhs ) const;
		Plane operator*( const Plane& rhs ) const;
	private:
		float m[4][4];
	};

	// Axis-aligned bounding box
	class AABB
	{
	public:
		AABB() = default;
		AABB( const Vec3& mins, const Vec3& maxs );
		AABB( const Vec3* points, size_t num_points );

		Vec3 GetCenter() const { return (mins + maxs) / 2; }
		// Re-aligned transformed mins/maxs
		// This just gets the AABB of the rotated AABB of the mesh
		// Will have lots of empty space, but better than recalculating AABB for rotated mesh vertices
		AABB Transform( const Mat4& transform ) const;
		void GetPoints( Vec3 out[8] ) const;
	public:
		Vec3 mins;
		Vec3 maxs;
	};
}