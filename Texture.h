#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

class Texture
{
private:
	ID3D11ShaderResourceView* m_texture = nullptr;
	ID3D11SamplerState* m_sampler       = nullptr; //ignore for now

public:
	HRESULT LoadFromFile(ID3D11Device* device, const wchar_t* filename);

	void Bind(ID3D11DeviceContext* deviceContext);
	Texture();
	~Texture();
};

