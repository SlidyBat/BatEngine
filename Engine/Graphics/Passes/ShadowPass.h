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

			pContext->SetRenderTarget( nullptr );
			pContext->SetDepthEnabled( true );
			pContext->SetDepthWriteEnabled( true );
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
			size_t shadow_map_counter = 0;

			for( Entity e : world )
			{
				if( e.Has<LightComponent>() )
				{
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

					if( light.GetType() == LightType::SPOT )
					{
						auto& t = e.Get<TransformComponent>();
						m_matLightView = DirectX::XMMatrixLookToLH( t.GetPosition(), light.GetDirection(), { 0.0f, 1.0f, 0.0f } );
					}
					else
					{
						ASSERT( false, "What?" );
						m_matLightView = DirectX::XMMatrixLookAtLH( { 0.0f, 0.0f, 0.0f }, light.GetDirection(), { 0.0f, 1.0f, 0.0f } );
					}
					
					Vec3 frustum_corners[8];
					camera.CalculateFrustumCorners( frustum_corners );
					DirectX::XMMATRIX inv_view = DirectX::XMMatrixInverse( nullptr, camera.GetViewMatrix() );
					for( int i = 0; i < 8; i++ )
					{
						frustum_corners[i] = DirectX::XMVector3Transform( frustum_corners[i], inv_view ); // To world space
						frustum_corners[i] = DirectX::XMVector3Transform( frustum_corners[i], m_matLightView ); // To light space
					}
					AABB bounds = AABB( frustum_corners, 8 );
					//m_matLightProj = DirectX::XMMatrixOrthographicOffCenterLH( bounds.mins.x, bounds.maxs.x, bounds.mins.y, bounds.maxs.y, bounds.mins.z, bounds.maxs.z );
					m_matLightProj = DirectX::XMMatrixPerspectiveFovLH( light.GetSpotlightAngle() * 2, 1.0f, 0.1f, light.GetRange() );

					light.SetShadowIndex( shadow_map_counter++ );

					pContext->ClearDepthStencil( m_pShadowMaps.get(), CLEAR_FLAG_DEPTH, 1.0f, 0, light.GetShadowIndex() );
					pContext->SetDepthStencil( m_pShadowMaps.get(), light.GetShadowIndex() );

					Traverse();
				}
			}

			pContext->SetDepthStencil( original_depth );
			pContext->PopViewport();

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
		std::unique_ptr<IDepthStencil> m_pShadowMaps;
	};
}