#pragma once

#include "SceneRenderPass.h"
#include "ShadowPipeline.h"
#include "ShaderManager.h"

namespace Bat
{
	class ShadowPass : public BaseRenderPass
	{
	public:
		static constexpr int SHADOW_MAP_WIDTH = 1024;
		static constexpr int SHADOW_MAP_HEIGHT = 1024;

		ShadowPass()
		{
			m_pShadowMaps = std::unique_ptr<IDepthStencil>( gpu->CreateDepthStencil( SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, TEX_FORMAT_R24G8_TYPELESS, MAX_SHADOW_SOURCES ) );
		}

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			m_pContext = pContext;

			ComparisonFunc original_comp_func = pContext->GetDepthComparisonFunc();

			pContext->SetRenderTarget( nullptr );
			pContext->SetDepthEnabled( true );
			pContext->SetDepthWriteEnabled( true );
			pContext->SetDepthComparisonFunc( ComparisonFunc::GREATER );
			pContext->SetBlendingEnabled( false );
			pContext->UnbindTextureSlot( PS_TEX_SHADOWMAPS );

			Viewport vp;
			vp.width = SHADOW_MAP_WIDTH;
			vp.height = SHADOW_MAP_HEIGHT;
			vp.min_depth = 0.0f;
			vp.max_depth = 1.0f;
			vp.top_left = { 0.0f, 0.0f };
			pContext->PushViewport( vp );

			IDepthStencil* original_depth = pContext->GetDepthStencil();
			CB_ShadowMatrices transforms;
			size_t shadow_map_counter = 0;

			for( Entity e : world )
			{
				if( e.Has<LightComponent>() )
				{
					auto& t = e.Get<TransformComponent>();
					auto& light = e.Get<LightComponent>();
					light.SetShadowIndex( INVALID_SHADOW_MAP_INDEX );
					
					if( !light.IsEnabled() || light.GetType() == LightType::POINT )
					{
						continue;
					}
					
					if( !light.HasFlag( LightFlags::EMIT_SHADOWS ) )
					{
						continue;
					}

					switch( light.GetType() )
					{
					case LightType::DIRECTIONAL:
					{
						DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(
							Math::DegToRad( t.GetRotation().x ),
							Math::DegToRad( t.GetRotation().y ),
							Math::DegToRad( t.GetRotation().z ) );
						Vec3 up = DirectX::XMVector3TransformNormal( { 0.0f, 1.0f, 0.0f }, rot );
						Vec3 to = DirectX::XMVector3TransformNormal( { 0.0f, 0.0f, 1.0f }, rot );

						Vec3 frustum_corners[8];
						Vec3 frustum_centre = { 0.0f, 0.0f, 0.0f };
						camera.CalculateFrustumCorners( frustum_corners );
						for( int i = 0; i < 8; i++ )
						{
							frustum_centre += frustum_corners[i];
						}
						frustum_centre /= 8.0f;

						DirectX::XMMATRIX light_view = DirectX::XMMatrixLookToLH( frustum_centre, to, up );
						Vec3 maxs = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
						Vec3 mins = { FLT_MAX, FLT_MAX, FLT_MAX };
						for( int i = 0; i < 8; i++ )
						{
							DirectX::XMVECTOR corner = DirectX::XMVector3TransformCoord( frustum_corners[i], light_view );
							mins = DirectX::XMVectorMin( mins, corner );
							maxs = DirectX::XMVectorMax( maxs, corner );
						}

						Vec3 shadow_camera_pos = frustum_centre - to * maxs.z;

						m_matLightView = DirectX::XMMatrixLookAtLH( shadow_camera_pos, frustum_centre, up );
						m_matLightProj = DirectX::XMMatrixOrthographicOffCenterLH( mins.x, maxs.x, mins.y, maxs.y, maxs.z, mins.z );
						
						break;
					}
					case LightType::SPOT:
					{
						Camera spot_cam( t.GetPosition(),
							t.GetRotation(),
							Math::RadToDeg( light.GetSpotlightAngle() * 2 ),
							(float)SHADOW_MAP_WIDTH / SHADOW_MAP_HEIGHT,
							light.GetRange(),
							0.1f);
						m_matLightView = spot_cam.GetViewMatrix();
						m_matLightProj = spot_cam.GetProjectionMatrix();
						break;
					}
					}

					light.SetShadowIndex( shadow_map_counter++ );
					transforms.shadow[light.GetShadowIndex()] = m_matLightView * m_matLightProj;

					pContext->ClearDepthStencil( m_pShadowMaps.get(), CLEAR_FLAG_DEPTH, 0.0f, 0, light.GetShadowIndex() );
					pContext->SetDepthStencil( m_pShadowMaps.get(), light.GetShadowIndex() );

					Traverse();
				}
			}

			pContext->SetDepthComparisonFunc( original_comp_func );
			pContext->SetDepthStencil( original_depth );
			pContext->PopViewport();

			m_cbufShadowMatrices.Update( pContext, transforms );
			pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufShadowMatrices, PS_CBUF_SHADOWMATRICES );
			pContext->BindTexture( m_pShadowMaps.get(), PS_TEX_SHADOWMAPS );
		}
	private:
		virtual void Visit( const DirectX::XMMATRIX& transform, Entity e ) override
		{
			if( e.Has<AnimationComponent>() )
			{
				auto& anim = e.Get<AnimationComponent>();

				DirectX::XMMATRIX* cbuf = m_cbufBones.Lock( m_pContext )->bone_transforms;

				GetMatrixPalette( cbuf, anim.bones, anim.current_pose );

				m_cbufBones.Unlock( m_pContext );

				m_pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufBones, VS_CBUF_BONES );
			}
			if( e.Has<ModelComponent>() )
			{
				auto& model = e.Get<ModelComponent>();

				if( model.HasRenderFlag( RenderFlags::NO_SHADOW ) )
				{
					return;
				}

				DirectX::XMMATRIX w = transform;

				auto& meshes = model.GetMeshes();
				for( auto& pMesh : meshes )
				{
					if( pMesh->HasRenderFlag( RenderFlags::NO_SHADOW ) )
					{
						continue;
					}

					if( pMesh->GetMaterial().IsTranslucent() )
					{
						continue;
					}

					auto pPipeline = ShaderManager::GetPipeline<ShadowPipeline>();
					pPipeline->Render( m_pContext, *pMesh, m_matLightView * m_matLightProj, w );
				}
			}
		}

		void GetMatrixPalette( DirectX::XMMATRIX* out, const std::vector<BoneData>& bones, SkeletonPose pose ) const
		{
			ASSERT( bones.size() <= MAX_BONES, "Too many bones!" );

			auto model_pose = SkeletonPose::ToModelSpace( std::move( pose ) );
			SkeletonPose::ToMatrixPalette( model_pose, bones, out );
		}
	private:
		IGPUContext* m_pContext;
		DirectX::XMMATRIX m_matLightView;
		DirectX::XMMATRIX m_matLightProj;
		struct CB_Bones
		{
			DirectX::XMMATRIX bone_transforms[MAX_BONES];
		};
		ConstantBuffer<CB_Bones> m_cbufBones;
		struct CB_ShadowMatrices
		{
			DirectX::XMMATRIX shadow[MAX_SHADOW_SOURCES];
		};
		ConstantBuffer<CB_ShadowMatrices> m_cbufShadowMatrices;
		std::unique_ptr<IDepthStencil> m_pShadowMaps;
	};
}