#pragma once

#include <string>
#include "GraphicsFormats.h"
#include "VertexTypes.h"
#include "MathLib.h"

namespace Bat
{
	class IRenderTarget;
	class ITexture;
	class IVertexBuffer;
	class IIndexBuffer;
	class IConstantBuffer;
	class IGPUContext;
	class IPixelShader;
	class IVertexShader;
	class IDepthStencil;
	class ISampler;

	static constexpr size_t MAX_SHADER_MACROS = 16;
	struct ShaderMacro
	{
		ShaderMacro() = default;
		ShaderMacro( const char* name, const char* value )
			:
			name( name ),
			value( value )
		{}

		const char* name = nullptr;
		const char* value = nullptr;
	};

	class IGPUDevice
	{
	public:
		virtual ~IGPUDevice() = default;

		virtual const DeviceInfo& GetDeviceInfo() const = 0;

		virtual IPixelShader* CreatePixelShader( const std::string& filename, const ShaderMacro* macros, size_t num_macros ) = 0;
		virtual IVertexShader*   CreateVertexShader( const std::string& filename, const ShaderMacro* macros, size_t num_macros ) = 0;

		// Creates vertex buffer with size `size`, element size `elem_size` bytes and `data` as initial data.
		// Pass nullptr to leave uninitialized.
		virtual IVertexBuffer*   CreateVertexBuffer( const void* data, size_t elem_size, size_t size ) = 0;
		// Creates index buffer with size `size`, element size `elem_size` bytes and `data` as initial data.
		// Pass nullptr to leave uninitialized.
		// Note: elem_size can only be 2 or 4, and the data type should be unsigned
		virtual IIndexBuffer*    CreateIndexBuffer( const void* data, size_t elem_size, size_t size ) = 0;
		// Creates constant buffer with size `size` bytes and `data` as initial data. Pass nullptr to leave uninitialized.
		virtual IConstantBuffer* CreateConstantBuffer( const void* data, size_t size ) = 0;

		virtual ITexture* CreateTexture( const std::string& filename, TexFlags flags = TexFlags::NONE ) = 0;
		virtual ITexture* CreateTexture( const char* pData, size_t size, TexFlags flags = TexFlags::NONE ) = 0;
		virtual ITexture* CreateTexture( const void* pPixels,
			size_t pitch,
			size_t width,
			size_t height,
			TexFormat format,
			GPUResourceUsage usage = GPUResourceUsage::DEFAULT,
			TexFlags flags = TexFlags::NONE ) = 0;

		virtual IDepthStencil* CreateDepthStencil( size_t width,
			size_t height,
			TexFormat format,
			size_t array_size = 1,
			MsaaQuality ms_quality = MsaaQuality::NONE,
			size_t ms_samples = 1,
			TexFlags flags = TexFlags::NONE ) = 0;

		virtual IRenderTarget* CreateRenderTarget( size_t width,
			size_t height,
			TexFormat format,
			MsaaQuality ms_quality = MsaaQuality::NONE,
			size_t ms_samples = 1,
			TexFlags flags = TexFlags::NONE ) = 0;
		virtual IRenderTarget* CreateCubemapRenderTarget( size_t width,
			size_t height,
			TexFormat format,
			size_t mip_levels = 1,
			TexFlags flags = TexFlags::NONE ) = 0;

		// Gets backbuffer as render target
		virtual IRenderTarget* GetBackbuffer() = 0;
		virtual const IRenderTarget* GetBackbuffer() const = 0;

		virtual ISampler* CreateSampler( const SamplerDesc& sampler_desc ) = 0;

		virtual void SwapBuffers() = 0;

		// Resizes internal buffers to the specified width/height
		// NOTE: Render targets and depth stencils are unbound after this method is called
		virtual void ResizeBuffers( size_t width, size_t height ) = 0;

		// Gets immediate context
		virtual IGPUContext* GetContext() = 0;

		// Gets pointer to underlying implementation
		// e.g. for D3D11 device this returns the ID3D11Device*
		virtual void* GetImpl() = 0;
	};

	class IGPUContext
	{
	public:
		virtual ~IGPUContext() = default;

		virtual IGPUDevice* GetDevice() = 0;
		virtual const IGPUDevice* GetDevice() const = 0;

		virtual void SetPrimitiveTopology( PrimitiveTopology topology ) = 0;
		virtual PrimitiveTopology GetPrimitiveTopology() const = 0;

		// Gets how many viewports are currently bound
		virtual size_t GetViewportCount() const = 0;
		// Get's currently set viewport at specified slot
		virtual const Viewport& GetViewport( size_t slot = 0 ) const = 0;
		// Sets current viewport
		virtual void SetViewport( const Viewport& viewport ) = 0;
		virtual void SetViewports( const std::vector<Viewport>& viewport ) = 0;
		// Pushes a new viewport on to the stack
		virtual void PushViewport( const Viewport& viewport ) = 0;
		virtual void PushViewports( const std::vector<Viewport>& viewport ) = 0;
		// Pops top viewport from stack.
		virtual void PopViewport() = 0;

		// Binds depth stencil buffer
		virtual void SetDepthStencil( IDepthStencil* pDepthStencil, size_t index = 0 ) = 0;
		// Gets currently bound depth stencil, or nullptr if none is bound
		virtual IDepthStencil* GetDepthStencil() const = 0;
		virtual bool IsDepthEnabled() const = 0;
		virtual void SetDepthEnabled( bool enabled ) = 0;
		virtual bool IsDepthWriteEnabled() const = 0;
		virtual void SetDepthWriteEnabled( bool enabled ) = 0;
		virtual ComparisonFunc GetDepthComparisonFunc() const = 0;
		virtual void SetDepthComparisonFunc( ComparisonFunc func ) = 0;
		virtual bool IsStencilEnabled() const = 0;
		virtual void SetStencilEnabled( bool enabled ) = 0;
		virtual uint8_t GetStencilReadMask() const = 0;
		virtual void SetStencilReadMask( uint8_t mask ) = 0;
		virtual uint8_t GetStencilWriteMask() const = 0;
		virtual void SetStencilWriteMask( uint8_t mask ) = 0;
		virtual StencilOpDesc GetStencilFrontFaceOp() const = 0;
		virtual void SetStencilFrontFaceOp( const StencilOpDesc& desc ) = 0;
		virtual StencilOpDesc GetStencilBackFaceOp() const = 0;
		virtual void SetStencilBackFaceOp( const StencilOpDesc& desc ) = 0;

		virtual CullMode GetCullMode() const = 0;
		virtual void SetCullMode( CullMode mode ) = 0;
		virtual bool GetDepthClipEnabled() const = 0;
		virtual void SetDepthClipEnabled( bool enabled ) = 0;

		virtual bool IsBlendingEnabled() const = 0;
		virtual void SetBlendingEnabled( bool enabled ) = 0;

		// Gets how many RTs are currently bound
		virtual size_t GetRenderTargetCount() const = 0;
		// Get's currently bound render target, or nullptr if no render target is bound
		virtual IRenderTarget* GetRenderTarget( size_t slot = 0 ) const = 0;
		// Sets current render target.
		virtual void SetRenderTarget( IRenderTarget* pRT, size_t array_index = 0, size_t mip_index = 0 ) = 0;
		virtual void SetRenderTargets( const std::vector<IRenderTarget*>& pRTs ) = 0;
		// Pushes a new layer in RT stack, but doesn't add any render targets to it. Use this to push a layer without any render targets bound.
		virtual void PushRenderTarget() = 0;
		// Pushes a render target on to RT stack
		virtual void PushRenderTarget( IRenderTarget* pRT ) = 0;
		virtual void PushRenderTargets( const std::vector<IRenderTarget*>& pRTs ) = 0;
		// Pops top render target on RT stack.
		virtual void PopRenderTarget() = 0;
		virtual void UnbindRenderTargets() = 0;
		virtual void ClearRenderTargetStack() = 0;

		// Sets current render target and also sets viewport with same size
		virtual void SetRenderTargetAndViewport( IRenderTarget* pRT ) = 0;
		// Pushes render target on to RT stack and also pushes viewport with same size
		virtual void PushRenderTargetAndViewport( IRenderTarget* pRT ) = 0;
		// Pops top render target & viewport from stack
		virtual void PopRenderTargetAndViewport() = 0;

		// Clears specified render target with the set colour
		virtual void ClearRenderTarget( IRenderTarget* pRT, float r, float g, float b, float a, size_t array_index = 0, size_t mip_index = 0 ) = 0;
		// See ClearFlag enum for list of valid flags (can be ORed together)
		virtual void ClearDepthStencil( IDepthStencil* pDepthStencil, int clearflag, float depth, uint8_t stencil, size_t index = 0 ) = 0;
		virtual void Resolve( IRenderTarget* pDst, IRenderTarget* pSrc ) = 0;

		virtual void UpdateTexturePixels( ITexture* pTexture, const void* pPixels, size_t pitch ) = 0;
		virtual void BindTexture( ITexture* pTexture, size_t slot ) = 0;
		virtual void BindTexture( IRenderTarget* pRT, size_t slot ) = 0;
		virtual void BindTexture( IDepthStencil* pDepthStencil, size_t slot ) = 0;
		virtual void UnbindTextureSlot( size_t slot ) = 0;

		// Update the vertex buffer with the given data
		// Count is the number of vertices to update, or 0 to update the full buffer
		// Offset is the offset from the beginning of the buffer to update from in vertices
		virtual void UpdateBuffer( IVertexBuffer* pBuffer, const void* pData, size_t count = 0, size_t offset = 0 ) = 0;
		virtual void* Lock( IVertexBuffer* pBuffer ) = 0;
		virtual void Unlock( IVertexBuffer* pBuffer ) = 0;
		// Update the index buffer with the given data
		// Count is the number of indices to update, or 0 to update the full buffer
		// Offset is the offset from the beginning of the buffer to update from in indices
		virtual void UpdateBuffer( IIndexBuffer* pBuffer, const void* pData, size_t count = 0, size_t offset = 0 ) = 0;
		virtual void* Lock( IIndexBuffer* pBuffer ) = 0;
		virtual void Unlock( IIndexBuffer* pBuffer ) = 0;
		// Update the constant buffer with the given data
		// Count is the number of bytes to update, or 0 to update the full buffer
		// Offset is the offset from the beginning of the buffer to update from in bytes
		virtual void UpdateBuffer( IConstantBuffer* pBuffer, const void* pData, size_t count = 0, size_t offset = 0 ) = 0;
		virtual void* Lock( IConstantBuffer* pBuffer ) = 0;
		virtual void Unlock( IConstantBuffer* pBuffer ) = 0;

		// Gets currently bound pixel shader, or nullptr if none are bound
		virtual IPixelShader* GetPixelShader() const = 0;
		// Binds a pixel shader. Pass nullptr to unbind pixel shader.
		virtual void SetPixelShader( IPixelShader* pShader ) = 0;
		// Gets currently bound vertex shader, or nullptr if none are bound
		virtual IVertexShader* GetVertexShader() const = 0;
		// Binds a vertex shader. Pass nullptr to unbind vertex shader.
		virtual void SetVertexShader( IVertexShader* pShader ) = 0;

		virtual void SetVertexBuffer( IVertexBuffer* pBuffer, size_t slot = 0 ) = 0;
		virtual void SetIndexBuffer( IIndexBuffer* pBuffer ) = 0;
		virtual void SetConstantBuffer( ShaderType shader, IConstantBuffer* pBuffer, size_t slot ) = 0;

		virtual void SetSampler( ShaderType shader, ISampler* pSampler, size_t slot ) = 0;

		virtual void Draw( size_t vertex_count ) = 0;
		virtual void DrawInstanced( size_t vertex_count, size_t instance_count ) = 0;
		virtual void DrawIndexed( size_t index_count ) = 0;
		virtual void DrawInstancedIndexed( size_t index_count, size_t instance_count ) = 0;

		virtual void BeginEvent( const std::string& name ) = 0;
		virtual void EndEvent() = 0;

		// Gets pointer to underlying implementation
		// e.g. for D3D11 device this returns the ID3D11DeviceContext*
		virtual void* GetImpl() = 0;
	};

	class IPixelShader
	{
	public:
		virtual ~IPixelShader() = default;

		virtual std::string GetName() const = 0;
	};

	class IVertexShader
	{
	public:
		virtual ~IVertexShader() = default;

		virtual std::string GetName() const = 0;
		// Gets the number of elements in the shader that use this attribute, or 0 if the shader doesn't require this attribute at all
		virtual int GetVertexAttributeCount( VertexAttribute attribute ) const = 0;
		// Gets the slot that this shader expects the attribute to be at, or -1 if the shader doesn't require this attribute at all
		virtual int GetVertexAttributeSlot( VertexAttribute attribute, int index ) const = 0;
	};

	class IVertexBuffer
	{
	public:
		virtual ~IVertexBuffer() = default;

		virtual size_t GetVertexCount() const = 0;
		virtual size_t GetElementSize() const = 0;
	};

	class IIndexBuffer
	{
	public:
		virtual ~IIndexBuffer() = default;

		virtual size_t GetIndexCount() const = 0;
		virtual size_t GetElementSize() const = 0;
	};

	class IConstantBuffer
	{
	public:
		virtual ~IConstantBuffer() = default;

		// Gets size of cbuf in bytes
		virtual size_t GetSize() const = 0;
	};

	class ITexture
	{
	public:
		virtual ~ITexture() = default;

		virtual size_t GetWidth() const = 0;
		virtual size_t GetHeight() const = 0;
		virtual TexFormat GetFormat() const = 0;
		virtual bool IsTranslucent() const = 0;
		virtual void* GetImpl() = 0;
	};

	class IDepthStencil
	{
	public:
		virtual ~IDepthStencil() = default;

		virtual size_t GetWidth() const = 0;
		virtual size_t GetHeight() const = 0;
		virtual TexFormat GetFormat() const = 0;
		virtual size_t GetArraySize() const = 0;
	};

	class IRenderTarget
	{
	public:
		virtual ~IRenderTarget() = default;

		virtual size_t GetWidth() const = 0;
		virtual size_t GetHeight() const = 0;
		virtual TexFormat GetFormat() const = 0;
		virtual ITexture* AsTexture() const = 0;
	};

	class ISampler
	{
	public:
		virtual ~ISampler() = default;
	};
}