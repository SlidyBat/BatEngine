#include "PCH.h"
#include "GraphicsConvert.h"

#include "ResourceManager.h"
#include "ShaderManager.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

namespace Bat
{
	static void BindUnitCube( IGPUContext* pContext )
	{
		static VertexBuffer<Vec3> positions( {
			// Front Face
			Vec3( -1.0f, -1.0f, -1.0f ),
			Vec3( -1.0f,  1.0f, -1.0f ),
			Vec3(  1.0f,  1.0f, -1.0f ),
			Vec3(  1.0f, -1.0f, -1.0f ),

			// Back Face            
			Vec3( -1.0f, -1.0f, 1.0f ),
			Vec3(  1.0f, -1.0f, 1.0f ),
			Vec3(  1.0f,  1.0f, 1.0f ),
			Vec3( -1.0f,  1.0f, 1.0f ),

			// Top Face                
			Vec3( -1.0f, 1.0f, -1.0f ),
			Vec3( -1.0f, 1.0f,  1.0f ),
			Vec3(  1.0f, 1.0f,  1.0f ),
			Vec3(  1.0f, 1.0f, -1.0f ),

			// Bottom Face            
			Vec3( -1.0f, -1.0f, -1.0f ),
			Vec3(  1.0f, -1.0f, -1.0f ),
			Vec3(  1.0f, -1.0f,  1.0f ),
			Vec3( -1.0f, -1.0f,  1.0f ),

			// Left Face            
			Vec3( -1.0f, -1.0f,  1.0f ),
			Vec3( -1.0f,  1.0f,  1.0f ),
			Vec3( -1.0f,  1.0f, -1.0f ),
			Vec3( -1.0f, -1.0f, -1.0f ),

			// Right Face            
			Vec3( 1.0f, -1.0f, -1.0f ),
			Vec3( 1.0f,  1.0f, -1.0f ),
			Vec3( 1.0f,  1.0f,  1.0f ),
			Vec3( 1.0f, -1.0f,  1.0f )
		} );

		static IndexBuffer indices( {
			// Front Face
			2,  1,  0,
			3,  2,  0,

			// Back Face
			6,  5,  4,
			7,  6,  4,

			// Top Face
			10,  9, 8,
			11, 10, 8,

			// Bottom Face
			14, 13, 12,
			15, 14, 12,

			// Left Face
			18, 17, 16,
			19, 18, 16,

			// Right Face
			22, 21, 20,
			23, 22, 20 
		} );

		pContext->SetVertexBuffer( positions );
		pContext->SetIndexBuffer( indices );
	}

	static Vec3 cube_directions[] =
	{
	   {  1.0f,  0.0f,  0.0f },
	   { -1.0f,  0.0f,  0.0f },
	   {  0.0f,  1.0f,  0.0f },
	   {  0.0f, -1.0f,  0.0f },
	   {  0.0f,  0.0f,  1.0f },
	   {  0.0f,  0.0f, -1.0f }
	};
	static Vec3 up_directions[] =
	{
	   { 0.0f, 1.0f,  0.0f },
	   { 0.0f, 1.0f,  0.0f },
	   { 0.0f, 0.0f, -1.0f },
	   { 0.0f, 0.0f,  1.0f },
	   { 0.0f, 1.0f,  0.0f },
	   { 0.0f, 1.0f,  0.0f }
	};


	struct CB_EquirectangularToCubemapMatrix
	{
		DirectX::XMMATRIX viewproj;
	};

	ITexture* GraphicsConvert::EquirectangularToCubemap( IGPUContext* pContext, ITexture* equirect, size_t width, size_t height )
	{
		static ConstantBuffer<CB_EquirectangularToCubemapMatrix> transforms;
		CB_EquirectangularToCubemapMatrix transforms_data;

		auto cubemap = std::unique_ptr<IRenderTarget>( gpu->CreateCubemapRenderTarget( width, height, equirect->GetFormat(), 1 ) );

		IDepthStencil* pOldDS = pContext->GetDepthStencil();
		bool old_depth_enabled = pContext->IsDepthEnabled();
		IRenderTarget* pOldRT = pContext->GetRenderTarget();
		pContext->SetDepthEnabled( false );
		pContext->SetDepthStencil( nullptr );

		pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );

		Viewport vp;
		vp.width = (float)width;
		vp.height = (float)height;
		vp.min_depth = 0.0f;
		vp.max_depth = 1.0f;
		vp.top_left = { 0, 0 };
		pContext->PushViewport( vp );

		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/SkyboxVS.hlsl" );
		pContext->SetVertexShader( pVertexShader );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/EquirectangularToCubemapPS.hlsl" );
		pContext->SetPixelShader( pPixelShader );

		pContext->BindTexture( equirect, PS_TEX_SLOT_0 );
		BindUnitCube( pContext );

		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH( Math::DegToRad( 90.0f ), 1.0f, 0.1f, 10.0f );

		for( size_t i = 0; i < 6; i++ )
		{
			DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH( DirectX::XMVectorZero(), cube_directions[i], up_directions[i] );
			transforms_data.viewproj = view * proj;
			transforms.Update( pContext, transforms_data );
			pContext->SetConstantBuffer( ShaderType::VERTEX, transforms, VS_CBUF_TRANSFORMS );

			pContext->SetRenderTarget( cubemap.get(), i );
			pContext->DrawIndexed( 36 );
		}

		pContext->PopViewport();

		pContext->SetRenderTarget( pOldRT );
		pContext->SetDepthStencil( pOldDS );
		pContext->SetDepthEnabled( old_depth_enabled );

		return cubemap->AsTexture();
	}

	ITexture* GraphicsConvert::MakeIrradianceMap( IGPUContext* pContext, ITexture* envmap, size_t width, size_t height )
	{
		static ConstantBuffer<CB_EquirectangularToCubemapMatrix> transforms;
		CB_EquirectangularToCubemapMatrix transforms_data;

		auto cubemap = std::unique_ptr<IRenderTarget>( gpu->CreateCubemapRenderTarget( width, height, envmap->GetFormat(), 1 ) );

		IDepthStencil* pOldDS = pContext->GetDepthStencil();
		bool old_depth_enabled = pContext->IsDepthEnabled();
		IRenderTarget* pOldRT = pContext->GetRenderTarget();
		pContext->SetDepthEnabled( false );
		pContext->SetDepthStencil( nullptr );

		pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );

		Viewport vp;
		vp.width = (float)width;
		vp.height = (float)height;
		vp.min_depth = 0.0f;
		vp.max_depth = 1.0f;
		vp.top_left = { 0, 0 };
		pContext->PushViewport( vp );

		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/SkyboxVS.hlsl" );
		pContext->SetVertexShader( pVertexShader );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/IrradianceConvolvePS.hlsl" );
		pContext->SetPixelShader( pPixelShader );

		pContext->BindTexture( envmap, PS_TEX_SLOT_0 );
		BindUnitCube( pContext );

		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH( Math::DegToRad( 90.0f ), 1.0f, 0.1f, 10.0f );

		for( size_t i = 0; i < 6; i++ )
		{
			DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH( DirectX::XMVectorZero(), cube_directions[i], up_directions[i] );
			transforms_data.viewproj = view * proj;
			transforms.Update( pContext, transforms_data );
			pContext->SetConstantBuffer( ShaderType::VERTEX, transforms, VS_CBUF_TRANSFORMS );

			pContext->SetRenderTarget( cubemap.get(), i );
			pContext->DrawIndexed( 36 );
		}

		pContext->PopViewport();

		pContext->SetRenderTarget( pOldRT );
		pContext->SetDepthStencil( pOldDS );
		pContext->SetDepthEnabled( old_depth_enabled );

		return cubemap->AsTexture();
	}
}
