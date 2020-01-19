#include "PCH.h"

#include "CoreEntityComponents.h"
#include "Camera.h"
#include "LitGenericPipeline.h"
#include "VertexTypes.h"
#include "COMException.h"
#include "Graphics.h"
#include "Globals.h"
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
		const DirectX::XMMATRIX& world_transform,
		const std::vector<Entity>& light_ents,
		const std::vector<DirectX::XMMATRIX>& light_transforms )
	{
		auto macros = ShaderManager::BuildMacrosForMesh( mesh );

		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/LitGenericVS.hlsl", macros );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/LitGenericPS.hlsl", macros );

		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );

		BindTransforms( pContext, camera, world_transform );

		mesh.Bind( pContext, pVertexShader );
		BindMaterial( pContext, mesh.GetMaterial() );
		BindLights( pContext, light_ents, light_transforms );

		pContext->DrawIndexed( mesh.GetIndexCount() );
	}

	void LitGenericPipeline::RenderInstanced( IGPUContext* pContext,
		const Mesh& mesh,
		const std::vector<LitGenericInstanceData>& instances,
		const Camera& camera,
		const std::vector<Entity>& light_ents,
		const std::vector<DirectX::XMMATRIX>& light_transforms )
	{
		auto macros = ShaderManager::BuildMacrosForInstancedMesh( mesh );

		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/LitGenericVS.hlsl", macros );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/LitGenericPS.hlsl", macros );

		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );

		BindTransforms( pContext, camera, DirectX::XMMatrixIdentity() );

		mesh.Bind( pContext, pVertexShader );
		BindMaterial( pContext, mesh.GetMaterial() );
		BindLights( pContext, light_ents, light_transforms );
		BindInstances( pContext, pVertexShader, instances );

		pContext->DrawInstancedIndexed( mesh.GetIndexCount(), instances.size() );
	}

	void LitGenericPipeline::BindTransforms( IGPUContext* pContext,
		const Camera& camera,
		const DirectX::XMMATRIX& world_transform )
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
		CB_LitGenericPipelineMaterial material_buf;
		material_buf.material.GlobalAmbient = { 1.0f, 1.0f, 1.0f };
		material_buf.material.AmbientColor = { 0.005f, 0.005f, 0.005f };
		material_buf.material.DiffuseColor = material.GetDiffuseColour();
		material_buf.material.SpecularColor = material.GetSpecularColour();
		material_buf.material.EmissiveColor = material.GetEmissiveColour();
		material_buf.material.Opacity = material.GetOpacity();

		material_buf.material.HasAmbientTexture = (material.GetAmbientTexture() != nullptr);
		material_buf.material.HasDiffuseTexture = (material.GetDiffuseTexture() != nullptr);
		material_buf.material.HasSpecularTexture = (material.GetSpecularTexture() != nullptr);
		material_buf.material.HasEmissiveTexture = (material.GetEmissiveTexture() != nullptr);
		material_buf.material.HasNormalTexture = (material.GetNormalTexture() != nullptr);
		material_buf.material.HasBumpTexture = false;
		material_buf.material.SpecularPower = material.GetShininess();
		m_cbufMaterial.Update( pContext, material_buf );
		pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufMaterial, PS_CBUF_SLOT_0 );


		if( auto tex = material.GetDiffuseTexture() )  pContext->BindTexture( *tex, PS_TEX_SLOT_0 );
		if( auto tex = material.GetSpecularTexture() ) pContext->BindTexture( *tex, PS_TEX_SLOT_1 );
		if( auto tex = material.GetEmissiveTexture() ) pContext->BindTexture( *tex, PS_TEX_SLOT_2 );
		if( auto tex = material.GetNormalTexture() )   pContext->BindTexture( *tex, PS_TEX_SLOT_3 );
		if( auto tex = material.GetAmbientTexture() )  pContext->BindTexture( *tex, PS_TEX_SLOT_4 );
	}

	void LitGenericPipeline::BindLights( IGPUContext* pContext, const std::vector<Entity>& light_ents, const std::vector<DirectX::XMMATRIX>& light_transforms )
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

			DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(
				Math::DegToRad( t.GetRotation().x ),
				Math::DegToRad( t.GetRotation().y ),
				Math::DegToRad( t.GetRotation().z ) );

			DirectX::XMVECTOR vs, vr, vp;
			DirectX::XMMatrixDecompose( &vs, &vr, &vp, light_transforms[i] );
			lights.lights[j].Position = vp;
			lights.lights[j].ShadowIndex = (int)l.GetShadowIndex();
			lights.lights[j].Direction = DirectX::XMVector3TransformNormal( DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ), rot );
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