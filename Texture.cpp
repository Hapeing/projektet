#include "Texture.h"
#include "WICTextureLoader\WICTextureLoader.h"


HRESULT Texture::LoadFromFile(ID3D11Device * device, const wchar_t * filename)
{
	HRESULT hr = CreateWICTextureFromFile(device, filename, nullptr, &m_texture);
	return hr;
}

void Texture::Bind(ID3D11DeviceContext * deviceContext)
{
	if (m_texture)
		deviceContext->PSSetShaderResources(0, 1, &m_texture);
}

Texture::Texture()
{
}


Texture::~Texture()
{
}
