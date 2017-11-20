#include "Shader.h"
#include <string>

#define SAFE_RELEASE(x) {if(x) x->Release();}

Shader::Shader(ID3D11Device * device, ID3D11DeviceContext * deviceContext) : m_device(device), m_deviceContext(deviceContext)
{
}

Shader::~Shader()
{
	SAFE_RELEASE(m_vertexLayout)
	SAFE_RELEASE(m_vertexShader)
	SAFE_RELEASE(m_pixelShader)
	SAFE_RELEASE(m_geometryShader)
	SAFE_RELEASE(m_device)
	SAFE_RELEASE(m_deviceContext)
}


bool Shader::Create(const wchar_t* filename, SHADER_TYPE type)
{
	m_type = type;

	if (type == 5)
	{
		int poop = 0;
	}
	// Binary Large OBject (BLOB), for compiled shader, and errors.
	ID3DBlob* pBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	
	std::string entryPoint, shaderModel;

	switch (m_type)
	{
	case NULL_SHADER:
		return false;
	case VERTEX_SHADER:
	{	
		entryPoint = "VS_main";
		shaderModel = "vs_5_0";
		break;
	}
	case PIXEL_SHADER:
	{
		entryPoint = "PS_main";
		shaderModel = "ps_5_0";
		break;
	}
	case GEOMETRY_SHADER:
	{
		entryPoint = "GS_main";
		shaderModel = "gs_5_0";
		break;
	}
	case COMPUTE_SHADER:
	{
		entryPoint = "CS_main";
		shaderModel = "cs_5_0";
		break;
	}
	default:
		break;
	}

	// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	HRESULT result = D3DCompileFromFile(
		filename,     // filename
		nullptr,		    // optional macros
		nullptr,		    // optional include files
		entryPoint.c_str(),		    // entry point
		shaderModel.c_str(),		    // shader model (target)
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	// shader compile options (DEBUGGING)
		0,				    // IGNORE...DEPRECATED.
		&pBlob,			    // double pointer to ID3DBlob		
		&errorBlob		    // pointer for Error Blob messages.
	);

	// compilation failed?
	if (FAILED(result))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			// release "reference" to errorBlob interface object
			errorBlob->Release();
		}
		if (pBlob)
			pBlob->Release();
		return false;
	}

	switch (m_type)
	{
	case NULL_SHADER:
	{
		break;
	}
	case VERTEX_SHADER:
	{
		m_device->CreateVertexShader(
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			nullptr,
			&m_vertexShader
		);

		//Create input layout
		//TODO
		D3D11_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,							   D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }

		};

		HRESULT hurr = m_device->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_vertexLayout);

		break;
	}
	case PIXEL_SHADER:
	{
		m_device->CreatePixelShader(
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			nullptr,
			&m_pixelShader
		);
		break;
	}
	case GEOMETRY_SHADER:
	{
		m_device->CreateGeometryShader(
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			nullptr,
			&m_geometryShader
		);
		break;
	}
	case COMPUTE_SHADER:
	{
		m_device->CreateComputeShader(
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			nullptr,
			&m_computeShader
		);
		break;
	}
	default:
		break;
	}

	return true;
}

void Shader::Bind()
{
	switch (m_type)
	{
	case NULL_SHADER:
		break;
	case VERTEX_SHADER:
		m_deviceContext->VSSetShader(m_vertexShader,   nullptr, 0);
		m_deviceContext->IASetInputLayout(m_vertexLayout);
		break;
	case PIXEL_SHADER:
		m_deviceContext->PSSetShader(m_pixelShader,	   nullptr, 0);
		break;
	case GEOMETRY_SHADER:
		m_deviceContext->GSSetShader(m_geometryShader, nullptr, 0);
		break;
	case COMPUTE_SHADER:
		m_deviceContext->CSSetShader(m_computeShader,  nullptr, 0);
		break;
	default:
		break;
	}
}
