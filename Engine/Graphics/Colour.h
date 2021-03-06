#pragma once

#include "imgui_color_gradient.h"

namespace Bat
{
	class Colour
	{
	public:
		constexpr Colour()
			:
			value()
		{}
		constexpr Colour( unsigned int val )
			:
			value( val )
		{}
		constexpr Colour( int r, int g, int b, int a = 255 )
			:
			Colour( (a << 24u) | (r << 16u) | (g << 8u) | b )
		{}
		constexpr Colour( const Colour& src )
			:
			value( src.value )
		{}
		Colour& operator=( const Colour& src )
		{
			value = src.value;
			return *this;
		}
		bool operator==( const Colour& rhs ) const
		{
			return (value << 8u) == (rhs.value << 8u);
		}
		bool operator!=( const Colour& rhs ) const
		{
			return !(*this == rhs);
		}

		constexpr unsigned char GetA() const
		{
			return (value >> 24u);
		}
		void SetA( unsigned char a )
		{
			value = (value & 0x00FFFFFF) | (a << 24u);
		}

		constexpr unsigned char GetR() const
		{
			return (value >> 16u) & 0xFFu;
		}
		void SetR( unsigned char r )
		{
			value = (value & 0xFF00FFFF) | (r << 16u);
		}

		constexpr unsigned char GetG() const
		{
			return (value >> 8u) & 0xFFu;
		}
		void SetG( unsigned char g )
		{
			value = (value & 0xFFFF00FF) | (g << 8u);
		}

		constexpr unsigned char GetB() const
		{
			return (value) & 0xFFu;
		}
		void SetB( unsigned char b )
		{
			value = (value & 0xFFFFFF00) | b;
		}

		constexpr unsigned int GetValue() const
		{
			return value;
		}
		constexpr void SetValue( unsigned int newval )
		{
			value = newval;
		}

		Vec4 AsVector() const
		{
			return Vec4( GetR() / 255.0f, GetG() / 255.0f, GetB() / 255.0f, GetA() / 255.0f );
		}
		static Colour FromVector( const Vec4& vec )
		{
			return Colour( (int)(vec.x * 255.0f), (int)(vec.y * 255.0f), (int)(vec.z * 255.0f), (int)(vec.w * 255.0f) );
		}

		static Colour Lerp( Colour a, Colour b, float t )
		{
			Vec4 a_vec = a.AsVector();
			Vec4 b_vec = b.AsVector();
			return Colour::FromVector( Vec4::Lerp( a_vec, b_vec, t ) );
		}
	private:
		unsigned int value;
	};

	namespace Colours
	{
		static constexpr Colour Black = Colour( 0, 0, 0 );
		static constexpr Colour White = Colour( 255, 255, 255 );
		static constexpr Colour Red = Colour( 255, 0, 0 );
		static constexpr Colour Green = Colour( 0, 255, 0 );
		static constexpr Colour Blue = Colour( 0, 0, 255 );
	}

	class Gradient
	{
	private:
		struct GradientStop
		{
			Colour c;
			float t;
		};
	public:
		Gradient& AddStop( Colour c, float t )
		{
			std::vector<GradientStop>::iterator it;
			for( it = m_Stops.begin(); it != m_Stops.end(); ++it )
			{
				if( it->t > t )
				{
					break;
				}
			}

			GradientStop stop;
			stop.c = c;
			stop.t = t;
			m_Stops.insert( it, stop );

			SyncWithImgui();

			return *this;
		}

		Colour Get( float t ) const
		{
			if( m_Stops.empty() )
			{
				return Colours::Black;
			}

			if( t <= m_Stops.front().t )
			{
				return m_Stops.front().c;
			}
			if( t >= m_Stops.back().t )
			{
				return m_Stops.back().c;
			}

			const GradientStop* pPrevStop = &m_Stops[0];
			const GradientStop* pNextStop = nullptr;
			for( size_t i = 1; i < m_Stops.size(); i++ )
			{
				pNextStop = &m_Stops[i];
				if( m_Stops[i].t > t )
				{
					break;
				}

				pPrevStop = pNextStop;
			}

			if( pNextStop == nullptr )
			{
				return pPrevStop->c;
			}
			if( pPrevStop == &m_Stops.back() )
			{
				return pPrevStop->c;
			}

			float range = pNextStop->t - pPrevStop->t;
			float actual_t = (t - pPrevStop->t) / range;
			return Colour::Lerp( pPrevStop->c, pNextStop->c, actual_t );
		}

		void DoImGuiButton()
		{
			if( ImGui::GradientButton( &m_ImguiGradient ) )
			{
				m_bEditorShowing = !m_bEditorShowing;
				if( m_bEditorShowing )
				{
					SyncWithImgui();
				}
			}

			if( m_bEditorShowing )
			{
				if( ImGui::Button( "Delete Selected Stop" ) )
				{
					m_ImguiGradient.removeMark( m_pSelectedMark );
				}
				if( ImGui::GradientEditor( &m_ImguiGradient, m_pDraggingMark, m_pSelectedMark ) )
				{
					m_Stops.clear();
					const std::list<ImGradientMark*>& pMarks = m_ImguiGradient.getMarks();
					for( auto it = pMarks.begin(); it != pMarks.end(); ++it )
					{
						ImGradientMark& mark = **it;
						GradientStop stop;
						stop.c = Colour::FromVector( Vec4{ mark.color[0], mark.color[1], mark.color[2], 1.0f } );
						stop.t = mark.position;
						m_Stops.push_back( stop );
					}
				}
			}
		}
	private:
		void SyncWithImgui()
		{
			m_ImguiGradient.reset();

			for( const GradientStop& stop : m_Stops )
			{
				m_ImguiGradient.addMark( stop.t, ImColor( stop.c.GetR(), stop.c.GetG(), stop.c.GetB(), stop.c.GetA() ) );
			}
		}
	private:
		std::vector<GradientStop> m_Stops;
		ImGradient m_ImguiGradient;
		ImGradientMark* m_pDraggingMark = nullptr;
		ImGradientMark* m_pSelectedMark = nullptr;
		bool m_bEditorShowing = false;
	};
}