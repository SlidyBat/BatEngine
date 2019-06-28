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

	class Light
	{
	public:
		Light() = default;
		
		LightType GetType() const { return m_Type; }
		void SetType( LightType type ) { m_Type = type; }

		bool IsEnabled() const { return m_bEnabled; }
		void SetEnabled( bool enabled ) { m_bEnabled = enabled; }

		Vec3 GetPosition() const { return m_vecPosition; }
		void SetPosition( const Vec3& pos ) { m_vecPosition = pos; }
		Vec3 GetDirection() const { return m_vecDirection; }
		void SetDirection( const Vec3& dir ) { m_vecDirection = dir; }
		float GetSpotlightAngle() const { return m_flSpotlightAngle; }
		void SetSpotlightAngle( float ang ) { m_flSpotlightAngle = ang; }
		Vec3 GetColour() const { return m_colColour; }
		void SetColour( const Vec3& colour ) { m_colColour = colour; }

		float GetRange() const { return m_flRange; }
		void SetRange( float attenuation ) { m_flRange = attenuation; }

		float GetIntensity() const { return m_flIntensity; }
		void SetIntensity( float intensity ) { m_flIntensity = intensity; }
	private:
		Vec3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		bool m_bEnabled = true;
		Vec3 m_vecDirection = { 0.0f, 0.0f, 0.0f };
		float m_flSpotlightAngle = 1.0f;
		LightType m_Type = LightType::POINT;
		float m_flIntensity = 1.0f;
		Vec3 m_colColour = { 1.0f, 1.0f, 1.0f };
		float m_flRange = 1000.0f;
	};

	struct LightComponent : public Component<LightComponent>
	{
		LightComponent() = default;
		LightComponent( Light light )
			:
			light( light )
		{}

		Light light;
	};
}