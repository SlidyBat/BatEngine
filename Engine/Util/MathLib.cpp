#include "PCH.h"
#include "MathLib.h"

#include <random>

namespace Bat
{
	namespace Math
	{
		class Random
		{
		public:
			Random()
				:
				rng( std::random_device{}() )
			{}

			template <typename T>
			T GetRandomInt( T min, T max )
			{
				std::uniform_int_distribution<T> dist( min, max );
				return dist( rng );
			}

			template <typename T>
			T GetRandomReal( T min, T max )
			{
				std::uniform_real_distribution<T> dist( min, max );
				return dist( rng );
			}
		private:
			std::mt19937 rng;
		};

		static Random rng;

		Vec3 QuaternionToEulerRad( const Vec4& quat )
		{
			Vec3 output;

			float sqw;
			float sqx;
			float sqy;
			float sqz;

			sqw = quat.w * quat.w;
			sqx = quat.x * quat.x;
			sqy = quat.y * quat.y;
			sqz = quat.z * quat.z;

			float unit = sqw + sqx + sqy + sqz;
			float test = quat.x * quat.w - quat.y * quat.z;

			if( test > 0.4995f * unit )
			{
				output.y = ( 2.0f * atan2( quat.y, quat.x ) );
				output.x = ( DirectX::XM_PIDIV2 );

				return output;
			}
			if( test < -0.4995f * unit )
			{
				output.y = ( -2.0f * atan2( quat.y, quat.x ) );
				output.x = ( -DirectX::XM_PIDIV2 );

				return output;
			}

			output.x = ( asin( 2.0f * ( quat.w * quat.x - quat.z * quat.y ) ) );
			output.y = ( atan2( 2.0f * quat.w * quat.y + 2.0f * quat.z * quat.x, 1 - 2.0f * ( sqx + sqy ) ) );
			output.z = ( atan2( 2.0f * quat.w * quat.z + 2.0f * quat.x * quat.y, 1 - 2.0f * ( sqz + sqx ) ) );

			return output;
		}

		Vec4 EulerToQuaternionRad( const Vec3& euler )
		{
			return DirectX::XMQuaternionRotationRollPitchYaw( euler.x, euler.y, euler.z );
		}
		Vec3 QuaternionToEulerDeg( const Vec4& quat )
		{
			Vec3 rad = QuaternionToEulerRad( quat );
			return {
				Math::RadToDeg( rad.x ),
				Math::RadToDeg( rad.y ),
				Math::RadToDeg( rad.z )
			};
		}

		Vec4 EulerToQuaternionDeg( const Vec3& euler )
		{
			Vec3 rad = {
				Math::DegToRad( euler.x ),
				Math::DegToRad( euler.y ),
				Math::DegToRad( euler.z )
			};
			return EulerToQuaternionRad( rad );
		}

		float NormalizeAngle( float ang )
		{
			while( ang >= 180.0f )
			{
				ang -= 360.0f;
			}
			while( ang < -180.0f )
			{
				ang += 360.0f;
			}
			return ang;
		}

		Vec3 NormalizeAngle( const Vec3& ang )
		{
			return {
				Math::NormalizeAngle( ang.x ),
				Math::NormalizeAngle( ang.y ),
				Math::NormalizeAngle( ang.z )
			};
		}

		__m128 Abs( __m128 m )
		{
			__m128 sign = _mm_castsi128_ps( _mm_set1_epi32( 0x80000000 ) );
			return _mm_andnot_ps( sign, m );
		}

		__m128 Sin( __m128 m_x )
		{
			const float B = 4.f / PI;
			const float C = -4.f / (PI * PI);
			const float P = 0.225f;
			//float y = B * x + C * x * abs(x);
			//y = P * (y * abs(y) - y) + y;
			__m128 m_pi = _mm_set1_ps( PI );
			__m128 m_mpi = _mm_set1_ps( -PI );
			__m128 m_2pi = _mm_set1_ps( PI * 2 );
			__m128 m_B = _mm_set1_ps( B );
			__m128 m_C = _mm_set1_ps( C );
			__m128 m_P = _mm_set1_ps( P );
			__m128 m1 = _mm_cmpnlt_ps( m_x, m_pi );
			m1 = _mm_and_ps( m1, m_2pi );
			m_x = _mm_sub_ps( m_x, m1 );
			m1 = _mm_cmpngt_ps( m_x, m_mpi );
			m1 = _mm_and_ps( m1, m_2pi );
			m_x = _mm_add_ps( m_x, m1 );
			__m128 m_abs = Abs( m_x );
			m1 = _mm_mul_ps( m_abs, m_C );
			m1 = _mm_add_ps( m1, m_B );
			__m128 m_y = _mm_mul_ps( m1, m_x );
			m_abs = Abs( m_y );
			m1 = _mm_mul_ps( m_abs, m_y );
			m1 = _mm_sub_ps( m1, m_y );
			m1 = _mm_mul_ps( m1, m_P );
			m_y = _mm_add_ps( m1, m_y );
			return m_y;
		}

		float Sin( float x )
		{
			__m128 m_x = _mm_set1_ps( x );
			__m128 m_sin = Sin( m_x );
			return _mm_cvtss_f32( m_sin );
		}

		float Cos( float x )
		{
			__m128 m_x = _mm_set1_ps( x + PI / 2.f );
			__m128 m_cos = Sin( m_x );
			return _mm_cvtss_f32( m_cos );
		}

		void SinCos( float x, float* s, float* c )
		{
			__m128 m_both = _mm_set_ps( 0.f, 0.f, x + PI / 2.f, x );
			__m128 m_sincos = Sin( m_both );
			__m128 m_cos = _mm_shuffle_ps( m_sincos, m_sincos, _MM_SHUFFLE( 0, 0, 0, 1 ) );
			*s = _mm_cvtss_f32( m_sincos );
			*c = _mm_cvtss_f32( m_cos );
		}

		void AngleVectors( const Vec3& angles, Vec3* forward )
		{
			float sp, sy, cp, cy;

			SinCos( DegToRad( angles.x ), &sp, &cp );
			SinCos( DegToRad( angles.y ), &sy, &cy );

			forward->x = sy * cp;
			forward->y = sp;
			forward->z = cy * cp;
		}

		void AngleVectors( const Vec3& angles, Vec3* forward, Vec3* right, Vec3* up )
		{
			float sp, sy, cp, cy;
			float syr, cyr;

			SinCos( DegToRad( angles.x ), &sp, &cp );
			SinCos( DegToRad( angles.y ), &sy, &cy );
			SinCos( DegToRad( angles.y + 90.0f ), &syr, &cyr );

			if( forward )
			{
				forward->x = sy * cp;
				forward->y = 0.0f;
				forward->z = cy * cp;
			}

			if( right )
			{
				right->x = syr * cp;
				right->y = 0.0f;
				right->z = cyr * cp;
			}

			if( up )
			{
				up->x = 0.0f;
				up->y = sp;
				up->z = 0.0f;
			}
		}

		int GetRandomInt( int min, int max )
		{
			return rng.GetRandomInt<int>( min, max );
		}

		float GetRandomFloat( float min, float max )
		{
			return rng.GetRandomReal<float>( min, max );
		}

		bool CloseEnough( float a, float b, float epsilon )
		{
			return abs( a - b ) < epsilon;
		}

		float Lerp( float a, float b, float t )
		{
			return a * (1 - t) + b * t;
		}
	}

	Mat4::Mat4()
	{
		memset( m, 0, sizeof( m ) );
	}
	Mat4::Mat4( const float values[4 * 4] )
	{
		memcpy( m, values, sizeof( m ) );
	}
	Mat4::Mat4( float _00, float _01, float _02, float _03,
		float _10, float _11, float _12, float _13,
		float _20, float _21, float _22, float _23,
		float _30, float _31, float _32, float _33 )
	{
		m[0][0] = _00; m[0][1] = _01; m[0][2] = _02; m[0][3] = _03;
		m[1][0] = _10; m[1][1] = _11; m[1][2] = _12; m[1][3] = _13;
		m[2][0] = _20; m[2][1] = _21; m[2][2] = _22; m[2][3] = _23;
		m[3][0] = _30; m[3][1] = _31; m[3][2] = _32; m[3][3] = _33;
	}
	Mat4::Mat4( const Mat4& copy )
	{
		*this = copy;
	}
	Mat4& Mat4::operator=( const Mat4& rhs )
	{
		memcpy( m, rhs.m, sizeof( m ) );
		return *this;
	}
	Vec4 Mat4::GetCol( size_t col ) const
	{
		ASSERT( col <= 3, "Invalid column index" );
		return {
			m[0][col],
			m[1][col],
			m[2][col],
			m[3][col]
		};
	}
	void Mat4::SetCol( size_t col, const Vec4& val )
	{
		ASSERT( col <= 3, "Invalid column index" );
		m[0][col] = val.x;
		m[1][col] = val.y;
		m[2][col] = val.z;
		m[3][col] = val.w;
	}
	Vec4 Mat4::GetRow( size_t row ) const
	{
		ASSERT( row <= 3, "Invalid row index" );
		return {
			m[row][0],
			m[row][1],
			m[row][2],
			m[row][3]
		};
	}
	void Mat4::SetRow( size_t row, const Vec4& val )
	{
		ASSERT( row <= 3, "Invalid row index" );
		m[row][0] = val.x;
		m[row][1] = val.y;
		m[row][2] = val.z;
		m[row][3] = val.w;
	}
	Vec3 Mat4::GetTranslation() const
	{
		return {
			m[0][3],
			m[1][3],
			m[2][3]
		};
	}
	void Mat4::SetTranslation( const Vec3& val )
	{
		m[0][3] = val.x;
		m[1][3] = val.y;
		m[2][3] = val.z;
	}
	Vec3 Mat4::GetRotationRad() const
	{
		Vec3 euler;

		if( !Math::CloseEnough( abs( m[1][2] ), 1.0f ) )
		{
			euler.x = -asin( m[1][2] );
			euler.y = atan2( m[0][2] / euler.x, m[2][2] / euler.x );
			euler.z = atan2( m[1][0] / euler.x, m[1][1] / euler.x );
		}
		else
		{
			euler.x = ( Math::PI / 2.0f ) * -m[1][2];
			euler.y = atan2( m[2][0] * -m[1][2], m[0][0] * -m[1][2] );
			euler.z = 0.0f;
		}

		return euler;
	}
	Vec3 Mat4::GetRotationDeg() const
	{
		Vec3 euler_rad = GetRotationRad();
		return { Math::RadToDeg( euler_rad.x ), Math::RadToDeg( euler_rad.y ), Math::RadToDeg( euler_rad.z ) };
	}
	Vec4 Mat4::GetRotationQuat() const
	{
		return Math::EulerToQuaternion( GetRotationRad() );
	}
	Mat4 Mat4::Identity()
	{
		Mat4 mat;
		for( int i = 0; i < 4; i++ )
		{
			mat.m[i][i] = 1.0f;
		}
		return mat;
	}
	Mat4 Mat4::Transpose( const Mat4& mat )
	{
		Mat4 transposed;
		for( int i = 0; i < 4; i++ )
		{
			for( int j = 0; j < 4; j++ )
			{
				transposed.m[i][j] = mat.m[j][i];
			}
		}
		return transposed;
	}
	Mat4 Mat4::InverseTransRot( const Mat4& mat )
	{
		Mat4 inverse;

		// Transpose upper 3x3 (inverse rotation)
		for( int i = 0; i < 3; i++ )
		{
			for( int j = 0; j < 3; j++ )
			{
				inverse.m[i][j] = mat.m[j][i];
			}
		}

		Vec3 invtransform = { -mat.m[0][3], -mat.m[1][3], -mat.m[2][3] };

		inverse.m[3][3] = 1.0f;

		return inverse;
	}
	Mat4 Mat4::Inverse( const Mat4& mat )
	{
		Mat4 inv;
		float wtmp[4][8];
		float m0, m1, m2, m3, s;
		float* r0;
		float* r1;
		float* r2;
		float* r3;

		r0 = wtmp[0]; r1 = wtmp[1]; r2 = wtmp[2]; r3 = wtmp[3];

		r0[0] = mat.m[0][0]; r0[1] = mat.m[0][1];
		r0[2] = mat.m[0][2]; r0[3] = mat.m[0][3];
		r0[4] = 1.0; r0[5] = r0[6] = r0[7] = 0.0;

		r1[0] = mat.m[1][0]; r1[1] = mat.m[1][1];
		r1[2] = mat.m[1][2]; r1[3] = mat.m[1][3];
		r1[5] = 1.0; r1[4] = r1[6] = r1[7] = 0.0;

		r2[0] = mat.m[2][0]; r2[1] = mat.m[2][1];
		r2[2] = mat.m[2][2]; r2[3] = mat.m[2][3];
		r2[6] = 1.0; r2[4] = r2[5] = r2[7] = 0.0;

		r3[0] = mat.m[3][0]; r3[1] = mat.m[3][1];
		r3[2] = mat.m[3][2]; r3[3] = mat.m[3][3];
		r3[7] = 1.0; r3[4] = r3[5] = r3[6] = 0.0;

		/* choose pivot - or die */
		if( fabsf( r3[0] ) > fabsf( r2[0] ) ) std::swap( r3, r2 );
		if( fabsf( r2[0] ) > fabsf( r1[0] ) ) std::swap( r2, r1 );
		if( fabsf( r1[0] ) > fabsf( r0[0] ) ) std::swap( r1, r0 );
		if( 0.0F == r0[0] )  return {};

		/* eliminate first variable     */
		m1 = r1[0] / r0[0]; m2 = r2[0] / r0[0]; m3 = r3[0] / r0[0];
		s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
		s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
		s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
		s = r0[4];
		if( s != 0.0F ) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
		s = r0[5];
		if( s != 0.0F ) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
		s = r0[6];
		if( s != 0.0F ) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
		s = r0[7];
		if( s != 0.0F ) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

		/* choose pivot - or die */
		if( fabsf( r3[1] ) > fabsf( r2[1] ) ) std::swap( r3, r2 );
		if( fabsf( r2[1] ) > fabsf( r1[1] ) ) std::swap( r2, r1 );
		if( 0.0F == r1[1] )  return {};

		/* eliminate second variable */
		m2 = r2[1] / r1[1]; m3 = r3[1] / r1[1];
		r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
		r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
		s = r1[4]; if( 0.0F != s ) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
		s = r1[5]; if( 0.0F != s ) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
		s = r1[6]; if( 0.0F != s ) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
		s = r1[7]; if( 0.0F != s ) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

		/* choose pivot - or die */
		if( fabsf( r3[2] ) > fabsf( r2[2] ) ) std::swap( r3, r2 );
		if( 0.0F == r2[2] )  return {};

		/* eliminate third variable */
		m3 = r3[2] / r2[2];
		r3[3] -= m3 * r2[3]; r3[4] -= m3 * r2[4];
		r3[5] -= m3 * r2[5]; r3[6] -= m3 * r2[6];
		r3[7] -= m3 * r2[7];

		/* last check */
		if( 0.0F == r3[3] ) return {};

		s = 1.0F / r3[3];             /* now back substitute row 3 */
		r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

		m2 = r2[3];                 /* now back substitute row 2 */
		s = 1.0F / r2[2];
		r2[4] = s * ( r2[4] - r3[4] * m2 ); r2[5] = s * ( r2[5] - r3[5] * m2 );
		r2[6] = s * ( r2[6] - r3[6] * m2 ); r2[7] = s * ( r2[7] - r3[7] * m2 );
		m1 = r1[3];
		r1[4] -= r3[4] * m1; r1[5] -= r3[5] * m1;
		r1[6] -= r3[6] * m1; r1[7] -= r3[7] * m1;
		m0 = r0[3];
		r0[4] -= r3[4] * m0; r0[5] -= r3[5] * m0;
		r0[6] -= r3[6] * m0; r0[7] -= r3[7] * m0;

		m1 = r1[2];                 /* now back substitute row 1 */
		s = 1.0F / r1[1];
		r1[4] = s * ( r1[4] - r2[4] * m1 ); r1[5] = s * ( r1[5] - r2[5] * m1 );
		r1[6] = s * ( r1[6] - r2[6] * m1 ); r1[7] = s * ( r1[7] - r2[7] * m1 );
		m0 = r0[2];
		r0[4] -= r2[4] * m0; r0[5] -= r2[5] * m0,
		r0[6] -= r2[6] * m0; r0[7] -= r2[7] * m0;

		m0 = r0[1];                 /* now back substitute row 0 */
		s = 1.0F / r0[0];
		r0[4] = s * ( r0[4] - r1[4] * m0 ), r0[5] = s * ( r0[5] - r1[5] * m0 ),
		r0[6] = s * ( r0[6] - r1[6] * m0 ), r0[7] = s * ( r0[7] - r1[7] * m0 );

		inv.m[0][0] = r0[4]; inv.m[0][1] = r0[5];
		inv.m[0][2] = r0[6]; inv.m[0][3] = r0[7];
		inv.m[1][0] = r1[4]; inv.m[1][1] = r1[5];
		inv.m[1][2] = r1[6]; inv.m[1][3] = r1[7];
		inv.m[2][0] = r2[4]; inv.m[2][1] = r2[5];
		inv.m[2][2] = r2[6]; inv.m[2][3] = r2[7];
		inv.m[3][0] = r3[4]; inv.m[3][1] = r3[5];
		inv.m[3][2] = r3[6]; inv.m[3][3] = r3[7];

		return inv;
	}
	Mat4 Mat4::RotateDeg( const Vec3& euler_deg )
	{
		return RotateDeg( euler_deg.x, euler_deg.y, euler_deg.z );
	}
	Mat4 Mat4::RotateDeg( float pitch, float yaw, float roll )
	{
		return RotateRad( Math::DegToRad( pitch ), Math::DegToRad( yaw ), Math::DegToRad( roll ) );
	}
	Mat4 Mat4::RotateRad( const Vec3& euler_rad )
	{
		return RotateRad( euler_rad.x, euler_rad.y, euler_rad.z );
	}
	Mat4 Mat4::RotateRad( float pitch, float yaw, float roll )
	{
		float sp, cp, sy, cy, sr, cr;
		Math::SinCos( pitch, &sp, &cp );
		Math::SinCos( yaw, &sy, &cy );
		Math::SinCos( roll, &sr, &cr );

		float crcy = cr * cy;
		float crsy = cr * sy;
		float srcy = sr * cy;
		float srsy = sr * sy;

		Mat4 rotate;
		rotate.m[0][0] = srsy * sp + crcy;
		rotate.m[1][0] = cp * sr;
		rotate.m[2][0] = srcy * sp - crsy;

		rotate.m[0][1] = crsy * sp - srcy;
		rotate.m[1][1] = cr * cp;
		rotate.m[2][1] = crcy * sp + srsy;

		rotate.m[0][2] = sy * cp;
		rotate.m[1][2] = -sp;
		rotate.m[2][2] = cy * cp;

		rotate.m[3][3] = 1.0f;

		return rotate;
	}
	Mat4 Mat4::RotateQuat( const Vec4& q )
	{
		Mat4 rotate;

		rotate.m[0][0] = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
		rotate.m[1][0] = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
		rotate.m[2][0] = 2.0f * q.x * q.z - 2.0f * q.w * q.y;

		rotate.m[0][1] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
		rotate.m[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
		rotate.m[2][1] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;

		rotate.m[0][2] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
		rotate.m[1][2] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
		rotate.m[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;

		rotate.m[3][3] = 1.0f;

		return rotate;
	}
	Mat4 Mat4::Translate( float x, float y, float z )
	{
		Mat4 translate;
		translate.m[0][3] = x;
		translate.m[1][3] = y;
		translate.m[2][3] = z;

		translate.m[0][0] = 1.0f;
		translate.m[1][1] = 1.0f;
		translate.m[2][2] = 1.0f;
		translate.m[3][3] = 1.0f;

		return translate;
	}
	Mat4 Mat4::Scale( float uniform_scale )
	{
		return Scale( uniform_scale, uniform_scale, uniform_scale );
	}
	Mat4 Mat4::Scale( const Vec3& scale )
	{
		return Scale( scale.x, scale.y, scale.z );
	}
	Mat4 Mat4::Scale( float x, float y, float z )
	{
		Mat4 scale;
		scale.m[0][0] = x;
		scale.m[1][1] = y;
		scale.m[2][2] = z;
		scale.m[3][3] = 1.0f;

		return scale;
	}
	Mat4 Mat4::Ortho( float width, float height, float near_z, float far_z )
	{
		float range = 1.0f / ( far_z - near_z );

		Mat4 ortho;

		ortho.m[0][0] = 2.0f / width;
		ortho.m[1][0] = 0.0f;
		ortho.m[2][0] = 0.0f;
		ortho.m[3][0] = 0.0f;

		ortho.m[0][1] = 0.0f;
		ortho.m[1][1] = 2.0f / height;
		ortho.m[2][1] = 0.0f;
		ortho.m[3][1] = 0.0f;

		ortho.m[0][2] = 0.0f;
		ortho.m[1][2] = 0.0f;
		ortho.m[2][2] = range;
		ortho.m[3][2] = 0.0f;

		ortho.m[0][3] = 0.0f;
		ortho.m[1][3] = 0.0f;
		ortho.m[2][3] = -range * near_z;
		ortho.m[3][3] = 1.0f;

		return ortho;
	}
	Mat4 Mat4::OrthoOffCentre( float left, float right, float bottom, float top, float near_z, float far_z )
	{
		float inv_width = 1.0f / ( right - left );
		float inv_height = 1.0f / ( top - bottom );
		float range = 1.0f / ( far_z - near_z );

		Mat4 ortho;

		ortho.m[0][0] = inv_width + inv_width;
		ortho.m[1][0] = 0.0f;
		ortho.m[2][0] = 0.0f;
		ortho.m[3][0] = 0.0f;

		ortho.m[0][1] = 0.0f;
		ortho.m[1][1] = inv_height + inv_height;
		ortho.m[2][1] = 0.0f;
		ortho.m[3][1] = 0.0f;

		ortho.m[0][2] = 0.0f;
		ortho.m[1][2] = 0.0f;
		ortho.m[2][2] = range;
		ortho.m[3][2] = 0.0f;

		ortho.m[0][3] = -( left + right ) * inv_width;
		ortho.m[1][3] = -( top + bottom ) * inv_height;
		ortho.m[2][3] = -range * near_z;
		ortho.m[3][3] = 1.0f;

		return ortho;
	}
	Mat4 Mat4::Perspective( float width, float height, float near_z, float far_z )
	{
		float two_near_z = near_z + near_z;
		float range = far_z / ( far_z - near_z );

		Mat4 perspective;

		perspective.m[0][0] = two_near_z / width;
		perspective.m[1][0] = 0.0f;
		perspective.m[2][0] = 0.0f;
		perspective.m[3][0] = 0.0f;

		perspective.m[0][1] = 0.0f;
		perspective.m[1][1] = two_near_z / height;
		perspective.m[2][1] = 0.0f;
		perspective.m[3][1] = 0.0f;

		perspective.m[0][2] = 0.0f;
		perspective.m[1][2] = 0.0f;
		perspective.m[2][2] = range;
		perspective.m[3][2] = 1.0f;

		perspective.m[0][3] = 0.0f;
		perspective.m[1][3] = 0.0f;
		perspective.m[2][3] = -range * near_z;
		perspective.m[3][3] = 0.0f;

		return perspective;
	}
	Mat4 Mat4::PerspectiveOffCentre( float left, float right, float bottom, float top, float near_z, float far_z )
	{
		float two_near_z = near_z + near_z;
		float inv_width = 1.0f / ( right - left );
		float inv_height = 1.0f / ( top - bottom );
		float range = far_z / ( far_z - near_z );

		Mat4 perspective;

		perspective.m[0][0] = two_near_z * inv_width;
		perspective.m[1][0] = 0.0f;
		perspective.m[2][0] = 0.0f;
		perspective.m[3][0] = 0.0f;

		perspective.m[0][1] = 0.0f;
		perspective.m[1][1] = two_near_z * inv_height;
		perspective.m[2][1] = 0.0f;
		perspective.m[3][1] = 0.0f;

		perspective.m[0][2] = -( left + right ) * inv_width;
		perspective.m[1][2] = -( top + bottom ) * inv_height;
		perspective.m[2][2] = range;
		perspective.m[3][2] = 1.0f;

		perspective.m[0][3] = 0.0f;
		perspective.m[1][3] = 0.0f;
		perspective.m[2][3] = -range * near_z;
		perspective.m[3][3] = 0.0f;

		return perspective;
	}
	Mat4 Mat4::PerspectiveFov( float fov, float ar, float near_z, float far_z )
	{
		float sfov, cfov;
		Math::SinCos( 0.5f * fov, &sfov, &cfov );

		float height = cfov / sfov;
		float width = height / ar;
		float range = far_z / ( far_z - near_z );

		Mat4 perspective;

		perspective.m[0][0] = width;
		perspective.m[1][0] = 0.0f;
		perspective.m[2][0] = 0.0f;
		perspective.m[3][0] = 0.0f;

		perspective.m[0][1] = 0.0f;
		perspective.m[1][1] = height;
		perspective.m[2][1] = 0.0f;
		perspective.m[3][1] = 0.0f;

		perspective.m[0][2] = 0.0f;
		perspective.m[1][2] = 0.0f;
		perspective.m[2][2] = range;
		perspective.m[3][2] = 1.0f;

		perspective.m[0][3] = 0.0f;
		perspective.m[1][3] = 0.0f;
		perspective.m[2][3] = -range * near_z;
		perspective.m[3][3] = 0.0f;

		return perspective;
	}
	Mat4 Mat4::LookTo( const Vec3& eye_pos, const Vec3& eye_dir, const Vec3& up )
	{
		Vec3 r2 = XMVector3Normalize( eye_dir );

		Vec3 r0 = Vec3::Cross( up, r2 );
		r0.Normalize();

		Vec3 r1 = Vec3::Cross( r2, r0 );

		const Vec3 neg_eye_pos = -eye_pos;

		float d0 = Vec3::Dot( r0, neg_eye_pos );
		float d1 = Vec3::Dot( r1, neg_eye_pos );
		float d2 = Vec3::Dot( r2, neg_eye_pos );

		Mat4 look;

		look.m[0][0] = r0.x;
		look.m[0][1] = r0.y;
		look.m[0][2] = r0.z;
		look.m[0][3] = d0;

		look.m[1][0] = r1.x;
		look.m[1][1] = r1.y;
		look.m[1][2] = r1.z;
		look.m[1][3] = d1;

		look.m[2][0] = r2.x;
		look.m[2][1] = r2.y;
		look.m[2][2] = r2.z;
		look.m[2][3] = d2;

		look.m[3][0] = 0.0f;
		look.m[3][1] = 0.0f;
		look.m[3][2] = 0.0f;
		look.m[3][3] = 1.0f;

		return look;
	}
	Mat4 Mat4::LookAt( const Vec3& eye_pos, const Vec3& target, const Vec3& up )
	{
		Vec3 eye_dir = target - eye_pos;
		return LookTo( eye_pos, eye_dir, up );
	}
	Vec3 Mat4::Transform( const Mat4& mat, const Vec3& vec )
	{
		return {
			mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z + mat.m[0][3],
			mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z + mat.m[1][3],
			mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z + mat.m[2][3]
		};
	}
	Vec3 Mat4::TransformNormal( const Mat4& mat, const Vec3& vec )
	{
		return {
			mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z,
			mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z,
			mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z
		};
	}
	Mat4 Mat4::operator*( const Mat4& r ) const
	{
		return {
			r.m[0][0] * m[0][0] + r.m[0][1] * m[1][0] + r.m[0][2] * m[2][0] + r.m[0][3] * m[3][0],
			r.m[0][0] * m[0][1] + r.m[0][1] * m[1][1] + r.m[0][2] * m[2][1] + r.m[0][3] * m[3][1],
			r.m[0][0] * m[0][2] + r.m[0][1] * m[1][2] + r.m[0][2] * m[2][2] + r.m[0][3] * m[3][2],
			r.m[0][0] * m[0][3] + r.m[0][1] * m[1][3] + r.m[0][2] * m[2][3] + r.m[0][3] * m[3][3],
			r.m[1][0] * m[0][0] + r.m[1][1] * m[1][0] + r.m[1][2] * m[2][0] + r.m[1][3] * m[3][0],
			r.m[1][0] * m[0][1] + r.m[1][1] * m[1][1] + r.m[1][2] * m[2][1] + r.m[1][3] * m[3][1],
			r.m[1][0] * m[0][2] + r.m[1][1] * m[1][2] + r.m[1][2] * m[2][2] + r.m[1][3] * m[3][2],
			r.m[1][0] * m[0][3] + r.m[1][1] * m[1][3] + r.m[1][2] * m[2][3] + r.m[1][3] * m[3][3],
			r.m[2][0] * m[0][0] + r.m[2][1] * m[1][0] + r.m[2][2] * m[2][0] + r.m[2][3] * m[3][0],
			r.m[2][0] * m[0][1] + r.m[2][1] * m[1][1] + r.m[2][2] * m[2][1] + r.m[2][3] * m[3][1],
			r.m[2][0] * m[0][2] + r.m[2][1] * m[1][2] + r.m[2][2] * m[2][2] + r.m[2][3] * m[3][2],
			r.m[2][0] * m[0][3] + r.m[2][1] * m[1][3] + r.m[2][2] * m[2][3] + r.m[2][3] * m[3][3],
			r.m[3][0] * m[0][0] + r.m[3][1] * m[1][0] + r.m[3][2] * m[2][0] + r.m[3][3] * m[3][0],
			r.m[3][0] * m[0][1] + r.m[3][1] * m[1][1] + r.m[3][2] * m[2][1] + r.m[3][3] * m[3][1],
			r.m[3][0] * m[0][2] + r.m[3][1] * m[1][2] + r.m[3][2] * m[2][2] + r.m[3][3] * m[3][2],
			r.m[3][0] * m[0][3] + r.m[3][1] * m[1][3] + r.m[3][2] * m[2][3] + r.m[3][3] * m[3][3]
		};
	}
	Vec4 Mat4::operator*( const Vec4& rhs ) const
	{
		return {
			m[0][0] * rhs.x + m[0][1] * rhs.y + m[0][2] * rhs.z + m[0][3] * rhs.w,
			m[1][0] * rhs.x + m[1][1] * rhs.y + m[1][2] * rhs.z + m[1][3] * rhs.w,
			m[2][0] * rhs.x + m[2][1] * rhs.y + m[2][2] * rhs.z + m[2][3] * rhs.w,
			m[3][0] * rhs.x + m[3][1] * rhs.y + m[3][2] * rhs.z + m[3][3] * rhs.w
		};
	}
	Vec3 Mat4::operator*( const Vec3& rhs ) const
	{
		return Transform( *this, rhs );
	}
	Plane Mat4::operator*( const Plane& rhs ) const
	{
		Plane res;
		res.n = TransformNormal( *this, rhs.n );
		res.d = rhs.d * rhs.n.LengthSq();
		res.d += Vec3::Dot( res.n, GetTranslation() );

		return res;
	}

	AABB::AABB( const Vec3& mins, const Vec3& maxs )
		:
		mins( mins ),
		maxs( maxs )
	{}
	AABB::AABB( const Vec3* points, size_t num_points )
	{
		mins = { FLT_MAX, FLT_MAX, FLT_MAX };
		maxs = { FLT_MIN, FLT_MIN, FLT_MIN };

		for( size_t i = 0; i < num_points; i++ )
		{
			const Vec3& point = points[i];

			if( point.x < mins.x ) mins.x = point.x;
			if( point.y < mins.y ) mins.y = point.y;
			if( point.z < mins.z ) mins.z = point.z;

			if( point.x > maxs.x ) maxs.x = point.x;
			if( point.y > maxs.y ) maxs.y = point.y;
			if( point.z > maxs.z ) maxs.z = point.z;
		}
	}
	AABB AABB::Transform( const Mat4& transform ) const
	{
		Vec3 transformed_mins = transform * mins;
		Vec3 transformed_maxs = transform * maxs;

		AABB aabb;
		aabb.mins = { FLT_MAX, FLT_MAX, FLT_MAX };
		aabb.maxs = { FLT_MIN, FLT_MIN, FLT_MIN };
		if( transformed_mins.x < aabb.mins.x ) aabb.mins.x = transformed_mins.x;
		if( transformed_mins.y < aabb.mins.y ) aabb.mins.y = transformed_mins.y;
		if( transformed_mins.z < aabb.mins.z ) aabb.mins.z = transformed_mins.z;
		if( transformed_maxs.x < aabb.mins.x ) aabb.mins.x = transformed_maxs.x;
		if( transformed_maxs.y < aabb.mins.y ) aabb.mins.y = transformed_maxs.y;
		if( transformed_maxs.z < aabb.mins.z ) aabb.mins.z = transformed_maxs.z;
		if( transformed_mins.x > aabb.maxs.x ) aabb.maxs.x = transformed_mins.x;
		if( transformed_mins.y > aabb.maxs.y ) aabb.maxs.y = transformed_mins.y;
		if( transformed_mins.z > aabb.maxs.z ) aabb.maxs.z = transformed_mins.z;
		if( transformed_maxs.x > aabb.maxs.x ) aabb.maxs.x = transformed_maxs.x;
		if( transformed_maxs.y > aabb.maxs.y ) aabb.maxs.y = transformed_maxs.y;
		if( transformed_maxs.z > aabb.maxs.z ) aabb.maxs.z = transformed_maxs.z;

		return aabb;
	}
	void AABB::GetPoints( Vec3 out[8] ) const
	{
		out[0] = { mins.x, mins.y, mins.z };
		out[1] = { mins.x, mins.y, maxs.z };
		out[2] = { maxs.x, mins.y, maxs.z };
		out[3] = { maxs.x, mins.y, mins.z };
		out[4] = { maxs.x, maxs.y, maxs.z };
		out[5] = { mins.x, maxs.y, maxs.z };
		out[6] = { mins.x, maxs.y, mins.z };
		out[7] = { maxs.x, maxs.y, mins.z };
	}
}