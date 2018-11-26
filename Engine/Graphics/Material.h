#pragma once

namespace Bat
{
	class Texture;

	class Material
	{
	public:
		Texture* GetAmbientTexture() const { return m_pAmbient; }
		void SetAmbientTexture( Texture* pTexture ) { m_pAmbient = pTexture; }

		Texture* GetDiffuseTexture() const { return m_pDiffuse; }
		void SetDiffuseTexture( Texture* pTexture ) { m_pDiffuse = pTexture; }

		Texture* GetSpecularTexture() const { return m_pSpecular; }
		void SetSpecularTexture( Texture* pTexture ) { m_pSpecular = pTexture; }

		Texture* GetEmissiveTexture() const { return m_pEmissive; }
		void SetEmissiveTexture( Texture* pTexture ) { m_pEmissive = pTexture; }

		Texture* GetBumpMapTexture() const { return m_pBumpMap; }
		void SetBumpMapTexture( Texture* pTexture ) { m_pBumpMap = pTexture; }

		float GetShininess() const { return m_fShininess; }
		void SetShininess( const float shininess ) { m_fShininess = shininess; }
	private:
		Texture* m_pAmbient = nullptr;
		Texture* m_pDiffuse = nullptr;
		Texture* m_pSpecular = nullptr;
		Texture* m_pEmissive = nullptr;
		Texture* m_pBumpMap = nullptr;
		float m_fShininess = 32.0f;
	};
}