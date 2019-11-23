#pragma once

#include "PCH.h"
#include "Entity.h"

namespace Bat
{
	enum class LightType
	{
		POINT = 0,
		DIRECTIONAL,
		SPOT
	};

	class LightComponent : public Component<LightComponent>
	{
	public:
		LightComponent() = default;

		LightType GetType() const { return m_Type; }
		LightComponent& SetType( LightType type ) { m_Type = type; return *this; }

		bool IsEnabled() const { return m_bEnabled; }
		LightComponent& SetEnabled( bool enabled ) { m_bEnabled = enabled; return *this; }

		Vec3 GetDirection() const { return m_vecDirection; }
		LightComponent& SetDirection( const Vec3& dir ) { m_vecDirection = dir; return *this; }
		float GetSpotlightAngle() const { return m_flSpotlightAngle; }
		LightComponent& SetSpotlightAngle( float ang ) { m_flSpotlightAngle = ang; return *this; }
		Vec3 GetColour() const { return m_colColour; }
		LightComponent& SetColour( const Vec3& colour ) { m_colColour = colour; return *this; }

		float GetRange() const { return m_flRange; }
		LightComponent& SetRange( float attenuation ) { m_flRange = attenuation; return *this; }

		float GetIntensity() const { return m_flIntensity; }
		LightComponent& SetIntensity( float intensity ) { m_flIntensity = intensity; return *this; }

		void DoImGuiMenu();
	private:
		bool m_bEnabled = true;
		Vec3 m_vecDirection = { 0.0f, 0.0f, 0.0f };
		float m_flSpotlightAngle = 1.0f;
		LightType m_Type = LightType::POINT;
		float m_flIntensity = 1.0f;
		Vec3 m_colColour = { 1.0f, 1.0f, 1.0f };
		float m_flRange = 10.0f;
	};
}