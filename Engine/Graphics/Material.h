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

		Resource<Texture> GetBumpMapTexture() const { return m_pBumpMap; }
		void SetBumpMapTexture( Resource<Texture> pTexture ) { m_pBumpMap = pTexture; }

		float GetShininess() const { return m_fShininess; }
		void SetShininess( const float shininess ) { m_fShininess = shininess; }

		IPipeline* GetDefaultPipeline() const;
	private:
		Resource<Texture> m_pAmbient = nullptr;
		Resource<Texture> m_pDiffuse = nullptr;
		Resource<Texture> m_pSpecular = nullptr;
		Resource<Texture> m_pEmissive = nullptr;
		Resource<Texture> m_pBumpMap = nullptr;
		float m_fShininess = 32.0f;
	};
}