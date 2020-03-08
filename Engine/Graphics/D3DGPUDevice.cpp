#include "PCH.h"
#include "D3DGPUDevice.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>

#ifdef _DEBUG
#include <initguid.h>
#include <d3d11sdklayers.h>

#ifdef GetMessage
#undef GetMessage
#endif
#include <dxgidebug.h>
#pragma comment( lib, "dxguid.lib" )
#endif

#include "IGPUDevice.h"
#include "COMException.h"
#include "Common.h"
#include "Window.h"
#include "FileWatchdog.h"
#include "MemoryStream.h"
#include <wrl.h>
#include <DirectXTex/DirectXTex.h>

#include "Hash.h"

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

MAKE_HASHABLE( D3D11_DEPTH_STENCILOP_DESC, t,
	t.StencilFailOp,
	t.StencilDepthFailOp,
	t.StencilPassOp,
	t.StencilFunc );

inline bool operator==( const D3D11_DEPTH_STENCILOP_DESC& a, const D3D11_DEPTH_STENCILOP_DESC& b )
{
	return a.StencilFailOp == b.StencilFailOp &&
		a.StencilDepthFailOp == b.StencilDepthFailOp &&
		a.StencilPassOp == b.StencilPassOp &&
		a.StencilFunc == b.StencilFunc;
}

MAKE_HASHABLE( D3D11_DEPTH_STENCIL_DESC, t,
	t.DepthEnable,
	t.DepthWriteMask,
	t.DepthFunc,
	t.StencilEnable,
	t.StencilReadMask,
	t.StencilWriteMask,
	t.FrontFace,
	t.BackFace );

inline bool operator==( const D3D11_DEPTH_STENCIL_DESC& a, const D3D11_DEPTH_STENCIL_DESC& b )
{
	return a.DepthEnable == b.DepthEnable &&
		a.DepthWriteMask == b.DepthWriteMask &&
		a.DepthFunc == b.DepthFunc &&
		a.StencilEnable == b.StencilEnable &&
		a.StencilReadMask == b.StencilReadMask &&
		a.StencilWriteMask == b.StencilWriteMask &&
		a.FrontFace == b.FrontFace &&
		a.BackFace == b.BackFace;
}

MAKE_HASHABLE( D3D11_RASTERIZER_DESC, t,
	t.FillMode,
	t.CullMode,
	t.FrontCounterClockwise,
	t.DepthBias,
	t.DepthBiasClamp,
	t.SlopeScaledDepthBias,
	t.DepthClipEnable,
	t.ScissorEnable,
	t.MultisampleEnable,
	t.AntialiasedLineEnable );

inline bool operator==( const D3D11_RASTERIZER_DESC& a, const D3D11_RASTERIZER_DESC& b )
{
	return a.FillMode == b.FillMode &&
		a.CullMode == b.CullMode &&
		a.FrontCounterClockwise == b.FrontCounterClockwise &&
		a.DepthBias == b.DepthBias &&
		a.DepthBiasClamp == b.DepthBiasClamp &&
		a.SlopeScaledDepthBias == b.SlopeScaledDepthBias &&
		a.DepthClipEnable == b.DepthClipEnable &&
		a.ScissorEnable == b.ScissorEnable &&
		a.MultisampleEnable == b.MultisampleEnable &&
		a.AntialiasedLineEnable == b.AntialiasedLineEnable;
}

#ifdef _DEBUG
#define DXGI_CALL( dev, fn ) do { \
		auto msg = dev->FlushMessages(); \
		fn; \
		msg = dev->FlushMessages(); \
		if( !msg.empty() ) \
		{ \
			ASSERT( false, msg ); \
		} \
	} while( false )
#else
#define DXGI_CALL( dev, fn ) fn
#endif

// Only use within GPUContext class. Does all the error checking in debug builds.
#define DXGI_CONTEXT_CALL( fn ) DXGI_CALL( m_pDevice, fn )
// Only use within GPUDevice class. Does all the error checking in debug builds.
#define DXGI_DEVICE_CALL( fn ) DXGI_CALL( this, fn )

namespace Bat
{
	static const char* FeatureLevel2String( D3D_FEATURE_LEVEL );

	class AttributeInfo
	{
	public:
		static VertexAttribute SemanticToAttribute( const std::string& semantic )
		{
			static std::unordered_map<std::string, VertexAttribute> s_mapConvert;
			static bool initialized = false;
			if( !initialized )
			{
				initialized = true;
				s_mapConvert["SV_VertexID"] = VertexAttribute::VertexId;
				s_mapConvert["POSITION"] = VertexAttribute::Position;
				s_mapConvert["COLOR"] = VertexAttribute::Colour;
				s_mapConvert["NORMAL"] = VertexAttribute::Normal;
				s_mapConvert["TEXCOORD"] = VertexAttribute::UV;
				s_mapConvert["TANGENT"] = VertexAttribute::Tangent;
				s_mapConvert["BITANGENT"] = VertexAttribute::Bitangent;
				s_mapConvert["BONEID"] = VertexAttribute::BoneId;
				s_mapConvert["BONEWEIGHT"] = VertexAttribute::BoneWeight;
				s_mapConvert["INSTANCE_DATA"] = VertexAttribute::InstanceData;
			}

			auto it = s_mapConvert.find( semantic );
			if( it == s_mapConvert.end() )
			{
				return VertexAttribute::Invalid;
			}

			return it->second;
		}

		static std::string AttributeToSemantic( const VertexAttribute attribute )
		{
			static std::unordered_map<VertexAttribute, std::string> s_mapConvert;
			static bool initialized = false;
			if( !initialized )
			{
				initialized = true;
				s_mapConvert[VertexAttribute::VertexId] = "SV_VertexID";
				s_mapConvert[VertexAttribute::Position] = "POSITION";
				s_mapConvert[VertexAttribute::Colour] = "COLOR";
				s_mapConvert[VertexAttribute::Normal] = "NORMAL";
				s_mapConvert[VertexAttribute::UV] = "TEXCOORD";
				s_mapConvert[VertexAttribute::Tangent] = "TANGENT";
				s_mapConvert[VertexAttribute::Bitangent] = "BITANGENT";
				s_mapConvert[VertexAttribute::BoneId] = "BONEID";
				s_mapConvert[VertexAttribute::BoneWeight] = "BONEWEIGHT";
				s_mapConvert[VertexAttribute::InstanceData] = "INSTANCE_DATA";
			}

			auto it = s_mapConvert.find( attribute );
			if( it == s_mapConvert.end() )
			{
				return "";
			}

			return it->second;
		}
	};

	class D3DRenderTarget : public IRenderTarget
	{
	public:
		D3DRenderTarget() = default;
		D3DRenderTarget( ID3D11Device* pDevice,
			size_t width,
			size_t height,
			TexFormat format,
			MsaaQuality ms_quality,
			size_t ms_samples,
			TexFlags flags );

		static D3DRenderTarget* Cubemap( ID3D11Device* pDevice,
			size_t width,
			size_t height,
			TexFormat format,
			size_t mip_levels,
			TexFlags flags );

		virtual size_t GetWidth() const override { return m_iWidth; }
		virtual size_t GetHeight() const override { return m_iHeight; }
		virtual TexFormat GetFormat() const override { return m_Format; }
		virtual ITexture* AsTexture() const override;

		ID3D11RenderTargetView*   GetRenderTargetView( size_t array_index, size_t mip_index ) const { return m_pRenderTargetViews[array_index * m_iMipLevels + mip_index].Get(); }
		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView.Get(); }
		ID3D11Texture2D*          GetResource() const { return m_pRenderTargetTexture.Get(); }
		void Reset( ID3D11RenderTargetView* pRenderTargetView, size_t width, size_t height )
		{
			m_pRenderTargetViews.resize( 1 );
			m_pRenderTargetViews[0] = pRenderTargetView;

			m_iWidth = width;
			m_iHeight = height;
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pRenderTargetTexture = nullptr;
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_pRenderTargetViews;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView = nullptr;

		TexFormat m_Format = TEX_FORMAT_UNKNOWN;
		size_t m_iWidth = 0;
		size_t m_iHeight = 0;
		size_t m_iMipLevels = 1;
		D3D_SRV_DIMENSION m_Dimension;
	};

	class D3DPixelShader : public IPixelShader
	{
	public:
		D3DPixelShader( ID3D11Device* pDevice, const std::string& filename, const std::vector<ShaderMacro>& macros );
		~D3DPixelShader();

		virtual std::string GetName() const override { return m_szName; }
		ID3D11PixelShader* GetShader( ID3D11Device* pDevice );
	private:
		void LoadFromFile( ID3D11Device* pDevice, const std::string& filename, bool crash_on_error );
		void OnFileChanged( const std::string& filename );
		bool IsDirty() const { return m_bDirty; }
		void SetDirty( bool dirty ) { m_bDirty = dirty; }
	private:
		std::string m_szName;
		std::vector<ShaderMacro> m_Macros;
		std::atomic_bool m_bDirty = true;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pShader;

#ifdef _DEBUG
		FileWatchHandle_t m_hFileWatch;
#endif
	};

	class D3DVertexShader : public IVertexShader
	{
	public:
		D3DVertexShader( ID3D11Device* pDevice, const std::string& filename, const std::vector<ShaderMacro>& macros );
		~D3DVertexShader();

		virtual std::string GetName() const override { return m_szName; }
		virtual int GetVertexAttributeCount( VertexAttribute attribute ) const { return m_iAttributeCount[(int)attribute]; }
		virtual int GetVertexAttributeSlot( VertexAttribute attribute, int index ) const override { return m_iAttributeSlot[(int)attribute][index]; }
		ID3D11VertexShader* GetShader( ID3D11Device* pDevice );
		ID3D11InputLayout* GetLayout() { return m_pInputLayout.Get(); }
		const ID3D11InputLayout* GetLayout() const { return m_pInputLayout.Get(); }
	private:
		void CreateInputLayoutDescFromVertexShaderSignature( ID3D11Device* pDevice, const void* pCodeBytes, const size_t size );
		void LoadFromFile( ID3D11Device* pDevice, const std::string& filename, bool crash_on_error );
		void OnFileChanged( const std::string& filename );
		bool IsDirty() const { return m_bDirty; }
		void SetDirty( bool dirty ) { m_bDirty = dirty; }
	private:
		static constexpr int MAX_ATTRIBUTE_COUNT = 16;

		std::string m_szName;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
		int m_iAttributeCount[(int)VertexAttribute::TotalAttributes];
		int m_iAttributeSlot[(int)VertexAttribute::TotalAttributes][MAX_ATTRIBUTE_COUNT];
		std::vector<ShaderMacro> m_Macros;
		std::atomic_bool m_bDirty = true;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pShader;

#ifdef _DEBUG
		FileWatchHandle_t m_hFileWatch;
#endif
	};

	class D3DGPUContext : public IGPUContext
	{
		friend class D3DGPUDevice;
	public:
		D3DGPUContext( D3DGPUDevice* pDevice );

		// Called once device context is initialized by device
		void Init();

		virtual IGPUDevice* GetDevice() override;
		virtual const IGPUDevice* GetDevice() const override;

		virtual void SetPrimitiveTopology( PrimitiveTopology topology ) override;
		virtual PrimitiveTopology GetPrimitiveTopology() const override;

		virtual size_t GetViewportCount() const override;
		virtual const Viewport& GetViewport(size_t slot = 0) const override;
		// Sets current viewport
		virtual void SetViewport( const Viewport& viewport ) override;
		virtual void SetViewports( const std::vector<Viewport>& viewport ) override;
		// Pushes a new viewport on to the stack
		virtual void PushViewport( const Viewport& viewport ) override;
		virtual void PushViewports( const std::vector<Viewport>& viewport ) override;
		// Pops top viewport from stack.
		virtual void PopViewport() override;

		virtual void SetDepthStencil( IDepthStencil* pDepthStencil, size_t index = 0 ) override;
		virtual IDepthStencil* GetDepthStencil() const override;
		virtual bool IsDepthEnabled() const override;
		virtual void SetDepthEnabled( bool enabled ) override;
		virtual bool IsDepthWriteEnabled() const override;
		virtual void SetDepthWriteEnabled( bool enabled ) override;
		virtual ComparisonFunc GetDepthComparisonFunc() const;
		virtual void SetDepthComparisonFunc( ComparisonFunc func );
		virtual bool IsStencilEnabled() const;
		virtual void SetStencilEnabled( bool enabled );
		virtual uint8_t GetStencilReadMask() const;
		virtual void SetStencilReadMask( uint8_t mask );
		virtual uint8_t GetStencilWriteMask() const;
		virtual void SetStencilWriteMask( uint8_t mask );
		virtual StencilOpDesc GetStencilFrontFaceOp() const;
		virtual void SetStencilFrontFaceOp( const StencilOpDesc& desc );
		virtual StencilOpDesc GetStencilBackFaceOp() const;
		virtual void SetStencilBackFaceOp( const StencilOpDesc& desc );

		virtual CullMode GetCullMode() const override;
		virtual void SetCullMode( CullMode mode ) override;
		virtual bool GetDepthClipEnabled() const override;
		virtual void SetDepthClipEnabled( bool enabled ) override;

		virtual bool IsBlendingEnabled() const override;
		virtual void SetBlendingEnabled( bool enabled ) override;

		virtual size_t GetRenderTargetCount() const override;
		// Get's currently bound render target, or nullptr if no render target is bound
		virtual IRenderTarget* GetRenderTarget( size_t slot = 0 ) const override;
		// Sets current render target. Pass nullptr to bind backbuffer
		virtual void SetRenderTarget( IRenderTarget* pRT, size_t array_index = 0, size_t mip_index = 0 ) override;
		virtual void SetRenderTargets( const std::vector<IRenderTarget*>& pRTs ) override;
		virtual void PushRenderTarget() override;
		virtual void PushRenderTarget( IRenderTarget* pRT ) override;
		virtual void PushRenderTargets( const std::vector<IRenderTarget*>& pRTs ) override;
		virtual void PopRenderTarget() override;
		virtual void UnbindRenderTargets() override;
		virtual void ClearRenderTargetStack() override;

		virtual void SetRenderTargetAndViewport( IRenderTarget* pRT ) override;
		virtual void PushRenderTargetAndViewport( IRenderTarget* pRT ) override;
		virtual void PopRenderTargetAndViewport() override;

		virtual void ClearRenderTarget( IRenderTarget* pRT, float r, float g, float b, float a, size_t array_index = 0, size_t mip_index = 0 ) override;
		virtual void ClearDepthStencil( IDepthStencil* pDepthStencil, int clearflag, float depth, uint8_t stencil, size_t index = 0 ) override;
		virtual void Resolve( IRenderTarget* pDst, IRenderTarget* pSrc ) override;

		virtual void UpdateTexturePixels( ITexture* pTexture, const void* pPixels, size_t pitch ) override;
		virtual void BindTexture( ITexture* pTexture, size_t slot ) override;
		virtual void BindTexture( IRenderTarget* pRT, size_t slot ) override;
		virtual void BindTexture( IDepthStencil* pDepthStencil, size_t slot ) override;
		virtual void UnbindTextureSlot( size_t slot ) override;

		virtual void UpdateBuffer( IVertexBuffer* pBuffer, const void* pData, size_t count = 0, size_t offset = 0 ) override;
		virtual void* Lock( IVertexBuffer* pBuffer ) override;
		virtual void Unlock( IVertexBuffer* pBuffer ) override;
		virtual void UpdateBuffer( IIndexBuffer* pBuffer, const void* pData, size_t count = 0, size_t offset = 0 ) override;
		virtual void* Lock( IIndexBuffer* pBuffer ) override;
		virtual void Unlock( IIndexBuffer* pBuffer ) override;
		virtual void UpdateBuffer( IConstantBuffer* pBuffer, const void* pData, size_t count = 0, size_t offset = 0 ) override;
		virtual void* Lock( IConstantBuffer* pBuffer ) override;
		virtual void Unlock( IConstantBuffer* pBuffer ) override;

		virtual IPixelShader* GetPixelShader() const override;
		virtual void SetPixelShader( IPixelShader* pShader ) override;
		virtual IVertexShader* GetVertexShader() const override;
		virtual void SetVertexShader( IVertexShader* pShader ) override;

		virtual void SetVertexBuffer( IVertexBuffer* pBuffer, size_t slot = 0 ) override;
		virtual void SetIndexBuffer( IIndexBuffer* pBuffer ) override;
		virtual void SetConstantBuffer( ShaderType shader, IConstantBuffer* pBuffer, size_t slot ) override;

		virtual void SetSampler( ShaderType shader, ISampler* pSampler, size_t slot ) override;

		virtual void Draw( size_t vertex_count ) override;
		virtual void DrawInstanced( size_t vertex_count, size_t instance_count ) override;
		virtual void DrawIndexed( size_t index_count ) override;
		virtual void DrawInstancedIndexed( size_t index_count, size_t instance_count ) override;

		virtual void BeginEvent( const std::string& name ) override;
		virtual void EndEvent() override;

		virtual void* GetImpl() override { return m_pDeviceContext.Get(); }
	private:
		void BindViewports();
		void BindRenderTarget();
		void BindDepthStencilState();
		void BindRasterizerState();
	private:
		D3DGPUDevice* m_pDevice = nullptr;

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext;
		Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_pAnnotation;
		std::vector<std::vector<IRenderTarget*>> m_RenderTargetStack;
		size_t m_iRenderTargetArrayIndex = 0;
		size_t m_iRenderTargetMipIndex = 0;
		std::vector<std::vector<Viewport>> m_ViewportStack;
		class D3DDepthStencil* m_pDepthStencil = nullptr;
		size_t m_iDepthStencilIndex = 0;
		D3DPixelShader* m_pPixelShader = nullptr;
		D3DVertexShader* m_pVertexShader = nullptr;

		D3D11_DEPTH_STENCIL_DESC m_CurrentDepthStencilState = {};
		D3D11_RASTERIZER_DESC m_CurrentRasterizerState = {};
		bool m_bBlendingEnabled = false;
	};

	class D3DGPUDevice : public IGPUDevice
	{
	public:
		D3DGPUDevice( Window& wnd, bool vsyncEnabled, float screendepth, float screennear );

		virtual const DeviceInfo& GetDeviceInfo() const override;

		virtual IPixelShader*    CreatePixelShader( const std::string& filename, const std::vector<ShaderMacro>& macros = {} ) override;
		virtual IVertexShader*   CreateVertexShader( const std::string& filename, const std::vector<ShaderMacro>& macros = {} ) override;

		virtual IVertexBuffer*   CreateVertexBuffer( const void* pData, size_t elem_size, size_t size ) override;
		virtual IIndexBuffer*    CreateIndexBuffer( const void* pData, size_t elem_size, size_t size ) override;
		virtual IConstantBuffer* CreateConstantBuffer( const void* pData, size_t size ) override;

		virtual ITexture* CreateTexture( const std::string& filename, TexFlags flags = TexFlags::NONE ) override;
		virtual ITexture* CreateTexture( const char* pData, size_t size, TexFlags flags = TexFlags::NONE ) override;
		virtual ITexture* CreateTexture( const void* pPixels,
			size_t pitch,
			size_t width,
			size_t height,
			TexFormat format,
			GPUResourceUsage usage = GPUResourceUsage::DEFAULT,
			TexFlags flags = TexFlags::NONE ) override;

		virtual IDepthStencil* CreateDepthStencil( size_t width,
			size_t height,
			TexFormat format,
			size_t array_size = 1,
			MsaaQuality ms_quality = MsaaQuality::NONE,
			size_t ms_samples = 1,
			TexFlags flags = TexFlags::NONE ) override;

		virtual IRenderTarget* CreateRenderTarget( size_t width,
			size_t height,
			TexFormat format,
			MsaaQuality ms_quality = MsaaQuality::NONE,
			size_t ms_samples = 1,
			TexFlags flags = TexFlags::NONE ) override;
		virtual IRenderTarget* CreateCubemapRenderTarget( size_t width,
			size_t height,
			TexFormat format,
			size_t mip_levels = 1,
			TexFlags flags = TexFlags::NONE ) override;
		virtual IRenderTarget* GetBackbuffer() override;
		virtual const IRenderTarget* GetBackbuffer() const override;

		virtual ISampler* CreateSampler( const SamplerDesc& sampler_desc ) override;

		virtual void SwapBuffers() override;

		virtual void ResizeBuffers( size_t width, size_t height ) override;

		virtual IGPUContext* GetContext() override { return &m_GPUContext; };

		virtual void* GetImpl() override { return m_pDevice.Get(); }

		ID3D11DepthStencilState* GetDepthStencilState( const D3D11_DEPTH_STENCIL_DESC& state );
		ID3D11DepthStencilState* AddDepthStencilState( const D3D11_DEPTH_STENCIL_DESC& state );
		ID3D11RasterizerState* GetRasterizerState( const D3D11_RASTERIZER_DESC& state );
		ID3D11RasterizerState* AddRasterizerState( const D3D11_RASTERIZER_DESC& state );
	public:
		ID3D11BlendState*        GetBlendEnabledState() { return m_pBlendEnabledState.Get(); }
		ID3D11BlendState*        GetBlendDisabledState() { return m_pBlendDisabledState.Get(); }

		std::string FlushMessages();
	private:
		bool       m_bVSyncEnabled;

		DeviceInfo m_DeviceInfo;

		Microsoft::WRL::ComPtr<IDXGISwapChain>           m_pSwapChain;

		Microsoft::WRL::ComPtr<ID3D11Device>             m_pDevice;

#ifdef _DEBUG
		Microsoft::WRL::ComPtr<IDXGIInfoQueue>           m_pInfoQueue;
#endif

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_pRenderTargetView;

		Microsoft::WRL::ComPtr<ID3D11BlendState>         m_pBlendEnabledState;
		Microsoft::WRL::ComPtr<ID3D11BlendState>         m_pBlendDisabledState;

		std::unordered_map<D3D11_DEPTH_STENCIL_DESC, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>>  m_pDepthStencilStates;
		std::unordered_map<D3D11_RASTERIZER_DESC, Microsoft::WRL::ComPtr<ID3D11RasterizerState>>  m_pRasterizerStates;

		D3DGPUContext m_GPUContext;
		D3DRenderTarget m_Backbuffer;
	};

	class D3DVertexBuffer : public IVertexBuffer
	{
	public:
		D3DVertexBuffer( ID3D11Device* pDevice, const void* pData, size_t elem_size, size_t size );

		virtual size_t GetVertexCount() const override { return m_iSize; }
		size_t GetElementSize() const override { return m_iElemSize; }

		ID3D11Buffer* GetBuffer() const { return m_pVertexBuffer.Get(); }
		void UpdateBuffer( ID3D11DeviceContext* pDeviceContext, const void* pData, size_t count = 0, size_t offset = 0 );
		void* Lock( ID3D11DeviceContext* pDeviceContext );
		void Unlock( ID3D11DeviceContext* pDeviceContext );
	private:
		size_t m_iElemSize = 0;
		size_t m_iSize = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer = nullptr;
	};

	class D3DIndexBuffer : public IIndexBuffer
	{
	public:
		D3DIndexBuffer( ID3D11Device* pDevice, const void* pData, size_t elem_size, size_t size );

		virtual size_t GetIndexCount() const override { return m_iSize; }
		virtual size_t GetElementSize() const override { return m_iElemSize; }

		ID3D11Buffer* GetBuffer() const { return m_pIndexBuffer.Get(); }
		void UpdateBuffer( ID3D11DeviceContext* pDeviceContext, const void* pData, size_t count = 0, size_t offset = 0 );
		void* Lock( ID3D11DeviceContext* pDeviceContext );
		void Unlock( ID3D11DeviceContext* pDeviceContext );
	private:
		size_t m_iElemSize = 0;
		size_t m_iSize = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer = nullptr;
	};

	class D3DConstantBuffer : public IConstantBuffer
	{
	public:
		D3DConstantBuffer( ID3D11Device* pDevice, const void* pData, size_t size );

		virtual size_t GetSize() const { return m_iSize; }

		ID3D11Buffer* GetBuffer() const { return m_pConstantBuffer.Get(); }
		void UpdateBuffer( ID3D11DeviceContext* pDeviceContext, const void* pData, size_t count = 0, size_t offset = 0 );
		void* Lock( ID3D11DeviceContext* pDeviceContext );
		void Unlock( ID3D11DeviceContext* pDeviceContext );
	private:
		size_t m_iSize = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBuffer = nullptr;
	};

	class D3DTexture : public ITexture
	{
	public:
		D3DTexture( ID3D11Resource* texture,
			ID3D11ShaderResourceView* texture_view,
			TexFormat format,
			size_t width,
			size_t height );
		D3DTexture( ID3D11Device* pDevice, const std::string& filename, TexFlags flags );
		D3DTexture( ID3D11Device* pDevice, const char* pData, size_t size, TexFlags flags );
		D3DTexture( ID3D11Device* pDevice, const DirectX::Image& image, const DirectX::TexMetadata& metadata, GPUResourceUsage usage, TexFlags flags );

		virtual size_t GetWidth() const override { return m_iWidth; }
		virtual size_t GetHeight() const override { return m_iHeight; }
		virtual TexFormat GetFormat() const override { return m_Format; }
		virtual bool IsTranslucent() const override { return m_bIsTranslucent; }
		virtual void* GetImpl() override { return m_pTextureView.Get(); }

		void UpdatePixels( ID3D11DeviceContext* pDeviceContext, const void* pPixels, size_t pitch );
		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pTextureView.Get(); }
	private:
		void InitializeTex( ID3D11Device* pDevice, const DirectX::Image& image, const DirectX::TexMetadata& metadata, GPUResourceUsage usage, TexFlags flags );
	private:
		Microsoft::WRL::ComPtr<ID3D11Resource>				m_pTexture = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pTextureView = nullptr;

		TexFormat m_Format;
		size_t m_iWidth = 0;
		size_t m_iHeight = 0;
		bool m_bIsTranslucent = false;
	};

	class D3DDepthStencil : public IDepthStencil
	{
	public:
		D3DDepthStencil( ID3D11Device* pDevice,
			size_t width,
			size_t height,
			TexFormat format,
			size_t array_size,
			MsaaQuality ms_quality,
			size_t ms_samples,
			TexFlags flags );

		virtual size_t GetWidth() const override { return m_iWidth; }
		virtual size_t GetHeight() const override { return m_iHeight; }
		virtual TexFormat GetFormat() const override { return m_Format; }
		virtual size_t GetArraySize() const override { return m_iArraySize; }

		ID3D11DepthStencilView* GetDepthStencilView( size_t index ) { return m_pDepthStencilView[index].Get(); }
		const ID3D11DepthStencilView* GetDepthStencilView( size_t index ) const { return m_pDepthStencilView[index].Get(); }
		ID3D11ShaderResourceView* GetShaderResourceView() { return m_pDepthShaderResourceView.Get(); }
		const ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pDepthShaderResourceView.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D>                     m_pDepthStencilBuffer;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_pDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>            m_pDepthShaderResourceView;

		TexFormat m_Format;
		size_t m_iWidth;
		size_t m_iHeight;
		size_t m_iArraySize;
	};

	class D3DSampler : public ISampler
	{
	public:
		D3DSampler( ID3D11Device* pDevice, const SamplerDesc& sampler_desc );

		ID3D11SamplerState* GetSamplerState() { return m_pSamplerState.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState = nullptr;
	};

	D3DGPUContext::D3DGPUContext( D3DGPUDevice* pDevice )
		:
		m_pDevice( pDevice )
	{}

	void D3DGPUContext::Init()
	{
		m_ViewportStack.emplace_back();

		m_CurrentDepthStencilState.DepthEnable = TRUE;
		m_CurrentDepthStencilState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
		m_CurrentDepthStencilState.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
		m_CurrentDepthStencilState.StencilEnable = FALSE;
		m_CurrentDepthStencilState.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		m_CurrentDepthStencilState.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		m_CurrentDepthStencilState.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		m_CurrentDepthStencilState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		m_CurrentDepthStencilState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		m_CurrentDepthStencilState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		m_CurrentDepthStencilState.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		m_CurrentDepthStencilState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		m_CurrentDepthStencilState.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		m_CurrentDepthStencilState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

		m_CurrentRasterizerState.AntialiasedLineEnable = false;
		m_CurrentRasterizerState.CullMode = D3D11_CULL_BACK;
		m_CurrentRasterizerState.DepthBias = 0;
		m_CurrentRasterizerState.DepthBiasClamp = 0.0f;
		m_CurrentRasterizerState.DepthClipEnable = true;
		m_CurrentRasterizerState.FillMode = D3D11_FILL_SOLID;
		m_CurrentRasterizerState.FrontCounterClockwise = false;
		m_CurrentRasterizerState.MultisampleEnable = true;
		m_CurrentRasterizerState.ScissorEnable = false;
		m_CurrentRasterizerState.SlopeScaledDepthBias = 0.0f;

		m_pDeviceContext.As( &m_pAnnotation );
	}

	IGPUDevice* D3DGPUContext::GetDevice()
	{
		return m_pDevice;
	}

	const IGPUDevice* D3DGPUContext::GetDevice() const
	{
		return m_pDevice;
	}

	void D3DGPUContext::SetPrimitiveTopology( PrimitiveTopology topology )
	{
		D3D11_PRIMITIVE_TOPOLOGY d3dtop = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		switch( topology )
		{
			case PrimitiveTopology::INVALID:
				d3dtop = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
				break;
			case PrimitiveTopology::POINTLIST:
				d3dtop = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
				break;
			case PrimitiveTopology::LINELIST:
				d3dtop = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
				break;
			case PrimitiveTopology::LINESTRIP:
				d3dtop = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
				break;
			case PrimitiveTopology::TRIANGLELIST:
				d3dtop = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				break;
			case PrimitiveTopology::TRIANGLESTRIP:
				d3dtop = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
				break;
			default:
				ASSERT( false, "Unknown primitive topology '%i'", (int)topology );
				break;
		}

		DXGI_CONTEXT_CALL( m_pDeviceContext->IASetPrimitiveTopology( d3dtop ) );
	}

	PrimitiveTopology D3DGPUContext::GetPrimitiveTopology() const
	{
		D3D11_PRIMITIVE_TOPOLOGY d3dtop;
		m_pDeviceContext->IAGetPrimitiveTopology( &d3dtop );
		switch( d3dtop )
		{
			case D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED:
				return PrimitiveTopology::INVALID;
			case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST:
				return PrimitiveTopology::POINTLIST;
			case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
				return PrimitiveTopology::LINELIST;
			case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
				return PrimitiveTopology::LINESTRIP;
			case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
				return PrimitiveTopology::TRIANGLELIST;
			case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
				return PrimitiveTopology::TRIANGLESTRIP;
			default:
				ASSERT( false, "Unhandled D3D primitive topology '%i'", (int)d3dtop );
				return PrimitiveTopology::INVALID;
		}
	}

	size_t D3DGPUContext::GetViewportCount() const
	{
		if (m_ViewportStack.empty())
		{
			return 0;
		}

		return m_ViewportStack.back().size();
	}

	const Viewport& D3DGPUContext::GetViewport(size_t slot) const
	{
		static Viewport null_vp;

		if( m_ViewportStack.empty() )
		{
			return null_vp;
		}

		return m_ViewportStack.back()[slot];
	}

	void D3DGPUContext::SetViewport( const Viewport& viewport )
	{
		m_ViewportStack.back() = { viewport };

		BindViewports();
	}

	void D3DGPUContext::SetViewports( const std::vector<Viewport>& viewports )
	{
		m_ViewportStack.back() = viewports;
		BindViewports();
	}

	void D3DGPUContext::PushViewport( const Viewport& viewport )
	{
		m_ViewportStack.push_back( { viewport } );
		BindViewports();
	}

	void D3DGPUContext::PushViewports( const std::vector<Viewport>& viewports )
	{
		m_ViewportStack.push_back( viewports );
		BindViewports();
	}

	void D3DGPUContext::PopViewport()
	{
		// Don't pop off null viewport
		if( m_ViewportStack.size() > 1 )
		{
			m_ViewportStack.pop_back();
		}
		BindViewports();
	}

	void D3DGPUContext::SetDepthStencil( IDepthStencil* pDepthStencil, size_t index )
	{
		m_pDepthStencil = static_cast<D3DDepthStencil*>( pDepthStencil );
		m_iDepthStencilIndex = index;
		BindRenderTarget();
	}

	IDepthStencil* D3DGPUContext::GetDepthStencil() const
	{
		return m_pDepthStencil;
	}

	bool D3DGPUContext::IsDepthEnabled() const
	{
		return m_CurrentDepthStencilState.DepthEnable;
	}

	void D3DGPUContext::SetDepthEnabled( bool enabled )
	{
		m_CurrentDepthStencilState.DepthEnable = enabled;
		BindDepthStencilState();
	}

	bool D3DGPUContext::IsDepthWriteEnabled() const
	{
		return m_CurrentDepthStencilState.DepthWriteMask != D3D11_DEPTH_WRITE_MASK_ZERO;
	}

	void D3DGPUContext::SetDepthWriteEnabled( bool enabled )
	{
		m_CurrentDepthStencilState.DepthWriteMask = enabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		BindDepthStencilState();
	}

	ComparisonFunc D3DGPUContext::GetDepthComparisonFunc() const
	{
		return (ComparisonFunc)m_CurrentDepthStencilState.DepthFunc;
	}

	void D3DGPUContext::SetDepthComparisonFunc( ComparisonFunc func )
	{
		m_CurrentDepthStencilState.DepthFunc = (D3D11_COMPARISON_FUNC)func;
		BindDepthStencilState();
	}

	bool D3DGPUContext::IsStencilEnabled() const
	{
		return m_CurrentDepthStencilState.StencilEnable;
	}

	void D3DGPUContext::SetStencilEnabled( bool enabled )
	{
		m_CurrentDepthStencilState.StencilEnable = enabled;
		BindDepthStencilState();
	}

	uint8_t D3DGPUContext::GetStencilReadMask() const
	{
		return (uint8_t)m_CurrentDepthStencilState.StencilReadMask;
	}

	void D3DGPUContext::SetStencilReadMask( uint8_t mask )
	{
		m_CurrentDepthStencilState.StencilReadMask = (UINT8)mask;
		BindDepthStencilState();
	}

	uint8_t D3DGPUContext::GetStencilWriteMask() const
	{
		return (uint8_t)m_CurrentDepthStencilState.StencilWriteMask;
	}

	void D3DGPUContext::SetStencilWriteMask( uint8_t mask )
	{
		m_CurrentDepthStencilState.StencilWriteMask = (UINT8)mask;
		BindDepthStencilState();
	}

	StencilOpDesc D3DGPUContext::GetStencilFrontFaceOp() const
	{
		StencilOpDesc desc;
		desc.fail_op = (StencilOp)m_CurrentDepthStencilState.FrontFace.StencilFailOp;
		desc.depth_fail_op = (StencilOp)m_CurrentDepthStencilState.FrontFace.StencilDepthFailOp;
		desc.pass_op = (StencilOp)m_CurrentDepthStencilState.FrontFace.StencilPassOp;
		desc.func = (ComparisonFunc)m_CurrentDepthStencilState.FrontFace.StencilFunc;

		return desc;
	}

	void D3DGPUContext::SetStencilFrontFaceOp( const StencilOpDesc& desc )
	{
		m_CurrentDepthStencilState.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)desc.fail_op;
		m_CurrentDepthStencilState.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)desc.depth_fail_op;
		m_CurrentDepthStencilState.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)desc.pass_op;
		m_CurrentDepthStencilState.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)desc.func;
		BindDepthStencilState();
	}

	StencilOpDesc D3DGPUContext::GetStencilBackFaceOp() const
	{
		StencilOpDesc desc;
		desc.fail_op = (StencilOp)m_CurrentDepthStencilState.BackFace.StencilFailOp;
		desc.depth_fail_op = (StencilOp)m_CurrentDepthStencilState.BackFace.StencilDepthFailOp;
		desc.pass_op = (StencilOp)m_CurrentDepthStencilState.BackFace.StencilPassOp;
		desc.func = (ComparisonFunc)m_CurrentDepthStencilState.BackFace.StencilFunc;

		return desc;
	}

	void D3DGPUContext::SetStencilBackFaceOp( const StencilOpDesc& desc )
	{
		m_CurrentDepthStencilState.BackFace.StencilFailOp = (D3D11_STENCIL_OP)desc.fail_op;
		m_CurrentDepthStencilState.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)desc.depth_fail_op;
		m_CurrentDepthStencilState.BackFace.StencilPassOp = (D3D11_STENCIL_OP)desc.pass_op;
		m_CurrentDepthStencilState.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)desc.func;
		BindDepthStencilState();
	}

	CullMode D3DGPUContext::GetCullMode() const
	{
		return (CullMode)m_CurrentRasterizerState.CullMode;
	}

	void D3DGPUContext::SetCullMode( CullMode mode )
	{
		m_CurrentRasterizerState.CullMode = (D3D11_CULL_MODE)mode;
		BindRasterizerState();
	}

	bool D3DGPUContext::GetDepthClipEnabled() const
	{
		return m_CurrentRasterizerState.DepthClipEnable;
	}

	void D3DGPUContext::SetDepthClipEnabled( bool enabled )
	{
		m_CurrentRasterizerState.DepthClipEnable = enabled;
		BindRasterizerState();
	}

	bool D3DGPUContext::IsBlendingEnabled() const
	{
		return m_bBlendingEnabled;
	}

	void D3DGPUContext::SetBlendingEnabled( bool enabled )
	{
		m_bBlendingEnabled = enabled;

		const float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		const UINT sample_mask = 0xFFFFFFFF;
		if( enabled )
		{
			DXGI_CONTEXT_CALL( m_pDeviceContext->OMSetBlendState( m_pDevice->GetBlendEnabledState(), blend_factor, sample_mask ) );
		}
		else
		{
			DXGI_CONTEXT_CALL( m_pDeviceContext->OMSetBlendState( m_pDevice->GetBlendDisabledState(), blend_factor, sample_mask ) );
		}
	}

	size_t D3DGPUContext::GetRenderTargetCount() const
	{
		if( m_RenderTargetStack.empty() )
		{
			return 0;
		}

		return m_RenderTargetStack.back().size();
	}

	IRenderTarget* D3DGPUContext::GetRenderTarget(size_t slot) const
	{
		if( m_RenderTargetStack.empty() )
		{
			return nullptr;
		}

		return m_RenderTargetStack.back()[slot];
	}

	void D3DGPUContext::SetRenderTarget( IRenderTarget* pRT, size_t array_index, size_t mip_index )
	{
		m_iRenderTargetArrayIndex = array_index;
		m_iRenderTargetMipIndex = mip_index;
		if( m_RenderTargetStack.empty() )
		{
			m_RenderTargetStack.push_back( { pRT } );
		}
		else
		{
			m_RenderTargetStack.back() = { pRT };
		}

		BindRenderTarget();
	}

	void D3DGPUContext::SetRenderTargets( const std::vector<IRenderTarget*>& pRT )
	{
		if( m_RenderTargetStack.empty() )
		{
			m_RenderTargetStack.push_back( pRT );
		}
		else
		{
			m_RenderTargetStack.back() = pRT;
		}

		BindRenderTarget();
	}

	void D3DGPUContext::PushRenderTarget()
	{
		m_RenderTargetStack.push_back( {} );
		BindRenderTarget();
	}

	void D3DGPUContext::PushRenderTarget( IRenderTarget* pRT )
	{
		m_RenderTargetStack.push_back( { pRT } );
		BindRenderTarget();
	}

	void D3DGPUContext::PushRenderTargets( const std::vector<IRenderTarget*>& pRTs )
	{
		m_RenderTargetStack.push_back( pRTs );
	}

	void D3DGPUContext::PopRenderTarget()
	{
		m_RenderTargetStack.pop_back();
		BindRenderTarget();
	}

	void D3DGPUContext::UnbindRenderTargets()
	{
		if( !m_RenderTargetStack.empty() )
		{
			m_RenderTargetStack.back() = {};
		}
		BindRenderTarget();
	}

	void D3DGPUContext::ClearRenderTargetStack()
	{
		m_RenderTargetStack.clear();
		BindRenderTarget();
	}

	void D3DGPUContext::SetRenderTargetAndViewport( IRenderTarget* pRT )
	{
		SetRenderTarget( pRT );

		Viewport vp;
		vp.top_left = { 0.0f, 0.0f };
		vp.width = (float)pRT->GetWidth();
		vp.height = (float)pRT->GetHeight();
		vp.min_depth = 0.0f;
		vp.max_depth = 1.0f;
		SetViewport( vp );
	}

	void D3DGPUContext::PushRenderTargetAndViewport( IRenderTarget* pRT )
	{
		PushRenderTarget( pRT );

		Viewport vp;
		vp.top_left = { 0.0f, 0.0f };
		vp.width = (float)pRT->GetWidth();
		vp.height = (float)pRT->GetHeight();
		vp.min_depth = 0.0f;
		vp.max_depth = 1.0f;
		PushViewport( vp );
	}

	void D3DGPUContext::PopRenderTargetAndViewport()
	{
		PopRenderTarget();
		PopViewport();
	}

	void D3DGPUContext::ClearDepthStencil( IDepthStencil* pDepthStencil, int clearflag, float depth, uint8_t stencil, size_t index )
	{
		ID3D11DepthStencilView* pView = static_cast<D3DDepthStencil*>( pDepthStencil )->GetDepthStencilView( index );

		DXGI_CONTEXT_CALL( m_pDeviceContext->ClearDepthStencilView( pView, (UINT)clearflag, (FLOAT)depth, (UINT8)stencil ) );
	}

	void D3DGPUContext::Resolve( IRenderTarget* pDst, IRenderTarget* pSrc )
	{
		ASSERT( pDst->GetFormat() == pSrc->GetFormat(), "Cannot resolve to a render-target of a different type" );

		auto format = (DXGI_FORMAT)pDst->GetFormat();
		ID3D11Resource* pDstResource = static_cast<D3DRenderTarget*>( pDst )->GetResource();
		ID3D11Resource* pSrcResource = static_cast<D3DRenderTarget*>( pSrc )->GetResource();
		m_pDeviceContext->ResolveSubresource( pDstResource, 0, pSrcResource, 0, format );
	}

	void D3DGPUContext::ClearRenderTarget( IRenderTarget* pRT, float r, float g, float b, float a, size_t array_index, size_t mip_index )
	{
		if( !pRT )
		{
			pRT = m_pDevice->GetBackbuffer();
		}

		ID3D11RenderTargetView* pView = static_cast<D3DRenderTarget*>( pRT )->GetRenderTargetView( array_index, mip_index );
		float colour[4];
		colour[0] = r;
		colour[1] = g;
		colour[2] = b;
		colour[3] = a;
		DXGI_CONTEXT_CALL( m_pDeviceContext->ClearRenderTargetView( pView, colour ) );
	}

	void D3DGPUContext::UpdateTexturePixels( ITexture* pTexture, const void* pPixels, size_t pitch )
	{
		static_cast<D3DTexture*>( pTexture )->UpdatePixels( m_pDeviceContext.Get(), pPixels, pitch );
	}

	void D3DGPUContext::BindTexture( ITexture* pTexture, size_t slot )
	{
		ID3D11ShaderResourceView* srv = static_cast<D3DTexture*>( pTexture )->GetShaderResourceView();
		DXGI_CONTEXT_CALL( m_pDeviceContext->PSSetShaderResources( (UINT)slot, 1, &srv ) );
	}

	void D3DGPUContext::BindTexture( IRenderTarget* pRT, size_t slot )
	{
		ID3D11ShaderResourceView* srv = static_cast<D3DRenderTarget*>( pRT )->GetShaderResourceView();
		DXGI_CONTEXT_CALL( m_pDeviceContext->PSSetShaderResources( (UINT)slot, 1, &srv ) );
	}

	void D3DGPUContext::BindTexture( IDepthStencil* pDepthStencil, size_t slot )
	{
		ID3D11ShaderResourceView* srv = static_cast<D3DDepthStencil*>( pDepthStencil )->GetShaderResourceView();
		DXGI_CONTEXT_CALL( m_pDeviceContext->PSSetShaderResources( (UINT)slot, 1, &srv ) );
	}

	void D3DGPUContext::UnbindTextureSlot( size_t slot )
	{
		ID3D11ShaderResourceView* pNullResource = nullptr;
		DXGI_CONTEXT_CALL( m_pDeviceContext->PSSetShaderResources( (UINT)slot, 1, &pNullResource ) );
	}

	void D3DGPUContext::UpdateBuffer( IVertexBuffer* pBuffer, const void* pData, size_t count, size_t offset )
	{
		static_cast<D3DVertexBuffer*>( pBuffer )->UpdateBuffer( m_pDeviceContext.Get(), pData, count, offset );
	}

	void* D3DGPUContext::Lock( IVertexBuffer* pBuffer )
	{
		return static_cast<D3DVertexBuffer*>(pBuffer)->Lock( m_pDeviceContext.Get() );
	}

	void D3DGPUContext::Unlock( IVertexBuffer* pBuffer )
	{
		static_cast<D3DVertexBuffer*>(pBuffer)->Unlock( m_pDeviceContext.Get() );
	}

	void D3DGPUContext::UpdateBuffer( IIndexBuffer* pBuffer, const void* pData, size_t count, size_t offset )
	{
		static_cast<D3DIndexBuffer*>( pBuffer )->UpdateBuffer( m_pDeviceContext.Get(), pData, count, offset );
	}

	void* D3DGPUContext::Lock( IIndexBuffer* pBuffer )
	{
		return static_cast<D3DIndexBuffer*>(pBuffer)->Lock( m_pDeviceContext.Get() );
	}

	void D3DGPUContext::Unlock( IIndexBuffer* pBuffer )
	{
		static_cast<D3DIndexBuffer*>(pBuffer)->Unlock( m_pDeviceContext.Get() );
	}

	void D3DGPUContext::UpdateBuffer( IConstantBuffer* pBuffer, const void* pData, size_t count, size_t offset )
	{
		static_cast<D3DConstantBuffer*>( pBuffer )->UpdateBuffer( m_pDeviceContext.Get(), pData, count, offset );
	}

	void* D3DGPUContext::Lock( IConstantBuffer* pBuffer )
	{
		return static_cast<D3DConstantBuffer*>(pBuffer)->Lock( m_pDeviceContext.Get() );
	}

	void D3DGPUContext::Unlock( IConstantBuffer* pBuffer )
	{
		static_cast<D3DConstantBuffer*>(pBuffer)->Unlock( m_pDeviceContext.Get() );
	}

	IPixelShader* D3DGPUContext::GetPixelShader() const
	{
		return m_pPixelShader;
	}

	void D3DGPUContext::SetPixelShader( IPixelShader* pShader )
	{
		m_pPixelShader = static_cast<D3DPixelShader*>( pShader );
		auto pD3DDevice = static_cast<ID3D11Device*>(m_pDevice->GetImpl());
		DXGI_CONTEXT_CALL( m_pDeviceContext->PSSetShader( m_pPixelShader ? m_pPixelShader->GetShader( pD3DDevice ) : nullptr, nullptr, 0 ) );
	}

	IVertexShader* D3DGPUContext::GetVertexShader() const
	{
		return m_pVertexShader;
	}

	void D3DGPUContext::SetVertexShader( IVertexShader* pShader )
	{
		m_pVertexShader = static_cast<D3DVertexShader*>( pShader );
		auto pD3DDevice = static_cast<ID3D11Device*>(m_pDevice->GetImpl());
		DXGI_CONTEXT_CALL( m_pDeviceContext->VSSetShader( m_pVertexShader ? m_pVertexShader->GetShader( pD3DDevice ) : nullptr, nullptr, 0 ) );
		DXGI_CONTEXT_CALL( m_pDeviceContext->IASetInputLayout( m_pVertexShader ? m_pVertexShader->GetLayout() : nullptr ) );
	}

	void D3DGPUContext::SetVertexBuffer( IVertexBuffer* pBuffer, size_t slot )
	{
		auto pD3DBuffer = static_cast<D3DVertexBuffer*>( pBuffer );

		ID3D11Buffer* pVertexBuffer = pD3DBuffer ? pD3DBuffer->GetBuffer() : nullptr;
		UINT stride = pD3DBuffer ? (UINT)pD3DBuffer->GetElementSize() : 0;
		UINT offset = 0;

		DXGI_CONTEXT_CALL( m_pDeviceContext->IASetVertexBuffers( (UINT)slot, 1, &pVertexBuffer, &stride, &offset ) );
	}

	void D3DGPUContext::SetIndexBuffer( IIndexBuffer* pBuffer )
	{
		auto pD3DBuffer = static_cast<D3DIndexBuffer*>( pBuffer );

		size_t elem_size = pD3DBuffer->GetElementSize();
		DXGI_FORMAT format;
		switch( elem_size )
		{
			case 2:
				format = DXGI_FORMAT_R16_UINT;
				break;
			case 4:
				format = DXGI_FORMAT_R32_UINT;
				break;
			default:
				ASSERT( false, "Invalid index buffer element size '%i'. Must be 1, 2 or 4" );
				return;
		}

		m_pDeviceContext->IASetIndexBuffer( pD3DBuffer->GetBuffer(), format, 0 );
	}

	void D3DGPUContext::SetConstantBuffer( ShaderType shader, IConstantBuffer* pBuffer, size_t slot )
	{
		ID3D11Buffer* pConstantBuffer = static_cast<D3DConstantBuffer*>( pBuffer )->GetBuffer();
		switch( shader )
		{
			case ShaderType::VERTEX:
				DXGI_CONTEXT_CALL( m_pDeviceContext->VSSetConstantBuffers( (UINT)slot, 1, &pConstantBuffer ) );
				break;
			case ShaderType::PIXEL:
				DXGI_CONTEXT_CALL( m_pDeviceContext->PSSetConstantBuffers( (UINT)slot, 1, &pConstantBuffer ) );
				break;
			case ShaderType::HULL:
				DXGI_CONTEXT_CALL( m_pDeviceContext->HSSetConstantBuffers( (UINT)slot, 1, &pConstantBuffer ) );
				break;
			case ShaderType::GEOMETRY:
				DXGI_CONTEXT_CALL( m_pDeviceContext->GSSetConstantBuffers( (UINT)slot, 1, &pConstantBuffer ) );
				break;
			case ShaderType::COMPUTE:
				DXGI_CONTEXT_CALL( m_pDeviceContext->CSSetConstantBuffers( (UINT)slot, 1, &pConstantBuffer ) );
				break;
			default:
				ASSERT( false, "Unknown shader type '%i'", (int)shader );
				break;
		}
	}

	void D3DGPUContext::SetSampler( ShaderType shader, ISampler* pSampler, size_t slot )
	{
		ID3D11SamplerState* pSamplerState = static_cast<D3DSampler*>( pSampler )->GetSamplerState();
		switch( shader )
		{
			case ShaderType::VERTEX:
				DXGI_CONTEXT_CALL( m_pDeviceContext->VSSetSamplers( (UINT)slot, 1, &pSamplerState ) );
				break;
			case ShaderType::PIXEL:
				DXGI_CONTEXT_CALL( m_pDeviceContext->PSSetSamplers( (UINT)slot, 1, &pSamplerState ) );
				break;
			case ShaderType::HULL:
				DXGI_CONTEXT_CALL( m_pDeviceContext->HSSetSamplers( (UINT)slot, 1, &pSamplerState ) );
				break;
			case ShaderType::GEOMETRY:
				DXGI_CONTEXT_CALL( m_pDeviceContext->GSSetSamplers( (UINT)slot, 1, &pSamplerState ) );
				break;
			case ShaderType::COMPUTE:
				DXGI_CONTEXT_CALL( m_pDeviceContext->CSSetSamplers( (UINT)slot, 1, &pSamplerState ) );
				break;
			default:
				ASSERT( false, "Unknown shader type '%i'", (int)shader );
				break;
		}
	}

	void D3DGPUContext::Draw( size_t vertex_count )
	{
		DXGI_CONTEXT_CALL( m_pDeviceContext->Draw( (UINT)vertex_count, 0 ) );
	}

	void D3DGPUContext::DrawInstanced( size_t vertex_count, size_t instance_count )
	{
		DXGI_CONTEXT_CALL( m_pDeviceContext->DrawInstanced( (UINT)vertex_count, (UINT)instance_count, 0, 0 ) );
	}

	void D3DGPUContext::DrawIndexed( size_t index_count )
	{
		DXGI_CONTEXT_CALL( m_pDeviceContext->DrawIndexed( (UINT)index_count, 0, 0 ) );
	}

	void D3DGPUContext::DrawInstancedIndexed( size_t index_count, size_t instance_count )
	{
		DXGI_CONTEXT_CALL( m_pDeviceContext->DrawIndexedInstanced( (UINT)index_count, (UINT)instance_count, 0, 0, 0 ) );
	}

	void D3DGPUContext::BeginEvent( const std::string& name )
	{
		if( m_pAnnotation )
		{
			std::wstring wname = StringToWide( name );
			m_pAnnotation->BeginEvent( wname.c_str() );
		}
	}

	void D3DGPUContext::EndEvent()
	{
		if( m_pAnnotation )
		{
			m_pAnnotation->EndEvent();
		}
	}

	void D3DGPUContext::BindViewports()
	{
		size_t count = m_ViewportStack.back().size();

		std::vector<D3D11_VIEWPORT> d3d11vps;
		d3d11vps.resize( count );

		for( size_t i = 0; i < count; i++ )
		{
			const Viewport& vp = m_ViewportStack.back()[i];
			d3d11vps[i].Width = vp.width;
			d3d11vps[i].Height = vp.height;
			d3d11vps[i].MinDepth = vp.min_depth;
			d3d11vps[i].MaxDepth = vp.max_depth;
			d3d11vps[i].TopLeftX = vp.top_left.x;
			d3d11vps[i].TopLeftY = vp.top_left.y;
		}

		DXGI_CONTEXT_CALL( m_pDeviceContext->RSSetViewports( (UINT)count, d3d11vps.data() ) );
	}

	void D3DGPUContext::BindRenderTarget()
	{
		size_t count = m_RenderTargetStack.empty() ? 0 : m_RenderTargetStack.back().size();

		ID3D11DepthStencilView* pDSV = m_pDepthStencil ? m_pDepthStencil->GetDepthStencilView( m_iDepthStencilIndex ) : nullptr;

		if( count )
		{
			std::vector<ID3D11RenderTargetView*> d3d11rts;
			d3d11rts.reserve( count );

			for( size_t i = 0; i < count; i++ )
			{
				const IRenderTarget* rt = m_RenderTargetStack.back()[i];
				if( rt )
				{
					d3d11rts.push_back( static_cast<const D3DRenderTarget*>(rt)->GetRenderTargetView( m_iRenderTargetArrayIndex, m_iRenderTargetMipIndex ) );
				}
				else
				{
					d3d11rts.push_back( nullptr );
				}
			}

			DXGI_CONTEXT_CALL( m_pDeviceContext->OMSetRenderTargets( (UINT)count, d3d11rts.data(), pDSV ) );
		}
		else
		{
			DXGI_CONTEXT_CALL( m_pDeviceContext->OMSetRenderTargets( 0, nullptr, pDSV ) );
		}
	}

	void D3DGPUContext::BindDepthStencilState()
	{
		auto state = m_pDevice->GetDepthStencilState( m_CurrentDepthStencilState );
		if( !state )
		{
			state = m_pDevice->AddDepthStencilState( m_CurrentDepthStencilState );
		}
		DXGI_CONTEXT_CALL( m_pDeviceContext->OMSetDepthStencilState( state, 0 ) );
	}

	void D3DGPUContext::BindRasterizerState()
	{
		auto state = m_pDevice->GetRasterizerState( m_CurrentRasterizerState );
		if( !state )
		{
			state = m_pDevice->AddRasterizerState( m_CurrentRasterizerState );
		}
		DXGI_CONTEXT_CALL( m_pDeviceContext->RSSetState( state ) );
	}

	D3DGPUDevice::D3DGPUDevice( Window& wnd, bool vsync_enabled, float screen_depth, float screen_near )
		:
		m_GPUContext( this ),
		m_bVSyncEnabled( vsync_enabled )
	{
		// Choose best device available based on VRAM
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		{
			Microsoft::WRL::ComPtr<IDXGIFactory> factory;
			COM_THROW_IF_FAILED( CreateDXGIFactory( IID_PPV_ARGS( &factory ) ) );

			{
				Microsoft::WRL::ComPtr<IDXGIAdapter> a;
				SIZE_T max_vram = 0;
				for( int i = 0; SUCCEEDED( factory->EnumAdapters( i, &a ) ); i++ )
				{
					DXGI_ADAPTER_DESC adapterDesc;
					COM_THROW_IF_FAILED( a->GetDesc( &adapterDesc ) );

					if( adapterDesc.DedicatedVideoMemory >= max_vram )
					{
						max_vram = adapterDesc.DedicatedVideoMemory;
						adapter = a;
					}
				}
			}

			DXGI_ADAPTER_DESC adapterDesc;
			COM_THROW_IF_FAILED( adapter->GetDesc( &adapterDesc ) );

			m_DeviceInfo.name = Bat::WideToString( adapterDesc.Description );
			m_DeviceInfo.memory = (size_t)(adapterDesc.DedicatedVideoMemory / 1024 / 1024); // in mb
		}

		/*Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
		COM_THROW_IF_FAILED( adapter->EnumOutputs( 0, &adapterOutput ) );

		UINT numModes;
		COM_THROW_IF_FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL ) );

		DXGI_MODE_DESC* displayModes = new DXGI_MODE_DESC[numModes];

		COM_THROW_IF_FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModes ) );
		*/
		UINT numerator = 0, denominator = 1;
		/*for( UINT i = 0; i < numModes; i++ )
		{
			if( displayModes[i].Width == (UINT)wnd.GetWidth() && displayModes[i].Height == (UINT)wnd.GetHeight() )
			{
				numerator = displayModes[i].RefreshRate.Numerator;
				denominator = displayModes[i].RefreshRate.Denominator;
			}
		}

		delete[] displayModes;*/


		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

			swapChainDesc.BufferCount = 1;
			swapChainDesc.BufferDesc.Width = 0;
			swapChainDesc.BufferDesc.Height = 0;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			if( m_bVSyncEnabled )
			{
				swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
				swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
			}
			else
			{
				swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
				swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			}

			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow = wnd.GetHandle();

			// multisampling
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;

			swapChainDesc.Windowed = !wnd.IsFullscreen();

			swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

			UINT flags = 0;
#ifdef _DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			COM_THROW_IF_FAILED( D3D11CreateDeviceAndSwapChain(
				adapter.Get(),
				D3D_DRIVER_TYPE_UNKNOWN,
				NULL,
				flags,
				NULL, 0,
				D3D11_SDK_VERSION,
				&swapChainDesc, &m_pSwapChain,
				&m_pDevice, NULL, &m_GPUContext.m_pDeviceContext ) );

			m_GPUContext.Init();

			BAT_LOG( "Using video card '%s' with %iMB VRAM",
				m_DeviceInfo.name,
				m_DeviceInfo.memory );
			BAT_LOG( "Device feature level: %s", FeatureLevel2String( m_pDevice->GetFeatureLevel() ) );

			if( m_pDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0 )
			{
				BAT_ERROR( "Your feature level is unsupported, certain features may not work" );
			}

			Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
			COM_THROW_IF_FAILED( m_pSwapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) ) );

			COM_THROW_IF_FAILED( m_pDevice->CreateRenderTargetView( pBackBuffer.Get(), NULL, &m_pRenderTargetView ) );

			m_Backbuffer.Reset( m_pRenderTargetView.Get(), wnd.GetWidth(), wnd.GetHeight() );
		}

		// Create blending enabled/disabled states
		{
			D3D11_BLEND_DESC blend_desc;
			ZeroMemory( &blend_desc, sizeof( blend_desc ) );
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

			blend_desc.RenderTarget[0].BlendEnable = true;
			COM_THROW_IF_FAILED( m_pDevice->CreateBlendState( &blend_desc, &m_pBlendEnabledState ) );

			blend_desc.RenderTarget[0].BlendEnable = false;
			COM_THROW_IF_FAILED( m_pDevice->CreateBlendState( &blend_desc, &m_pBlendDisabledState ) );
		}

#ifdef _DEBUG
		// create the info queue
		typedef HRESULT (WINAPI* LPDXGIGETDEBUGINTERFACE)(REFIID, void**);

		HMODULE dxgidebug = LoadLibraryEx( "dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 );
		if( dxgidebug )
		{
			auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(
				reinterpret_cast<void*>( GetProcAddress( dxgidebug, "DXGIGetDebugInterface" ) ) );

			if( SUCCEEDED( dxgiGetDebugInterface( IID_PPV_ARGS( m_pInfoQueue.GetAddressOf() ) ) ) )
			{
				//m_pInfoQueue->SetMuteDebugOutput( DXGI_DEBUG_ALL, TRUE );
				m_pInfoQueue->ClearStoredMessages( DXGI_DEBUG_ALL );
				m_pInfoQueue->ClearRetrievalFilter( DXGI_DEBUG_ALL );
				m_pInfoQueue->ClearStorageFilter( DXGI_DEBUG_ALL );

				DXGI_INFO_QUEUE_FILTER filter {0};

				std::vector<DXGI_INFO_QUEUE_MESSAGE_SEVERITY> severity_filter_list = {
					// list of severities to filter out
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE,
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO,
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING,
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR,
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION
				};

				if( !severity_filter_list.empty() )
				{
					filter.DenyList.NumSeverities = (UINT)severity_filter_list.size();
					filter.DenyList.pSeverityList = severity_filter_list.data();
				}

#define CREATE_N_DESTROY_MSG( type ) D3D11_MESSAGE_ID_CREATE_##type, D3D11_MESSAGE_ID_DESTROY_##type
				std::vector<DXGI_INFO_QUEUE_MESSAGE_ID> message_filter_list = {
					D3D11_MESSAGE_ID_DEVICE_DRAW_SHADERRESOURCEVIEW_NOT_SET,
					D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
					CREATE_N_DESTROY_MSG( AUTHENTICATEDCHANNEL ),
					CREATE_N_DESTROY_MSG( BLENDSTATE ),
					CREATE_N_DESTROY_MSG( BUFFER ),
					CREATE_N_DESTROY_MSG( CLASSINSTANCE ),
					CREATE_N_DESTROY_MSG( CLASSLINKAGE ),
					CREATE_N_DESTROY_MSG( COMMANDLIST ),
					CREATE_N_DESTROY_MSG( COMPUTESHADER ),
					CREATE_N_DESTROY_MSG( CONTEXT ),
					CREATE_N_DESTROY_MSG( COUNTER ),
					CREATE_N_DESTROY_MSG( CRYPTOSESSION ),
					CREATE_N_DESTROY_MSG( DECODEROUTPUTVIEW ),
					CREATE_N_DESTROY_MSG( DEPTHSTENCILSTATE ),
					CREATE_N_DESTROY_MSG( DEPTHSTENCILVIEW ),
					CREATE_N_DESTROY_MSG( DEVICECONTEXTSTATE ),
					CREATE_N_DESTROY_MSG( DOMAINSHADER ),
					CREATE_N_DESTROY_MSG( FENCE ),
					CREATE_N_DESTROY_MSG( GEOMETRYSHADER ),
					CREATE_N_DESTROY_MSG( HULLSHADER ),
					CREATE_N_DESTROY_MSG( INPUTLAYOUT ),
					CREATE_N_DESTROY_MSG( PIXELSHADER ),
					CREATE_N_DESTROY_MSG( PREDICATE ),
					CREATE_N_DESTROY_MSG( PROCESSORINPUTVIEW ),
					CREATE_N_DESTROY_MSG( PROCESSOROUTPUTVIEW ),
					CREATE_N_DESTROY_MSG( QUERY ),
					CREATE_N_DESTROY_MSG( RASTERIZERSTATE ),
					CREATE_N_DESTROY_MSG( RENDERTARGETVIEW ),
					CREATE_N_DESTROY_MSG( SAMPLER ),
					CREATE_N_DESTROY_MSG( SHADERRESOURCEVIEW ),
					CREATE_N_DESTROY_MSG( SYNCHRONIZEDCHANNEL ),
					CREATE_N_DESTROY_MSG( TEXTURE1D ),
					CREATE_N_DESTROY_MSG( TEXTURE2D ),
					CREATE_N_DESTROY_MSG( TEXTURE3D ),
					CREATE_N_DESTROY_MSG( UNORDEREDACCESSVIEW ),
					CREATE_N_DESTROY_MSG( VERTEXSHADER ),
					CREATE_N_DESTROY_MSG( VIDEODECODER ),
					CREATE_N_DESTROY_MSG( VIDEOPROCESSOR ),
					CREATE_N_DESTROY_MSG( VIDEOPROCESSORENUM ),
				};
#undef CREATE_N_DESTROY_MSG

				if( !message_filter_list.empty() )
				{
					filter.DenyList.NumIDs = (UINT)message_filter_list.size();
					filter.DenyList.pIDList = message_filter_list.data();
				}

				m_pInfoQueue->AddStorageFilterEntries( DXGI_DEBUG_ALL, &filter );
				m_pInfoQueue->AddRetrievalFilterEntries( DXGI_DEBUG_ALL, &filter );

				BAT_TRACE( "Initialized debug layer info queue" );
			}
		}
#endif
	}

	const DeviceInfo& D3DGPUDevice::GetDeviceInfo() const
	{
		return m_DeviceInfo;
	}

	IPixelShader* D3DGPUDevice::CreatePixelShader( const std::string& filename, const std::vector<ShaderMacro>& macros )
	{
		return new D3DPixelShader( m_pDevice.Get(), filename, macros );
	}

	IVertexShader* D3DGPUDevice::CreateVertexShader( const std::string& filename, const std::vector<ShaderMacro>& macros )
	{
		return new D3DVertexShader( m_pDevice.Get(), filename, macros );
	}

	IVertexBuffer* D3DGPUDevice::CreateVertexBuffer( const void* pData, size_t elem_size, size_t size )
	{
		return new D3DVertexBuffer( m_pDevice.Get(), pData, elem_size, size );
	}

	IIndexBuffer* D3DGPUDevice::CreateIndexBuffer( const void* pData, size_t elem_size, size_t size )
	{
		return new D3DIndexBuffer( m_pDevice.Get(), pData, elem_size, size );
	}

	IConstantBuffer* D3DGPUDevice::CreateConstantBuffer( const void* pData, size_t size )
	{
		return new D3DConstantBuffer( m_pDevice.Get(), pData, size );
	}

	ITexture* D3DGPUDevice::CreateTexture( const std::string& filename, TexFlags flags )
	{
		return new D3DTexture( m_pDevice.Get(), filename, flags );
	}

	ITexture* D3DGPUDevice::CreateTexture( const char* pData, size_t size, TexFlags flags )
	{
		return new D3DTexture( m_pDevice.Get(), pData, size, flags );
	}

	ITexture* D3DGPUDevice::CreateTexture( const void* pPixels,
		size_t pitch,
		size_t width,
		size_t height,
		TexFormat format,
		GPUResourceUsage usage,
		TexFlags flags )
	{
		DirectX::Image image;
		image.width = width;
		image.height = height;
		image.format = (DXGI_FORMAT)format;
		image.rowPitch = pitch;
		image.pixels = (uint8_t*)pPixels;

		DirectX::TexMetadata metadata;
		metadata.width = image.width;
		metadata.height = image.height;
		metadata.depth = 1;
		metadata.format = image.format;
		metadata.arraySize = 1;
		metadata.mipLevels = 1;
		metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
		metadata.miscFlags = 0;
		metadata.miscFlags2 = 0;
		if( TexFormatInfo( format ).num_alpha_bits == 0 )
		{
			metadata.SetAlphaMode( DirectX::TEX_ALPHA_MODE_OPAQUE );
		}

		return new D3DTexture( m_pDevice.Get(), image, metadata, usage, flags );
	}

	IDepthStencil* D3DGPUDevice::CreateDepthStencil( size_t width,
		size_t height,
		TexFormat format,
		size_t array_size,
		MsaaQuality ms_quality,
		size_t ms_samples,
		TexFlags flags )
	{
		return new D3DDepthStencil( m_pDevice.Get(), width, height, format, array_size, ms_quality, ms_samples, flags );
	}

	IRenderTarget* D3DGPUDevice::CreateRenderTarget( size_t width,
		size_t height,
		TexFormat format,
		MsaaQuality ms_quality,
		size_t ms_samples,
		TexFlags flags )
	{
		return new D3DRenderTarget( m_pDevice.Get(), width, height, format, ms_quality, ms_samples, flags );
	}

	IRenderTarget* D3DGPUDevice::CreateCubemapRenderTarget( size_t width,
		size_t height,
		TexFormat format,
		size_t mip_levels,
		TexFlags flags )
	{
		return D3DRenderTarget::Cubemap( m_pDevice.Get(), width, height, format, mip_levels, flags );
	}

	IRenderTarget* D3DGPUDevice::GetBackbuffer()
	{
		return &m_Backbuffer;
	}

	const IRenderTarget* D3DGPUDevice::GetBackbuffer() const
	{
		return &m_Backbuffer;
	}

	ISampler* D3DGPUDevice::CreateSampler( const SamplerDesc& sampler_desc )
	{
		return new D3DSampler( m_pDevice.Get(), sampler_desc );
	}

	std::string D3DGPUDevice::FlushMessages()
	{
#ifdef _DEBUG
		std::string msg;

		if( m_pInfoQueue )
		{
			UINT64 nMessages = m_pInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilters( DXGI_DEBUG_ALL );
			for (UINT64 i = 0 ; i < nMessages ; i++ )
			{
				SIZE_T length = 0;
				COM_THROW_IF_FAILED( m_pInfoQueue->GetMessage( DXGI_DEBUG_ALL, i, NULL, &length) );

				auto pMessage = (DXGI_INFO_QUEUE_MESSAGE*)malloc( length );
				COM_THROW_IF_FAILED( m_pInfoQueue->GetMessage( DXGI_DEBUG_ALL, i, pMessage, &length) );

				switch( pMessage->Severity )
				{
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
						BAT_LOG( pMessage->pDescription );
						break;
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
						BAT_WARN( pMessage->pDescription );
						break;
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
						BAT_ERROR( pMessage->pDescription );
						break;
				}

				msg += pMessage->pDescription;
				msg += '\n';

				free( pMessage );
			}

			m_pInfoQueue->ClearStoredMessages( DXGI_DEBUG_ALL );
		}

		return msg;
#else
		return "";
#endif
	}

	void D3DGPUDevice::SwapBuffers()
	{
		HRESULT hr;
		if( FAILED( hr = m_pSwapChain->Present( m_bVSyncEnabled, 0 ) ) )
		{
			if( hr == DXGI_ERROR_DEVICE_REMOVED )
			{
				THROW_COM_ERROR( m_pDevice->GetDeviceRemovedReason(), "Device removed error" );
			}
			else
			{
				THROW_COM_ERROR( hr, "Swap chain present error" );
			}
		}

		auto msg = FlushMessages();
	}

	void D3DGPUDevice::ResizeBuffers( size_t width, size_t height )
	{
		IGPUContext* pContext = GetContext();

		pContext->ClearRenderTargetStack();
		pContext->SetDepthStencil( nullptr );

		m_Backbuffer.Reset( nullptr, 0, 0 );
		m_pRenderTargetView = nullptr;

		DXGI_DEVICE_CALL( m_pSwapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0 ) );

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		COM_THROW_IF_FAILED( m_pSwapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) ) );

		COM_THROW_IF_FAILED( m_pDevice->CreateRenderTargetView( pBackBuffer.Get(), NULL, &m_pRenderTargetView ) );

		m_Backbuffer.Reset( m_pRenderTargetView.Get(), width, height );
	}

	ID3D11DepthStencilState* D3DGPUDevice::GetDepthStencilState( const D3D11_DEPTH_STENCIL_DESC& state )
	{
		auto it = m_pDepthStencilStates.find( state );
		if( it == m_pDepthStencilStates.end() )
		{
			return nullptr;
		}

		return it->second.Get();
	}

	ID3D11DepthStencilState* D3DGPUDevice::AddDepthStencilState( const D3D11_DEPTH_STENCIL_DESC& state )
	{
		auto& depth_stencil_state = m_pDepthStencilStates[state];
		COM_THROW_IF_FAILED( m_pDevice->CreateDepthStencilState( &state, &depth_stencil_state ) );
		return depth_stencil_state.Get();
	}

	ID3D11RasterizerState* D3DGPUDevice::GetRasterizerState( const D3D11_RASTERIZER_DESC& state )
	{
		auto it = m_pRasterizerStates.find( state );
		if( it == m_pRasterizerStates.end() )
		{
			return nullptr;
		}

		return it->second.Get();
	}

	ID3D11RasterizerState* D3DGPUDevice::AddRasterizerState( const D3D11_RASTERIZER_DESC& state )
	{
		auto& rasterizer_state = m_pRasterizerStates[state];
		COM_THROW_IF_FAILED( m_pDevice->CreateRasterizerState( &state, &rasterizer_state ) );
		return rasterizer_state.Get();
	}

	static const char* FeatureLevel2String( D3D_FEATURE_LEVEL level )
	{
		switch( level )
		{
#define STRINGIFY_CASE( s ) case s: return STRINGIFY( s );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_9_1 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_9_2 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_9_3 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_10_0 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_10_1 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_11_0 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_11_1 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_12_0 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_12_1 );
#undef STRINGIFY_CASE

			default:
				return "Unknown";
		}
	}

	IGPUDevice* CreateD3DGPUDevice( Window & wnd, bool vsync_enabled, float screen_depth, float screen_near )
	{
		return new D3DGPUDevice( wnd, vsync_enabled, screen_depth, screen_near );
	}

	D3DPixelShader::D3DPixelShader( ID3D11Device* pDevice, const std::string& filename, const std::vector<ShaderMacro>& macros )
		:
		m_szName( filename ),
		m_Macros( macros )
	{
		LoadFromFile( pDevice, filename, true );

#ifdef _DEBUG
		m_hFileWatch = FileWatchdog::AddFileChangeListener( filename, BIND_MEM_FN( D3DPixelShader::OnFileChanged ) );

		D3D_SET_OBJECT_NAME_N_A( m_pShader, (UINT)filename.size(), filename.c_str() );
#endif
	}

	D3DPixelShader::~D3DPixelShader()
	{
#ifdef _DEBUG
		FileWatchdog::RemoveFileChangeListener( m_hFileWatch );
#endif
	}

	ID3D11PixelShader* D3DPixelShader::GetShader( ID3D11Device* pDevice )
	{
		if( IsDirty() )
		{
			LoadFromFile( pDevice, m_szName, false );

			SetDirty( false );
		}

		return m_pShader.Get();
	}

	void D3DPixelShader::LoadFromFile( ID3D11Device* pDevice, const std::string& filename, bool crash_on_error )
	{
		// compiled shader object
		if( Bat::GetFileExtension( filename ) == "cso" )
		{
			auto bytes = MemoryStream::FromFile( filename );
			COM_THROW_IF_FAILED( pDevice->CreatePixelShader( bytes.Base(), bytes.Size(), NULL, &m_pShader ) );
		}
		// not compiled, lets compile ourselves
		else
		{
			std::vector<D3D_SHADER_MACRO> macros;
			for( const ShaderMacro& macro : m_Macros )
			{
				macros.push_back( { macro.name.c_str(), macro.value.c_str() } );
			}
			macros.push_back( { NULL, NULL } ); // Sentinel

			HRESULT hr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;
			Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBuffer;

			UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
			flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

			if( FAILED( hr = D3DCompileFromFile( Bat::StringToWide( filename ).c_str(), macros.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, &errorMessage ) ) )
			{
				if( errorMessage )
				{
					const std::string error = (char*)errorMessage->GetBufferPointer();
					if( crash_on_error )
					{
						THROW_COM_ERROR( hr, Bat::Format( "Failed to compile pixel shader file '%s'\n%s", filename, error ) );
					}
					else
					{
						BAT_ERROR( Bat::Trim( error ) );
						return;
					}
				}
				else
				{
					if( crash_on_error )
					{
						THROW_COM_ERROR( hr, Bat::Format( "Failed to compile pixel shader file '%s'", filename ) );
					}
					else
					{
						BAT_ERROR( "Failed to compile pixel shader file '%s'.", filename );
						return;
					}
				}
			}

			COM_THROW_IF_FAILED( pDevice->CreatePixelShader( pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pShader ) );
		}
	}

	void D3DPixelShader::OnFileChanged( const std::string& filename )
	{
		SetDirty( true );
	}

	D3DVertexShader::D3DVertexShader( ID3D11Device* pDevice, const std::string& filename, const std::vector<ShaderMacro>& macros )
		:
		m_szName( filename ),
		m_Macros( macros )
	{
		LoadFromFile( pDevice, filename, true );

#ifdef _DEBUG
		m_hFileWatch = FileWatchdog::AddFileChangeListener( filename, BIND_MEM_FN( D3DVertexShader::OnFileChanged ) );

		D3D_SET_OBJECT_NAME_N_A( m_pShader, (UINT)filename.size(), filename.c_str() );
#endif
	}

	D3DVertexShader::~D3DVertexShader()
	{
#ifdef _DEBUG
		FileWatchdog::RemoveFileChangeListener( m_hFileWatch );
#endif
	}

	ID3D11VertexShader* D3DVertexShader::GetShader( ID3D11Device* pDevice )
	{
		if( IsDirty() )
		{
			LoadFromFile( pDevice, m_szName, false );

			SetDirty( false );
		}

		return m_pShader.Get();
	}

	void D3DVertexShader::CreateInputLayoutDescFromVertexShaderSignature( ID3D11Device* pDevice, const void* pCodeBytes, const size_t size )
	{
		// Reflect shader info
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pVertexShaderReflection;

		COM_THROW_IF_FAILED(
			D3DReflect( pCodeBytes, size, IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection )
		);
		// Get shader info
		D3D11_SHADER_DESC shaderDesc;
		pVertexShaderReflection->GetDesc( &shaderDesc );

		size_t slot = 0;

		// Read input layout description from shader info
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for( UINT i = 0; i < shaderDesc.InputParameters; i++ )
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			COM_THROW_IF_FAILED( pVertexShaderReflection->GetInputParameterDesc( i, &paramDesc ) );

			auto attribute = AttributeInfo::SemanticToAttribute( paramDesc.SemanticName );
			ASSERT( attribute != VertexAttribute::Invalid, "Unknown semantic type" );

			if( attribute == VertexAttribute::VertexId )
			{
				// Vertex ID is a system value, not handled by us
				continue;
			}

			// fill out input element desc
			D3D11_INPUT_ELEMENT_DESC elementDesc;
			elementDesc.SemanticName = paramDesc.SemanticName;
			elementDesc.SemanticIndex = paramDesc.SemanticIndex;
			elementDesc.InputSlot = (UINT)slot;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			int attribute_index = m_iAttributeCount[(int)attribute];
			if( attribute != VertexAttribute::InstanceData )
			{
				elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				elementDesc.InstanceDataStepRate = 0;
				m_iAttributeCount[(int)attribute]++;

				ASSERT( m_iAttributeSlot[(int)attribute][attribute_index] == -1, "Per-vertex shader attribute slot set twice" );
				m_iAttributeSlot[(int)attribute][attribute_index] = elementDesc.InputSlot;

				slot++;
			}
			else
			{
				elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
				elementDesc.InstanceDataStepRate = 1;
				if( m_iAttributeSlot[(int)attribute][attribute_index] != -1 )
				{
					elementDesc.InputSlot = (UINT)m_iAttributeSlot[(int)attribute][attribute_index];
				}
				else
				{
					m_iAttributeSlot[(int)attribute][attribute_index] = elementDesc.InputSlot;
					slot++;
				}
			}
			
			
			// determine DXGI format
			int val_count = 0;
			uint8_t mask = paramDesc.Mask;
			while( mask & 1 )
			{
				val_count += 1;
				mask >>= 1;
			}

			if( val_count == 1 )
			{
				if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if( val_count == 2 )
			{
				if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if( val_count == 3 )
			{
				if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if( val_count == 4 )
			{
				if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			//save element desc
			inputLayoutDesc.push_back( elementDesc );
		}

		if( !inputLayoutDesc.empty() )
		{
			COM_THROW_IF_FAILED( pDevice->CreateInputLayout( inputLayoutDesc.data(), (UINT)inputLayoutDesc.size(), pCodeBytes, size, &m_pInputLayout ) );
		}
	}

	void D3DVertexShader::LoadFromFile( ID3D11Device* pDevice, const std::string& filename, bool crash_on_error )
	{
		for( int attrib = 0; attrib < (int)VertexAttribute::TotalAttributes; attrib++ )
		{
			for( int index = 0; index < MAX_ATTRIBUTE_COUNT; index++ )
			{
				m_iAttributeSlot[attrib][index] = -1;
			}
			m_iAttributeCount[attrib] = 0;
		}

		// compiled shader object
		if( Bat::GetFileExtension( filename ) == "cso" )
		{
			auto bytes = MemoryStream::FromFile( filename );

			COM_THROW_IF_FAILED( pDevice->CreateVertexShader( bytes.Base(), bytes.Size(), NULL, &m_pShader ) );
			CreateInputLayoutDescFromVertexShaderSignature( pDevice, bytes.Base(), bytes.Size() );
		}
		// not compiled, lets compile ourselves
		else
		{
			std::vector<D3D_SHADER_MACRO> macros;
			for( const ShaderMacro& macro : m_Macros )
			{
				macros.push_back( { macro.name.c_str(), macro.value.c_str() } );
			}
			macros.push_back( { NULL, NULL }  ); // Sentinel

			HRESULT hr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBuffer;

			UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
			flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

			if( FAILED( hr = D3DCompileFromFile( Bat::StringToWide( filename ).c_str(), macros.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, &errorMessage ) ) )
			{
				if( errorMessage )
				{
					std::string error = (char*)errorMessage->GetBufferPointer();
					if( crash_on_error )
					{
						THROW_COM_ERROR( hr, Bat::Format( "Failed to compile vertex shader file '%s'\n%s", filename, error ) );
					}
					else
					{
						BAT_ERROR( Bat::Trim( error ) );
						return;
					}
				}
				else
				{
					if( crash_on_error )
					{
						THROW_COM_ERROR( hr, Bat::Format( "Failed to compile vertex shader file '%s'", filename ) );
					}
					else
					{
						BAT_ERROR( "Failed to compile vertex shader file '%s'.", filename );
						return;
					}
				}
			}

			COM_THROW_IF_FAILED( pDevice->CreateVertexShader( vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_pShader ) );
			CreateInputLayoutDescFromVertexShaderSignature( pDevice, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize() );
		}
	}

	void D3DVertexShader::OnFileChanged( const std::string & filename )
	{
		SetDirty( true );
	}

	D3DTexture::D3DTexture( ID3D11Resource* texture, ID3D11ShaderResourceView* texture_view, TexFormat format, size_t width, size_t height )
	{
		m_pTexture = texture;
		m_pTextureView = texture_view;
		
		m_Format = format;
		m_iWidth = width;
		m_iHeight = height;
		m_bIsTranslucent = TexFormatInfo( format ).num_alpha_bits > 0;
	}

	D3DTexture::D3DTexture( ID3D11Device* pDevice, const std::string& filename, TexFlags flags )
	{
		std::wstring wfilename = Bat::StringToWide( filename );

		DirectX::ScratchImage scratch;
		DirectX::TexMetadata metadata;
		std::string_view ext = Bat::GetFileExtension( filename );

		if( !std::ifstream( filename ) )
		{
			COM_THROW_IF_FAILED(
				DirectX::LoadFromWICFile( L"Assets/error.png", DirectX::WIC_FLAGS_IGNORE_SRGB, &metadata, scratch )
			);
		}
		else if( ext == "dds" )
		{
			COM_THROW_IF_FAILED(
				DirectX::LoadFromDDSFile( wfilename.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratch )
			);
		}
		else if( ext == "tga" )
		{
			COM_THROW_IF_FAILED(
				DirectX::LoadFromTGAFile( wfilename.c_str(), &metadata, scratch )
			);
		}
		else if( ext == "hdr" )
		{
			COM_THROW_IF_FAILED(
				DirectX::LoadFromHDRFile( wfilename.c_str(), &metadata, scratch )
			);
		}
		else
		{
			COM_THROW_IF_FAILED(
				DirectX::LoadFromWICFile( wfilename.c_str(), DirectX::WIC_FLAGS_IGNORE_SRGB, &metadata, scratch )
			);
		}

		const DirectX::Image* image = scratch.GetImage( 0, 0, 0 );
		ASSERT( image, "Failed to load image" );

		InitializeTex( pDevice, *image, metadata, GPUResourceUsage::DEFAULT, flags );
	}

	D3DTexture::D3DTexture( ID3D11Device* pDevice, const char* pData, size_t size, TexFlags flags )
	{
		DirectX::ScratchImage scratch;
		DirectX::TexMetadata metadata;
		COM_THROW_IF_FAILED(
			DirectX::LoadFromWICMemory( pData, size, DirectX::WIC_FLAGS_IGNORE_SRGB, &metadata, scratch )
		);

		const DirectX::Image* image = scratch.GetImage( 0, 0, 0 );
		ASSERT( image, "Failed to load image" );

		InitializeTex( pDevice, *image, metadata, GPUResourceUsage::DEFAULT, flags );
	}

	D3DTexture::D3DTexture( ID3D11Device* pDevice, const DirectX::Image& image, const DirectX::TexMetadata& metadata, GPUResourceUsage usage, TexFlags flags )
	{
		InitializeTex( pDevice, image, metadata, usage, flags );
	}

	void D3DTexture::UpdatePixels( ID3D11DeviceContext* pDeviceContext, const void* pPixels, size_t pitch )
	{
		D3D11_MAPPED_SUBRESOURCE mapped_tex;
		COM_THROW_IF_FAILED( pDeviceContext->Map( m_pTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_tex ) );

		const char* pSrcBytes = reinterpret_cast<const char*>( pPixels );
		char* pDstBytes = reinterpret_cast<char*>( mapped_tex.pData );
		const UINT min_pitch = std::min( (UINT)pitch, mapped_tex.RowPitch );

		// perform the copy line-by-line
		for( size_t y = 0; y < GetHeight(); y++ )
		{
			memcpy( &pDstBytes[y * mapped_tex.RowPitch], &pSrcBytes[y * pitch], min_pitch );
		}

		pDeviceContext->Unmap( m_pTexture.Get(), 0 );
	}

	void D3DTexture::InitializeTex( ID3D11Device* pDevice, const DirectX::Image& image, const DirectX::TexMetadata& metadata, GPUResourceUsage usage, TexFlags flags )
	{
		bool bindable = ( flags & TexFlags::NO_SHADER_BIND ) != TexFlags::NO_SHADER_BIND;
		bool gen_mips = ( flags & TexFlags::NO_GEN_MIPS ) != TexFlags::NO_GEN_MIPS;

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;
		if( gen_mips )
		{
			pDevice->GetImmediateContext( &pDeviceContext );
		}

		UINT access_flags = 0;
		if( usage == GPUResourceUsage::DYNAMIC )
		{
			access_flags = D3D11_CPU_ACCESS_WRITE;
		}

		UINT bind_flags = 0;
		if( bindable )
		{
			bind_flags |= D3D11_BIND_SHADER_RESOURCE;
		}

		UINT misc_flags = 0;
		if( gen_mips )
		{
			ASSERT( bind_flags & D3D11_BIND_SHADER_RESOURCE, "Cannot generate mips for non-bindable textures" )
			misc_flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			bind_flags |= D3D11_BIND_RENDER_TARGET;
		}

		if( metadata.IsCubemap() )
		{
			misc_flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = (UINT)image.width;
		desc.Height = (UINT)image.height;
		desc.ArraySize = (UINT)metadata.arraySize;
		desc.MipLevels = gen_mips ? 0 : (UINT)metadata.mipLevels;
		desc.Format = image.format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = (D3D11_USAGE)usage;
		desc.BindFlags = bind_flags;
		desc.CPUAccessFlags = access_flags;
		desc.MiscFlags = misc_flags;

		ID3D11Texture2D* p2DTexture;
		if( image.pixels && !gen_mips )
		{
			std::vector<D3D11_SUBRESOURCE_DATA> initialData;
			initialData.resize( desc.ArraySize );
			for( UINT i = 0; i < desc.ArraySize; i++ )
			{
				initialData[i].pSysMem = image.pixels + i * image.slicePitch;
				initialData[i].SysMemPitch = (UINT)image.rowPitch;
				initialData[i].SysMemSlicePitch = (UINT)image.slicePitch;
			}
			COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &desc, initialData.data(), &p2DTexture ) );
		}
		else
		{
			COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &desc, nullptr, &p2DTexture ) );
		}

		m_pTexture = static_cast<ID3D11Resource*>( p2DTexture );

		if( image.pixels && gen_mips )
		{
			ASSERT( desc.ArraySize == 1, "Mip generation for array textures is unsupported" );
			pDeviceContext->UpdateSubresource( m_pTexture.Get(), 0, nullptr, image.pixels, (UINT)image.rowPitch, (UINT)image.slicePitch );
		}

		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc( metadata.IsCubemap() ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D, desc.Format );
		COM_THROW_IF_FAILED(
			pDevice->CreateShaderResourceView( m_pTexture.Get(), &srvDesc, &m_pTextureView )
		);

		if( gen_mips )
		{
			pDeviceContext->GenerateMips( m_pTextureView.Get() );
		}

		m_iWidth = image.width;
		m_iHeight = image.height;
		m_Format = (TexFormat)image.format;

		m_bIsTranslucent = ( TexFormatInfo( m_Format ).num_alpha_bits > 0 );
	}

	D3DRenderTarget::D3DRenderTarget( ID3D11Device* pDevice,
		size_t width,
		size_t height,
		TexFormat format,
		MsaaQuality ms_quality,
		size_t ms_samples,
		TexFlags flags )
	{
		m_Format = format;
		m_iWidth = width;
		m_iHeight = height;
		m_Dimension = D3D_SRV_DIMENSION_TEXTURE2D;

		UINT bind_flags = D3D11_BIND_RENDER_TARGET;
		if( ( flags & TexFlags::NO_SHADER_BIND ) != TexFlags::NO_SHADER_BIND )
		{
			bind_flags |= D3D11_BIND_SHADER_RESOURCE;
		}

		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = (UINT)width;
		textureDesc.Height = (UINT)height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = (DXGI_FORMAT)format;
		textureDesc.SampleDesc.Count = (UINT)ms_samples;
		textureDesc.SampleDesc.Quality = (UINT)ms_quality;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = bind_flags;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &textureDesc, NULL, &m_pRenderTargetTexture ) );

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
		renderTargetViewDesc.Format = textureDesc.Format;
		if( ms_samples <= 1 )
		{
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
		}
		else
		{
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		}

		m_pRenderTargetViews.resize( 1 );
		COM_THROW_IF_FAILED(
			pDevice->CreateRenderTargetView( m_pRenderTargetTexture.Get(),
				&renderTargetViewDesc,
				&m_pRenderTargetViews[0]
			)
		);

		if( (flags & TexFlags::NO_SHADER_BIND) != TexFlags::NO_SHADER_BIND )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
			shaderResourceViewDesc.Format = textureDesc.Format;
			if( ms_samples <= 1 )
			{
				shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
				shaderResourceViewDesc.Texture2D.MipLevels = 1;
			}
			else
			{
				shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			}

			COM_THROW_IF_FAILED( pDevice->CreateShaderResourceView( m_pRenderTargetTexture.Get(), &shaderResourceViewDesc, &m_pShaderResourceView ) );
		}
	}

	D3DRenderTarget* D3DRenderTarget::Cubemap( ID3D11Device* pDevice, size_t width, size_t height, TexFormat format, size_t mip_levels, TexFlags flags )
	{
		auto rt = new D3DRenderTarget;
		rt->m_Format = format;
		rt->m_iWidth = width;
		rt->m_iHeight = height;
		rt->m_Dimension = D3D_SRV_DIMENSION_TEXTURECUBE;
		rt->m_iMipLevels = mip_levels;

		UINT bind_flags = D3D11_BIND_RENDER_TARGET;
		if( ( flags & TexFlags::NO_SHADER_BIND ) != TexFlags::NO_SHADER_BIND )
		{
			bind_flags |= D3D11_BIND_SHADER_RESOURCE;
		}

		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = (UINT)width;
		textureDesc.Height = (UINT)height;
		textureDesc.MipLevels = (UINT)mip_levels;
		textureDesc.ArraySize = 6;
		textureDesc.Format = (DXGI_FORMAT)format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = bind_flags;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		COM_THROW_IF_FAILED( pDevice->CreateTexture2D( &textureDesc, NULL, &rt->m_pRenderTargetTexture ) );

		rt->m_pRenderTargetViews.resize( 6 * mip_levels );
		for( size_t mip_index = 0; mip_index < mip_levels; mip_index++ )
		{
			for( size_t array_index = 0; array_index < 6; array_index++ )
			{
				D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
				renderTargetViewDesc.Format = textureDesc.Format;
				renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				renderTargetViewDesc.Texture2DArray.ArraySize = 1;
				renderTargetViewDesc.Texture2DArray.FirstArraySlice = (UINT)array_index;
				renderTargetViewDesc.Texture2DArray.MipSlice = (UINT)mip_index;

				COM_THROW_IF_FAILED(
					pDevice->CreateRenderTargetView( rt->m_pRenderTargetTexture.Get(),
						&renderTargetViewDesc,
						&rt->m_pRenderTargetViews[array_index * mip_levels + mip_index]
					)
				);
			}
		}

		if( ( flags & TexFlags::NO_SHADER_BIND ) != TexFlags::NO_SHADER_BIND )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
			shaderResourceViewDesc.Format = textureDesc.Format;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
			shaderResourceViewDesc.TextureCube.MipLevels = (UINT)mip_levels;

			COM_THROW_IF_FAILED( pDevice->CreateShaderResourceView( rt->m_pRenderTargetTexture.Get(), &shaderResourceViewDesc, &rt->m_pShaderResourceView ) );
		}

		return rt;
	}

	ITexture* D3DRenderTarget::AsTexture() const
	{
		return new D3DTexture( m_pRenderTargetTexture.Get(), m_pShaderResourceView.Get(), m_Format, m_iWidth, m_iHeight );
	}

	D3DVertexBuffer::D3DVertexBuffer( ID3D11Device* pDevice, const void* pData, size_t elem_size, size_t size )
	{
		m_iElemSize = elem_size;
		m_iSize = size;

		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = (UINT)(elem_size * size);
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA* pVertexData = nullptr;
		D3D11_SUBRESOURCE_DATA vertexData;
		if( pData )
		{
			vertexData.pSysMem = pData;
			vertexData.SysMemPitch = 0;
			vertexData.SysMemSlicePitch = 0;
			pVertexData = &vertexData;
		}

		COM_THROW_IF_FAILED( pDevice->CreateBuffer( &vertexBufferDesc, pVertexData, &m_pVertexBuffer ) );
	}

	void D3DVertexBuffer::UpdateBuffer( ID3D11DeviceContext* pDeviceContext, const void* pData, size_t count, size_t offset )
	{
		size_t actual_count = (count > 0) ? count : m_iSize;
		size_t pitch = m_iElemSize * actual_count;

		const char* pSrcBytes = reinterpret_cast<const char*>( pData );
		char* pDstBytes = reinterpret_cast<char*>( Lock( pDeviceContext ) ) + m_iElemSize * offset;

		memcpy( pDstBytes, pSrcBytes, pitch );

		Unlock( pDeviceContext );
	}

	void* D3DVertexBuffer::Lock( ID3D11DeviceContext* pDeviceContext )
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		COM_THROW_IF_FAILED( pDeviceContext->Map( m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) );
		return mapped.pData;
	}

	void D3DVertexBuffer::Unlock( ID3D11DeviceContext* pDeviceContext )
	{
		pDeviceContext->Unmap( m_pVertexBuffer.Get(), 0 );
	}

	D3DIndexBuffer::D3DIndexBuffer( ID3D11Device* pDevice, const void* pData, size_t elem_size, size_t size )
	{
		m_iElemSize = elem_size;
		m_iSize = size;
		
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = (UINT)( elem_size * size );
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA* pIndexData = nullptr;
		D3D11_SUBRESOURCE_DATA indexData;
		if( pData )
		{
			indexData.pSysMem = pData;
			indexData.SysMemPitch = 0;
			indexData.SysMemSlicePitch = 0;
			pIndexData = &indexData;
		}

		COM_THROW_IF_FAILED( pDevice->CreateBuffer( &indexBufferDesc, pIndexData, &m_pIndexBuffer ) );
	}

	void D3DIndexBuffer::UpdateBuffer( ID3D11DeviceContext* pDeviceContext, const void* pData, size_t count, size_t offset )
	{
		size_t actual_count = (count > 0) ? count : m_iSize;
		size_t pitch = m_iElemSize * actual_count;

		const char* pSrcBytes = reinterpret_cast<const char*>(pData);
		char* pDstBytes = reinterpret_cast<char*>(Lock( pDeviceContext )) + m_iElemSize * offset;

		memcpy( pDstBytes, pSrcBytes, pitch );

		Unlock( pDeviceContext );
	}

	void* D3DIndexBuffer::Lock( ID3D11DeviceContext* pDeviceContext )
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		COM_THROW_IF_FAILED( pDeviceContext->Map( m_pIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) );
		return mapped.pData;
	}

	void D3DIndexBuffer::Unlock( ID3D11DeviceContext* pDeviceContext )
	{
		pDeviceContext->Unmap( m_pIndexBuffer.Get(), 0 );
	}

	D3DConstantBuffer::D3DConstantBuffer( ID3D11Device* pDevice, const void* pData, size_t size )
	{
		m_iSize = size;

		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.ByteWidth = static_cast<UINT>(size + (16 - (size % 16)));
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA* pCbufData = nullptr;
		D3D11_SUBRESOURCE_DATA cbuf_data;

		if( pData )
		{
			cbuf_data.pSysMem = pData;
			cbuf_data.SysMemPitch = 0;
			cbuf_data.SysMemSlicePitch = 0;
		}

		COM_THROW_IF_FAILED( pDevice->CreateBuffer( &desc, pCbufData, &m_pConstantBuffer ) );
	}

	void D3DConstantBuffer::UpdateBuffer( ID3D11DeviceContext* pDeviceContext, const void* pData, size_t count, size_t offset )
	{
		size_t pitch = (count > 0) ? count : m_iSize;

		const char* pSrcBytes = reinterpret_cast<const char*>(pData);
		char* pDstBytes = reinterpret_cast<char*>(Lock( pDeviceContext )) + offset;

		memcpy( pDstBytes, pSrcBytes, pitch );

		Unlock( pDeviceContext );
	}

	void* D3DConstantBuffer::Lock( ID3D11DeviceContext* pDeviceContext )
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		COM_THROW_IF_FAILED( pDeviceContext->Map( m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) );
		return mapped.pData;
	}

	void D3DConstantBuffer::Unlock( ID3D11DeviceContext* pDeviceContext )
	{
		pDeviceContext->Unmap( m_pConstantBuffer.Get(), 0 );
	}

	D3DSampler::D3DSampler( ID3D11Device* pDevice, const SamplerDesc & sampler_desc )
	{
		D3D11_SAMPLER_DESC d3dsd;
		d3dsd.Filter = (D3D11_FILTER)sampler_desc.filter;
		d3dsd.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)sampler_desc.address_u;
		d3dsd.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)sampler_desc.address_v;
		d3dsd.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)sampler_desc.address_w;
		d3dsd.MipLODBias = (FLOAT)sampler_desc.mip_lod_bias;
		d3dsd.MaxAnisotropy = (UINT)sampler_desc.max_anisotropy;
		d3dsd.ComparisonFunc = (D3D11_COMPARISON_FUNC)sampler_desc.comparison_func;
		d3dsd.BorderColor[0] = (FLOAT)sampler_desc.border_color[0];
		d3dsd.BorderColor[1] = (FLOAT)sampler_desc.border_color[1];
		d3dsd.BorderColor[2] = (FLOAT)sampler_desc.border_color[2];
		d3dsd.BorderColor[3] = (FLOAT)sampler_desc.border_color[3];
		d3dsd.MinLOD = (FLOAT)sampler_desc.min_lod;
		d3dsd.MaxLOD = (FLOAT)sampler_desc.max_lod;

		pDevice->CreateSamplerState( &d3dsd, &m_pSamplerState );
	}

	D3DDepthStencil::D3DDepthStencil( ID3D11Device* pDevice,
		size_t width,
		size_t height,
		TexFormat format,
		size_t array_size,
		MsaaQuality ms_quality,
		size_t ms_samples,
		TexFlags flags )
	{
		m_iWidth = width;
		m_iHeight = height;
		m_Format = format;
		m_iArraySize = array_size;

		UINT bind_flags = D3D11_BIND_DEPTH_STENCIL;
		if( ( flags & TexFlags::NO_SHADER_BIND ) != TexFlags::NO_SHADER_BIND )
		{
			bind_flags |= D3D11_BIND_SHADER_RESOURCE;
		}

		//Describe our Depth/Stencil Buffer
		D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
		depthStencilBufferDesc.Width = (UINT)width;
		depthStencilBufferDesc.Height = (UINT)height;
		depthStencilBufferDesc.MipLevels = 1;
		depthStencilBufferDesc.ArraySize = (UINT)array_size;
		depthStencilBufferDesc.Format = (DXGI_FORMAT)format;
		depthStencilBufferDesc.SampleDesc.Count = (UINT)ms_samples;
		depthStencilBufferDesc.SampleDesc.Quality = (UINT)ms_quality;
		depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilBufferDesc.BindFlags = bind_flags;
		depthStencilBufferDesc.CPUAccessFlags = 0;
		depthStencilBufferDesc.MiscFlags = 0;

		COM_THROW_IF_FAILED(
			pDevice->CreateTexture2D( &depthStencilBufferDesc, NULL, &m_pDepthStencilBuffer )
		);

		DXGI_FORMAT depth_format;
		switch( format )
		{
		case DXGI_FORMAT_R24G8_TYPELESS: depth_format = DXGI_FORMAT_D24_UNORM_S8_UINT; break;
		case DXGI_FORMAT_R32_FLOAT:      depth_format = DXGI_FORMAT_D32_FLOAT; break;
		case DXGI_FORMAT_R16_UNORM:      depth_format = DXGI_FORMAT_D16_UNORM; break;
		default: ASSERT( false, "Unknown depth stencil format" );
		}

		m_pDepthStencilView.resize( array_size );
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Flags = 0;
		depthStencilViewDesc.Format = depth_format;
		if( array_size <= 1 )
		{
			if( ms_samples <= 1 )
			{
				depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				depthStencilViewDesc.Texture2D.MipSlice = 0;
			}
			else
			{
				depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			}

			COM_THROW_IF_FAILED(
				pDevice->CreateDepthStencilView( m_pDepthStencilBuffer.Get(),
					&depthStencilViewDesc,
					&m_pDepthStencilView[0]
				)
			);
		}
		else
		{
			for( size_t i = 0; i < array_size; i++ )
			{
				if( ms_samples <= 1 )
				{
					depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					depthStencilViewDesc.Texture2DArray.ArraySize = (UINT)1;
					depthStencilViewDesc.Texture2DArray.FirstArraySlice = (UINT)i;
					depthStencilViewDesc.Texture2DArray.MipSlice = 0;
				}
				else
				{
					depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
					depthStencilViewDesc.Texture2DMSArray.ArraySize = (UINT)1;
					depthStencilViewDesc.Texture2DMSArray.FirstArraySlice = (UINT)i;
				}

				COM_THROW_IF_FAILED(
					pDevice->CreateDepthStencilView( m_pDepthStencilBuffer.Get(),
						&depthStencilViewDesc,
						&m_pDepthStencilView[i]
					)
				);
			}
		}

		DXGI_FORMAT shader_format;
		switch( format )
		{
		case DXGI_FORMAT_R24G8_TYPELESS: shader_format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; break;
		case DXGI_FORMAT_R32_FLOAT:      shader_format = DXGI_FORMAT_R32_FLOAT; break;
		case DXGI_FORMAT_R16_UNORM:      shader_format = DXGI_FORMAT_R16_UNORM; break;
		default: ASSERT( false, "Unknown depth stencil format" );
		}


		if( ( flags & TexFlags::NO_SHADER_BIND ) != TexFlags::NO_SHADER_BIND )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC depthShaderResourceViewDesc;
			depthShaderResourceViewDesc.Format = shader_format;
			if( ms_samples <= 1 )
			{
				depthShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				depthShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
				depthShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
				depthShaderResourceViewDesc.Texture2DArray.ArraySize = (UINT)array_size;
				depthShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
			}
			else
			{
				depthShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
				depthShaderResourceViewDesc.Texture2DMSArray.ArraySize = (UINT)array_size;
				depthShaderResourceViewDesc.Texture2DMSArray.FirstArraySlice = 0;
			}

			COM_THROW_IF_FAILED(
				pDevice->CreateShaderResourceView(
					m_pDepthStencilBuffer.Get(),
					&depthShaderResourceViewDesc,
					&m_pDepthShaderResourceView
				)
			);
		}
	}
}