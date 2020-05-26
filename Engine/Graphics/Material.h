#pragma once

#include "Core/ResourceManager.h"
#include "Texture.h"

namespace Bat
{
	class IPipeline;

	enum class AlphaMode
	{
		NONE, // Opaque
		MASK, // Use alpha cutoff
		BLEND // Alpha blending
	};

	class Material
	{
	public:
		Resource<Texture> GetBaseColour() const { return m_pBaseColour; }
		void SetBaseColour( Resource<Texture> pTexture ) { m_pBaseColour = pTexture; }

		Resource<Texture> GetMetallicRoughness() const { return m_pMetallicRoughness; }
		void SetMetallicRoughness( Resource<Texture> pTexture ) { m_pMetallicRoughness = pTexture; }

		Resource<Texture> GetNormalMap() const { return m_pNormalMap; }
		void SetNormalMap( Resource<Texture> pTexture ) { m_pNormalMap = pTexture; }

		Resource<Texture> GetOcclusionMap() const { return m_pOcclusionMap; }
		void SetOcclusionMap( Resource<Texture> pTexture ) { m_pOcclusionMap = pTexture; }

		Resource<Texture> GetEmissiveMap() const { return m_pEmissiveMap; }
		void SetEmissiveMap( Resource<Texture> pTexture ) { m_pEmissiveMap = pTexture; }

		Vec4 GetBaseColourFactor() const { return m_vecBaseColourFactor; }
		void SetBaseColourFactor( float r, float g, float b, float a ) { m_vecBaseColourFactor = { r, g, b, a }; }
		float GetMetallicFactor() const { return m_flMetallicFactor; }
		void SetMetallicFactor( float metallic ) { m_flMetallicFactor = metallic; }
		float GetRoughnessFactor() const { return m_flRoughnessFactor; }
		void SetRoughnessFactor( float roughness ) { m_flRoughnessFactor = roughness; }
		Vec3 GetEmissiveFactor() const { return m_vecEmissiveFactor; }
		void SetEmissiveFactor( float r, float g, float b ) { m_vecEmissiveFactor = { r, g, b }; }

		AlphaMode GetAlphaMode() const { return m_AlphaMode; }
		void SetAlphaMode( AlphaMode mode ) { m_AlphaMode = mode; }
		float GetAlphaCutoff() const { return m_flAlphaCutoff; }
		void SetAlphaCutoff( float cutoff ) { m_flAlphaCutoff = cutoff; }
	private:
		Vec4 m_vecBaseColourFactor = { 0.0f, 0.0f, 0.0f, 0.0f };
		Vec3 m_vecEmissiveFactor = { 0.0f, 0.0f, 0.0f };
		float m_flMetallicFactor = 0.0f;
		float m_flRoughnessFactor = 0.0f;

		Resource<Texture> m_pBaseColour = nullptr;
		Resource<Texture> m_pMetallicRoughness = nullptr;
		Resource<Texture> m_pNormalMap = nullptr;
		Resource<Texture> m_pOcclusionMap = nullptr;
		Resource<Texture> m_pEmissiveMap = nullptr;

		AlphaMode m_AlphaMode = AlphaMode::NONE;
		float m_flAlphaCutoff = 0.5f;
	};
}