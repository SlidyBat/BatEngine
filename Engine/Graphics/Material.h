#pragma once

#include "ResourceManager.h"

namespace Bat
{
	class Texture;
	class IPipeline;

	class Material
	{
	public:
		Resource<Texture> GetAmbientTexture() const { return m_pAmbient; }
		void SetAmbientTexture( Resource<Texture> pTexture ) { m_pAmbient = pTexture; }

		Resource<Texture> GetDiffuseTexture() const { return m_pDiffuse; }
		void SetDiffuseTexture( Resource<Texture> pTexture ) { m_pDiffuse = pTexture; }

		Resource<Texture> GetSpecularTexture() const { return m_pSpecular; }
		void SetSpecularTexture( Resource<Texture> pTexture ) { m_pSpecular = pTexture; }

		Resource<Texture> GetEmissiveTexture() const { return m_pEmissive; }
		void SetEmissiveTexture( Resource<Texture> pTexture ) { m_pEmissive = pTexture; }

		Resource<Texture> GetNormalTexture() const { return m_pNormalMap; }
		void SetNormalTexture( Resource<Texture> pTexture ) { m_pNormalMap = pTexture; }

		Vec3 GetAmbientColour() const { return m_colAmbient; }
		void SetAmbientColour( float r, float g, float b ) { m_colAmbient = { r, g, b }; }
		Vec3 GetDiffuseColour() const { return m_colDiffuse; }
		void SetDiffuseColour( float r, float g, float b ) { m_colDiffuse = { r, g, b }; }
		Vec3 GetSpecularColour() const { return m_colSpecular; }
		void SetSpecularColour( float r, float g, float b ) { m_colSpecular = { r, g, b }; }
		Vec3 GetEmissiveColour() const { return m_colEmissive; }
		void SetEmissiveColour( float r, float g, float b ) { m_colEmissive = { r, g, b }; }

		float GetShininess() const { return m_fShininess; }
		void SetShininess( const float shininess ) { m_fShininess = shininess; }
	private:
		Vec3 m_colAmbient = { 0.0f, 0.0f, 0.0f };
		Vec3 m_colDiffuse = { 0.0f, 0.0f, 0.0f };
		Vec3 m_colSpecular = { 0.0f, 0.0f, 0.0f };
		Vec3 m_colEmissive = { 0.0f, 0.0f, 0.0f };

		Resource<Texture> m_pAmbient = nullptr;
		Resource<Texture> m_pDiffuse = nullptr;
		Resource<Texture> m_pSpecular = nullptr;
		Resource<Texture> m_pEmissive = nullptr;
		Resource<Texture> m_pNormalMap = nullptr;
		float m_fShininess = 32.0f;
	};
}