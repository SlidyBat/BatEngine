#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "COMException.h"

namespace Bat
{
	class ConstantBuffer
	{
	public:
		ConstantBuffer( const size_t size );
		ConstantBuffer( const void* pData, const size_t size );

		void SetData( const void* pData );

		operator ID3D11Buffer*() const;

		ID3D11Buffer* const * GetAddressOf() const;
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBuffer;
		size_t m_iSize;
	};
}