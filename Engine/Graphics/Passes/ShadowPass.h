#pragma once

#include "SceneRenderPass.h"
#include "ShadowPipeline.h"
#include "ShaderManager.h"

namespace Bat
{
	class ShadowPass : public BaseRenderPass
	{
	public:
		static constexpr int SHADOW_MAP_SIZE = 2048;
		static constexpr int NUM_CASCADES = 3;

		ShadowPass()
		{
			m_pShadowMaps = std::unique_ptr<IDepthStencil>( gpu->CreateDepthStencil( SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, TEX_FORMAT_R24G8_TYPELESS, MAX_SHADOW_SOURCES ) );
		}

		virtual void Execute( IGPUContext* pContext, Camera& camera, SceneNode& scene, RenderData& data ) override
		{
			m_pContext = pContext;

			ComparisonFunc original_comp_func = pContext->GetDepthComparisonFunc();

			pContext->SetRenderTarget( nullptr );
			pContext->SetDepthEnabled( true );
			pContext->SetDepthWriteEnabled( true );
			pContext->SetDepthComparisonFunc( ComparisonFunc::GREATER );
			pContext->SetDepthClipEnabled( false );
			pContext->SetCullMode( CullMode::NONE );
			pContext->SetBlendingEnabled( false );
			pContext->UnbindTextureSlot( PS_TEX_SHADOWMAPS );

			Viewport vp;
			vp.width = SHADOW_MAP_SIZE;
			vp.height = SHADOW_MAP_SIZE;
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
						camera.CalculateFrustumCorners( frustum_corners );

						light.SetShadowIndex( shadow_map_counter );

						float splits[NUM_CASCADES + 1] = {
							0.0f,
							0.01f,
							0.1f,
							1.0f
						};

						for( int cascade = 0; cascade < NUM_CASCADES; cascade++ )
						{
							Vec3 cascade_centre = { 0.0f, 0.0f, 0.0f };
							Vec3 cascade_corners[8];
							for( int i = 0; i < 8; i++ )
							{
								// First 4 corners are near plane, next 4 are the adjacent far plane corners
								// Lerp between the 2 corners to generate a split

								Vec3 near_corner = frustum_corners[i % 4];
								Vec3 far_corner = frustum_corners[(i % 4) + 4];

								cascade_corners[i] = DirectX::XMVectorLerp( near_corner, far_corner, splits[cascade + i/4] );
								cascade_centre += cascade_corners[i];
							}
							cascade_centre /= 8.0f;

							float radius = 0.0f;
							for( int i = 0; i < 8; i++ )
							{
								float dist = ( cascade_corners[i] - cascade_centre ).Length();
								radius = std::max( dist, radius );
							}

							Vec3 maxs = { radius, radius, radius };
							Vec3 mins = -maxs;

							// Snap to texel grid
							Vec3 extents = maxs - mins;
							Vec3 texel_size = extents / SHADOW_MAP_SIZE;
							mins = DirectX::XMVectorFloor( mins / texel_size ) * texel_size;

							DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH( cascade_centre, to, up );
							DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH( mins.x, maxs.x, mins.y, maxs.y, maxs.z, mins.z );
							m_matLightViewProj = view * proj;
							m_LightFrustum = Frustum( m_matLightViewProj );
							transforms.shadow[shadow_map_counter++] = m_matLightViewProj;

							pContext->ClearDepthStencil( m_pShadowMaps.get(), CLEAR_FLAG_DEPTH, 0.0f, 0, light.GetShadowIndex() + cascade );
							pContext->SetDepthStencil( m_pShadowMaps.get(), light.GetShadowIndex() + cascade );

							Traverse();
						}
						
						break;
					}
					case LightType::SPOT:
					{
						Camera spot_cam( t.GetPosition(),
							t.GetRotation(),
							Math::RadToDeg( light.GetSpotlightAngle() * 2 ),
							1.0f,
							light.GetRange(),
							0.1f);

						DirectX::XMMATRIX view = spot_cam.GetViewMatrix();
						DirectX::XMMATRIX proj = spot_cam.GetProjectionMatrix();
						m_matLightViewProj = view * proj;
						m_LightFrustum = Frustum( m_matLightViewProj );

						light.SetShadowIndex( shadow_map_counter++ );
						transforms.shadow[light.GetShadowIndex()] = m_matLightViewProj;

						pContext->ClearDepthStencil( m_pShadowMaps.get(), CLEAR_FLAG_DEPTH, 0.0f, 0, light.GetShadowIndex() );
						pContext->SetDepthStencil( m_pShadowMaps.get(), light.GetShadowIndex() );

						Traverse();

						break;
					}
					}
				}
			}

			pContext->SetCullMode( CullMode::BACK );
			pContext->SetDepthClipEnabled( true );
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
					pPipeline->Render( m_pContext, *pMesh, m_matLightViewProj, w );
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
		DirectX::XMMATRIX m_matLightViewProj;
		Frustum m_LightFrustum;
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