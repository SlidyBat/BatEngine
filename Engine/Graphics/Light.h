#pragma once

#include "PCH.h"

namespace Bat
{
	class Light
	{
	public:
		Light() = default;
		Light( const Vec3& pos );
		Light( const Vec3& pos, const Vec3& ambient, const Vec3& diffuse, const Vec3& specular );
		
		Vec3 GetPosition() const { return m_vecPosition; }
		void SetPosition( const Vec3& pos ) { m_vecPosition = pos; }
		Vec3 GetAmbient() const { return m_colAmbient; }
		void SetAmbient( const Vec3& ambient ) { m_colAmbient = ambient; }
		Vec3 GetDiffuse() const { return m_colDiffuse; }
		void SetDiffuse( const Vec3& diffuse ) { m_colDiffuse = diffuse; }
		Vec3 GetSpecular() const { return m_colSpecular; }
		void SetSpecular( const Vec3& specular ) { m_colSpecular = specular; }
	private:
		Vec3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		Vec3 m_colAmbient = { 0.1f, 0.1f, 0.1f };
		Vec3 m_colDiffuse = { 0.5f, 0.5f, 0.5f };
		Vec3 m_colSpecular = { 1.0f, 1.0f, 1.0f };
	};
}