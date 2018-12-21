#include "PCH.h"
#include "Material.h"

#include "Graphics.h"

namespace Bat
{
	const char* Bat::Material::GetDefaultPipelineName() const
	{
		if( m_pBumpMap != nullptr )
		{
			return "bumpmap";
		}
		else
		{
			return "light";
		}
	}
}
