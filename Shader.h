#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

enum SHADER_TYPE
{
	NULL_SHADER,
	VERTEX_SHADER,
	PIXEL_SHADER,
	GEOMETRY_SHADER,
	COMPUTE_SHADER
};

class Shader
{
private:
	ID3D11Device*         m_device         = nullptr;
	ID3D11DeviceContext*  m_deviceContext  = nullptr;

	ID3D11VertexShader*   m_vertexShader   = nullptr;
	ID3D11PixelShader*    m_pixelShader    = nullptr;
	ID3D11GeometryShader* m_geometryShader = nullptr;
	ID3D11ComputeShader*  m_computeShader  = nullptr;

	ID3D11InputLayout*    m_vertexLayout   = nullptr;

	SHADER_TYPE m_type = NULL_SHADER;

public:
	Shader(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~Shader();

	bool Create(const wchar_t* filename, SHADER_TYPE type);
	void Bind();
};

