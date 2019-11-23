#include "PCH.h"
#include "Light.h"

#include "imgui.h"

namespace Bat
{
	void LightComponent::DoImGuiMenu()
	{
		if( ImGui::TreeNode( "Light" ) )
		{
			ImGui::Checkbox( "Enabled", &m_bEnabled );
			ImGui::Combo( "Type", (int*)&m_Type, "Point\0Directional\0Spot" );
			ImGui::DragFloat( "Intensity", &m_flIntensity, 0.1f, 0.0f, 1000.0f );
			ImGui::ColorEdit3( "Colour", &m_colColour.x );

			if( GetType() == LightType::DIRECTIONAL || GetType() == LightType::SPOT )
			{
				ImGui::InputFloat3( "Direction", &m_vecDirection.x );
			}

			if( GetType() == LightType::POINT || GetType() == LightType::SPOT )
			{
				ImGui::DragFloat( "Range", &m_flRange, 0.1f, 0.0f, 1000.0f );
			}

			if( GetType() == LightType::SPOT )
			{
				ImGui::DragFloat( "Spotlight Angle", &m_flSpotlightAngle, 0.1f, 0.0f, 180.0f );
			}


			ImGui::TreePop();
		}
	}
}