#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "COMException.h"
#include "IGraphics.h"

namespace Bat
{
	class ConstantBuffer
	{
	public:
		ConstantBuffer( const size_t size )
		{
			m_iSize = size;

			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.ByteWidth = static_cast<UINT>(size + (16 - (size % 16)));
			desc.StructureByteStride = 0;

			auto pDevice = g_pGfx->GetDevice();
			COM_THROW_IF_FAILED( pDevice->CreateBuffer( &desc, NULL, &m_pConstantBuffer ) );
		}
		ConstantBuffer( const void* pData, const size_t size )
		{
			m_iSize = size;

			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.ByteWidth = static_cast<UINT>(size + (16 - (size % 16)));
			desc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = pData;
			data.SysMemPitch = 0;
			data.SysMemSlicePitch = 0;

			auto pDevice = g_pGfx->GetDevice();
			COM_THROW_IF_FAILED( pDevice->CreateBuffer( &desc, &data, &m_pConstantBuffer ) );
		}

		void SetData( const void* pData )
		{
			auto pDeviceContext = g_pGfx->GetDeviceContext();

			D3D11_MAPPED_SUBRESOURCE resource;
			COM_THROW_IF_FAILED( pDeviceContext->Map( m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) );
			memcpy( resource.pData, pData, (UINT)m_iSize );
			pDeviceContext->Unmap( m_pConstantBuffer.Get(), 0 );
		}

		operator ID3D11Buffer*() const
		{
			return m_pConstantBuffer.Get();
		}

		ID3D11Buffer* const * GetAddressOf() const
		{
			return m_pConstantBuffer.GetAddressOf();
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBuffer;
		size_t m_iSize;
	};
}