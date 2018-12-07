#pragma once

#include <wrl.h>

#include "COMException.h"
#include "IGraphics.h"

namespace Bat
{
	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		IndexBuffer( const int* pData, const UINT size );
		IndexBuffer( const std::vector<int>& indices );

		void SetData( const int* pData, const UINT size );
		void SetData( const std::vector<int>& indices );

		operator ID3D11Buffer*() const;

		void Bind() const;

		UINT GetIndexCount() const;
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer>	m_pIndexBuffer;
		UINT m_iSize = 0;
	};
}