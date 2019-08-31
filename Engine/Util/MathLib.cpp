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

		Vec3 QuaternionToEuler( const Vec4& quat )
		{
			Vec3 angle;

			float sinr_cosp = +2.0f * (quat.w * quat.x + quat.y * quat.z);
			float cosr_cosp = +1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y);
			angle.z = atan2( sinr_cosp, cosr_cosp );

			float sinp = +2.0f * (quat.w * quat.y - quat.z * quat.x);
			if( fabs( sinp ) >= 1.0f )
			{
				angle.x = copysign( Math::PI / 2, sinp );
			}
			else
			{
				angle.x = asin( sinp );
			}

			float siny_cosp = +2.0f * (quat.w * quat.z + quat.x * quat.y);
			float cosy_cosp = +1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z);
			angle.y = atan2( siny_cosp, cosy_cosp );

			return angle;
		}

		Vec4 EulerToQuaternion( const Vec3& euler )
		{
			float cp = cosf( euler.x * 0.5f );
			float sp = sinf( euler.x * 0.5f );
			float cy = cosf( euler.y * 0.5f );
			float sy = sinf( euler.y * 0.5f );
			float cr = cosf( euler.z * 0.5f );
			float sr = sinf( euler.z * 0.5f );

			Vec4 quat;
			quat.w = cy * cp * cr + sy * sp * sr;
			quat.x = cy * cp * sr - sy * sp * cr;
			quat.y = sy * cp * sr + cy * sp * cr;
			quat.z = sy * cp * cr - cy * sp * sr;

			return quat;
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

		void AngleVectors( const Vec3& angles, Vec3* forward, Vec3* right, Vec3* up )
		{
			float sr, sp, sy, cr, cp, cy;

			SinCos( DegToRad( angles.x ), &sy, &cy );
			SinCos( DegToRad( angles.y ), &sp, &cp );
			SinCos( DegToRad( angles.z ), &sr, &cr );

			if( forward )
			{
				forward->x = cp * cy;
				forward->y = cp * sy;
				forward->z = -sp;
			}

			if( right )
			{
				right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
				right->y = (-1 * sr * sp * sy + -1 * cr * cy);
				right->z = -1 * sr * cp;
			}

			if( up )
			{
				up->x = (cr * sp * cy + -sr * -sy);
				up->y = (cr * sp * sy + -sr * cy);
				up->z = cr * cp;
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
	}
}