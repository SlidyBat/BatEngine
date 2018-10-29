#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include <wrl.h>

class VertexShader
{
public:
	VertexShader( ID3D11Device* pDevice, const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC* pInputElementsDesc, UINT elements );
	~VertexShader();

	void Bind( ID3D11DeviceContext* pDeviceContext );
	void AddSampler( ID3D11Device* pDevice, const D3D11_SAMPLER_DESC* pSamplerDesc );
	void SetResource( ID3D11DeviceContext* pDeviceContext, int slot, ID3D11ShaderResourceView* const pResource );
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_pInputLayout;
	std::vector<ID3D11SamplerState*> m_pSamplerStates;
};