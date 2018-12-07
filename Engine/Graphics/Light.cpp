#include "PCH.h"
#include "Light.h"

Bat::Light::Light( const Vec3& pos )
	:
	m_vecPosition( pos )
{}

Bat::Light::Light( const Vec3& pos, const Vec3& ambient, const Vec3& diffuse, const Vec3& specular )
	:
	m_vecPosition( pos ),
	m_colAmbient( ambient ),
	m_colDiffuse( diffuse ),
	m_colSpecular( specular )
{}
