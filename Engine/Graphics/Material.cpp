#include "PCH.h"
#include "Material.h"

#include "IGraphics.h"

namespace Bat
{
	IPipeline* Bat::Material::GetDefaultPipeline() const
	{
		if( m_pBumpMap != nullptr )
		{
			return g_pGfx->GetPipeline( "bumpmap" );
		}
		else
		{
			return g_pGfx->GetPipeline( "light" );
		}
	}
}
