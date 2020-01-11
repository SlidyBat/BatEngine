#pragma once

#include "PCH.h"
#include "Entity.h"
#include "IGPUDevice.h"

namespace Bat
{
	static constexpr int MAX_SHADOW_SOURCES = 16;
	static constexpr size_t INVALID_SHADOW_MAP_INDEX = ~0;

	enum class LightType
	{
		POINT = 0,
		DIRECTIONAL,
		SPOT
	};

	enum class LightFlags
	{
		NONE = 0,
		EMIT_SHADOWS = (1 << 0)
	};
	BAT_ENUM_OPERATORS( LightFlags );

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

		size_t GetShadowIndex() const { return m_iShadowIndex; }
		void SetShadowIndex( size_t index ) { m_iShadowIndex = index; }

		LightComponent& AddFlag( LightFlags flag ) { m_Flags |= flag; return *this; }
		LightComponent& RemoveFlag( LightFlags flag ) { m_Flags &= ~flag; return *this; }
		bool HasFlag( LightFlags flag ) { return ( m_Flags & flag ) == flag; }

		void DoImGuiMenu();
	private:
		bool m_bEnabled = true;
		Vec3 m_vecDirection = { 0.0f, 0.0f, 0.0f };
		float m_flSpotlightAngle = 1.0f;
		LightType m_Type = LightType::POINT;
		float m_flIntensity = 1.0f;
		Vec3 m_colColour = { 1.0f, 1.0f, 1.0f };
		float m_flRange = 10.0f;
		LightFlags m_Flags;
		size_t m_iShadowIndex = INVALID_SHADOW_MAP_INDEX;
	};
}