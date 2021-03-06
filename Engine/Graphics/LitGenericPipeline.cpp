#include "PCH.h"

#include "Core/CoreEntityComponents.h"
#include "Core/Globals.h"
#include "Camera.h"
#include "LitGenericPipeline.h"
#include "VertexTypes.h"
#include "COMException.h"
#include "Renderer.h"
#include "Texture.h"
#include "Light.h"
#include "Mesh.h"
#include "Material.h"
#include "ShaderManager.h"

namespace Bat
{
	void LitGenericPipeline::Render( IGPUContext* pContext,
		const Mesh& mesh,
		const Camera& camera,
		const Mat4& world_transform,
		const std::vector<Entity>& light_ents,
		const std::vector<Mat4>& light_transforms,
		const PbrGlobalMaps& maps )
	{
		ShaderMacro macros[MAX_SHADER_MACROS];
		size_t num_macros = ShaderManager::BuildMacrosForMesh( mesh, macros, MAX_SHADER_MACROS );

		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "../Engine/Graphics/Shaders/LitGenericVS.hlsl", macros, num_macros );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "../Engine/Graphics/Shaders/LitGenericPS.hlsl", macros, num_macros );

		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );

		BindTransforms( pContext, camera, world_transform );

		mesh.Bind( pContext, pVertexShader );
		BindMaterial( pContext, mesh.GetMaterial() );
		BindLights( pContext, light_ents, light_transforms );

		if( maps.irradiance_map )
		{
			pContext->BindTexture( maps.irradiance_map, PS_TEX_SLOT_5 );
		}
		if( maps.prefilter_map )
		{
			pContext->BindTexture( maps.prefilter_map, PS_TEX_SLOT_6 );
		}
		if( maps.brdf_integration_map )
		{
			pContext->BindTexture( maps.brdf_integration_map, PS_TEX_SLOT_7 );
		}

		pContext->DrawIndexed( mesh.GetIndexCount() );
	}

	void LitGenericPipeline::RenderInstanced( IGPUContext* pContext,
		const Mesh& mesh,
		const std::vector<LitGenericInstanceData>& instances,
		const Camera& camera,
		const std::vector<Entity>& light_ents,
		const std::vector<Mat4>& light_transforms,
		const PbrGlobalMaps& maps )
	{
		ShaderMacro macros[MAX_SHADER_MACROS];
		size_t num_macros = ShaderManager::BuildMacrosForInstancedMesh( mesh, macros, MAX_SHADER_MACROS );

		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "../Engine/Graphics/Shaders/LitGenericVS.hlsl", macros, num_macros );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "../Engine/Graphics/Shaders/LitGenericPS.hlsl", macros, num_macros );

		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );

		BindTransforms( pContext, camera, Mat4::Identity() );

		mesh.Bind( pContext, pVertexShader );
		BindMaterial( pContext, mesh.GetMaterial() );
		BindLights( pContext, light_ents, light_transforms );
		BindInstances( pContext, pVertexShader, instances );

		pContext->BindTexture( maps.irradiance_map, PS_TEX_SLOT_5 );

		pContext->DrawInstancedIndexed( mesh.GetIndexCount(), instances.size() );
	}

	void LitGenericPipeline::BindTransforms( IGPUContext* pContext,
		const Camera& camera,
		const Mat4& world_transform )
	{
		{
			CB_LitGenericPipelineMatrix transform;
			transform.world = world_transform;
			transform.viewproj = camera.GetViewMatrix() * camera.GetProjectionMatrix();
			m_cbufTransform.Update( pContext, transform );
			pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, VS_CBUF_TRANSFORMS );
		}
	}

	void LitGenericPipeline::BindMaterial( IGPUContext* pContext, const Material& material )
	{
		// Bind material cbuffer
		{
			CB_LitGenericPipelineMaterial cbuf;

			cbuf.mat.BaseColourFactor = material.GetBaseColourFactor();
			cbuf.mat.EmissiveFactor   = material.GetEmissiveFactor();
			cbuf.mat.MetallicFactor   = material.GetMetallicFactor();
			cbuf.mat.RoughnessFactor  = material.GetRoughnessFactor();

			cbuf.mat.HasBaseColourTexture        = ( material.GetBaseColour() != nullptr );
			cbuf.mat.HasMetallicRoughnessTexture = ( material.GetMetallicRoughness() != nullptr );
			cbuf.mat.HasNormalTexture            = ( material.GetNormalMap() != nullptr );
			cbuf.mat.HasOcclusionTexture         = ( material.GetOcclusionMap() != nullptr );
			cbuf.mat.HasEmissiveTexture          = ( material.GetEmissiveMap() != nullptr );

			cbuf.mat.AlphaCutoff = material.GetAlphaCutoff();

			m_cbufMaterial.Update( pContext, cbuf );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufMaterial, PS_CBUF_SLOT_0 );
		}

		// Bind material textures
		{
			if( auto tex = material.GetBaseColour() )        pContext->BindTexture( *tex, PS_TEX_SLOT_0 );
			if( auto tex = material.GetMetallicRoughness() ) pContext->BindTexture( *tex, PS_TEX_SLOT_1 );
			if( auto tex = material.GetNormalMap() )         pContext->BindTexture( *tex, PS_TEX_SLOT_2 );
			if( auto tex = material.GetOcclusionMap() )      pContext->BindTexture( *tex, PS_TEX_SLOT_3 );
			if( auto tex = material.GetEmissiveMap() )       pContext->BindTexture( *tex, PS_TEX_SLOT_4 );
		}
	}

	void LitGenericPipeline::BindLights( IGPUContext* pContext, const std::vector<Entity>& light_ents, const std::vector<Mat4>& light_transforms )
	{
		CB_LitGenericPipelineLights lights;
		size_t j = 0;
		for( size_t i = 0; i < light_ents.size(); i++ )
		{
			if( j == MAX_LIGHTS )
			{
				break;
			}

			const auto& l = light_ents[i].Get<LightComponent>();
			const auto& t = light_ents[i].Get<TransformComponent>();

			if( !l.IsEnabled() )
			{
				continue;
			}

			Mat4 rot = Mat4::RotateDeg( t.GetRotation() );

			lights.lights[j].Position = light_transforms[i].GetTranslation();
			lights.lights[j].ShadowIndex = (int)l.GetShadowIndex();
			lights.lights[j].Direction = Mat4::TransformNormal( rot, { 0.0f, 0.0f, 1.0f } );
			lights.lights[j].SpotlightAngle = l.GetSpotlightAngle();
			lights.lights[j].Colour = l.GetColour();
			lights.lights[j].Range = l.GetRange();
			lights.lights[j].Intensity = l.GetIntensity();
			lights.lights[j].Type = (int)l.GetType();

			j++;
		}
		lights.num_lights = (uint32_t)j;
		m_cbufLightParams.Update( pContext, lights );
		pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufLightParams, PS_CBUF_SLOT_1 );
	}
	void LitGenericPipeline::BindInstances( IGPUContext* pContext, IVertexShader* pVertexShader, const std::vector<LitGenericInstanceData>& instances )
	{
		if( !m_vbufInstanceData )
		{
			m_vbufInstanceData.Reset( instances );
		}
		else if( m_vbufInstanceData->GetVertexCount() < instances.size() )
		{
			m_vbufInstanceData.Reset( instances );
		}
		else
		{
			m_vbufInstanceData.Update( pContext, instances.data(), instances.size() );
		}

		int slot = pVertexShader->GetVertexAttributeSlot( VertexAttribute::InstanceData, 0 );
		ASSERT( slot != -1, "Shader does not allow instanced drawing" );

		pContext->SetVertexBuffer( m_vbufInstanceData, slot );
	}
}