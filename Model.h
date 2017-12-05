#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "VertexHeader.h"
#include <vector>
#include "Shader.h"
#include <fstream>
#include <string>
#include "OBJHelpers.h"
#include "Texture.h"


using DirectX::XMVECTOR;
using DirectX::XMMATRIX;

class Model
{
public:

	struct Material
	{	
		float KsR, KsG, KsB, Ns;			//specular color + power
		float KdR, KdG, KdB, UseTexture;	//diffuse color  + useTexture 'boolean'
		float KaR, KaG, KaB, pad2;			//Specular power
	};

private:
	ID3D11Device*		 gDevice             = nullptr;
	ID3D11DeviceContext* gDeviceContext      = nullptr;

	std::vector<Shader*>     m_shaders;

	ID3D11Buffer*		     m_pVertexBuffer = nullptr;
	ID3D11Buffer*		     m_pIndexBuffer  = nullptr;
	
	D3D11_PRIMITIVE_TOPOLOGY m_topologyType  = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT                     m_vertexCount   = 0;
	UINT                     m_indexCount    = 0;
	UINT                     m_stride        = 0;

	Texture* texture                         = nullptr;
	Material material                        = {};
	//transformations for world matrix
	XMVECTOR m_rotation    = DirectX::XMVectorSet(0.0, 0.0, 0.0, 0.0);
	XMVECTOR m_scale       = DirectX::XMVectorSet(1.0, 1.0, 1.0, 0.0);
	XMVECTOR m_translation = DirectX::XMVectorSet(0.0, 0.0, 0.0, 0.0);

	ID3D11Buffer* m_materialCBuffer;

	DirectX::XMMATRIX m_worldMatrix;

public:
	void Draw();
	void Update();

	XMMATRIX GetWorldMatrix();

	void LoadOBJ(const char* name); //TODO: get material
	void AssignTexture(std::string name);
	void LoadTexture(const wchar_t* filename);
	//Euler rotation
	void IncreaseRotation(XMVECTOR rotation);
	void IncreaseScale(XMVECTOR scale);
	void IncreaseTranslation(XMVECTOR translation);


	void SetShaders(Shader* vs, Shader* ps, Shader* gs);
	void SetShadersAndDraw();
	void CreateDebugPlane();

	Model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Buffer* materialBuffer);
	~Model();
};

