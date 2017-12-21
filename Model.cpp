#include "Model.h"
#include "Application.h"

#define SAFE_RELEASE(x) {if(x) x->Release();}

void Model::Draw()
{
	//Leave it up to caller to set shaders

	//Set topology type
	gDeviceContext->IASetPrimitiveTopology(m_topologyType);
	//Send material to Pixel Shader
	gDeviceContext->UpdateSubresource(m_materialCBuffer, 0, NULL, &material, 0, 0);

	if (m_pIndexBuffer)
	{
		UINT offset = 0;
		//Set vbuffer
		gDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_stride, &offset);

		//Bind texture(s)
		if (texture)
			texture->Bind(gDeviceContext);

		//Set ibuffer
		gDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, offset);

		//Draw
		gDeviceContext->DrawIndexed(m_indexCount, 0, 0);
	}
	else
	{
		UINT offset = 0;

		//Bind texture(s)
		if (texture)
			texture->Bind(gDeviceContext);

		//Set vbuffer
		gDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_stride, &offset);

		//Draw
		gDeviceContext->Draw(m_vertexCount, 0);
	}
}

void Model::Update()
{
	XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYawFromVector(m_rotation);
	m_worldMatrix = m_ModelMatrix;
}

//dont forget to Update()
XMMATRIX Model::GetWorldMatrix()
{
	return m_worldMatrix;
}

void Model::SetWorldMatrix(XMMATRIX matrix)
{
	m_ModelMatrix = matrix;
}

void Model::SetShaders(Shader* vs, Shader* ps, Shader* gs)
{
	m_shaders.push_back(vs);
	m_shaders.push_back(ps);
	m_shaders.push_back(gs);
}

void Model::SetShadersAndDraw()
{
	//Bind shaders we have references to
	for (auto shader : m_shaders)
	{
		if (shader)
			shader->Bind();
	}
	//Set topology type
	gDeviceContext->IASetPrimitiveTopology(m_topologyType);
	//Send material to Pixel Shader
	gDeviceContext->UpdateSubresource(m_materialCBuffer, 0, NULL, &material, 0, 0);

	if (m_pIndexBuffer)
	{
		UINT offset = 0;
		//Set vbuffer
		gDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_stride, &offset);

		//Bind texture(s)
		if (texture)
			texture->Bind(gDeviceContext);

		//Set ibuffer
		gDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, offset);

		//Draw
		gDeviceContext->DrawIndexed(m_indexCount, 0, 0);
	}
	else
	{
		UINT offset = 0;

		//Bind texture(s)
		if (texture)
			texture->Bind(gDeviceContext);

		//Set vbuffer
		gDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_stride, &offset);

		//Draw
		gDeviceContext->Draw(m_vertexCount, 0);
	}
}

void Model::RemakeTopology(vector<Vertex_pos3nor3uv2>& verts)
{

	//Release old resources
	m_pVertexBuffer->Release();

	//Create vertex buffer
	{
		// Describe the Vertex Buffer
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		// what type of buffer will this be?
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		// what type of usage (press F1, read the docs)
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		// how big in bytes each element in the buffer is.
		bufferDesc.ByteWidth = verts.size() * sizeof(Vertex_pos3nor3uv2);

		// this struct is created just to set a pointer to the
		// data containing the vertices.
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = verts.data();

		m_vertexCount = verts.size();
		m_stride = sizeof(Vertex_pos3nor3uv2);


		// create a Vertex Buffer
		HRESULT hr = gDevice->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);

		m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	//TODO
	//material.KaR = 1.0f;
	//material.KaG = 1.0f;
	//material.KaB = 1.0f;
	//material.KdR = 1.0f;
	//material.KdG = 1.0f;
	//material.KdB = 1.0f;
	//material.KsR = 1.0f;
	//material.KsG = 1.0f;
	//material.KsB = 1.0f;
	//material.Ns = 1.0f;
	//material.UseTexture = -1.0f;


}

void Model::LoadFromComLib(const char * name, vector<Vertex_pos3nor3uv2>& verts)
{
	m_name = name;

	//make normals
	vector<Vertex_pos3nor3uv2> _verts;
	for (int i = 0; i < verts.size(); i += 3)
	{
		Vertex_pos3nor3uv2 v1 = verts[i];
		Vertex_pos3nor3uv2 v2 = verts[i+1];
		Vertex_pos3nor3uv2 v3 = verts[i+2];

		_verts.push_back(v1);
		_verts.push_back(v2);
		_verts.push_back(v3);
	}

	//Create vertex buffer
	{
		// Describe the Vertex Buffer
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		// what type of buffer will this be?
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		// what type of usage (press F1, read the docs)
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		// how big in bytes each element in the buffer is.
		bufferDesc.ByteWidth = _verts.size() * sizeof(Vertex_pos3nor3uv2);

		// this struct is created just to set a pointer to the
		// data containing the vertices.
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = _verts.data();

		m_vertexCount = _verts.size();
		m_stride = sizeof(Vertex_pos3nor3uv2);


		// create a Vertex Buffer
		HRESULT hr = gDevice->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);

		m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
	material.KaR = .7f;
	material.KaG = .3f;
	material.KaB = .3f;
	material.KdR = .6f;
	material.KdG = .6f;
	material.KdB = .6f;
	material.KsR =  .8f;
	material.KsG =  .8f;
	material.KsB =  .8f;
	material.Ns = 10.0f;
	material.UseTexture = -1.0f;

	
}

void Model::LoadOBJ(const char* name)
{
	m_name = name;

	std::ifstream OBJFile;
	//Open file
	string poop = string(string("Resources/") + name + ".obj");
	OBJFile.open(poop);

	assert(OBJFile.is_open() && "obj file not open");

	std::vector<Vertex_pos3> positions;
	std::vector<Vertex_pos3> normals;
	std::vector<Vertex_uv2> uvs;
	std::vector<Vertex_pos3nor3uv2> verts;
	std::vector<UINT> indicies;
	std::string mtllib;
	bool useTexture = false;
	//Parse vertex, UV and Normal information from the file
	if (OBJFile.is_open())
	{
		std::string input;
		while (!OBJFile.eof())
		{
			OBJFile >> input;
			if (input == "v")
			{
				Vertex_pos3 v;
				OBJFile >> v.x >> v.y >> v.z;
				positions.push_back(v);
			}
			else if (input == "vt")
			{
				Vertex_uv2 v;
				OBJFile >> v.u >> v.v;
				uvs.push_back(v);
			}
			else if (input == "vn")
			{
				Vertex_pos3 v;
				OBJFile >> v.x >> v.y >> v.z;
				normals.push_back(v);
			}
			else if (input == "f")
			{

				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

				std::string line;

				//get the three verts as strings ("X/X/X")
				std::vector<std::string> vStrings;
				std::getline(OBJFile, line);
				split(line, ' ', vStrings);

				//for each vertex
				for (int i = 1; i < vStrings.size(); i++) //v[0] is empty, so we start at [1]
				{
					std::vector<std::string> vIndicies;
					vIndicies.clear();

					Vertex_pos3nor3uv2 vertex;
					ZeroMemory(&vertex, sizeof(vertex));

					//split vertex. We now have index of v, vt and vn as strings.
					split(vStrings[i], '/', vIndicies);

					//positions
					vertex.posX = positions[std::stoi(vIndicies[0]) - 1].x;
					vertex.posY = positions[std::stoi(vIndicies[0]) - 1].y;
					vertex.posZ = positions[std::stoi(vIndicies[0]) - 1].z;

					//UVS
					vertex.u = uvs[std::stoi(vIndicies[1]) - 1].u;
					vertex.v = uvs[std::stoi(vIndicies[1]) - 1].v;

					//normals
					vertex.norX = normals[std::stoi(vIndicies[2]) - 1].x;
					vertex.norY = normals[std::stoi(vIndicies[2]) - 1].y;
					vertex.norZ = normals[std::stoi(vIndicies[2]) - 1].z;

					verts.push_back(vertex);
				}
				//DWORD num1, num2, num3;
				//std::string garbage;
				//OBJFile >> 
				//	num1 >>
				//	garbage >> 
				//	num2 >>
				//	garbage >> 
				//	num3 >>
				//	garbage;

				//indicies.push_back(num1 - 1);
				//indicies.push_back(num2 - 1);
				//indicies.push_back(num3 - 1);
			}

			else if (input == "mtllib")
			{
				OBJFile >> mtllib;
			}

		}
	}

	//Create vertex buffer
	{
		// Describe the Vertex Buffer
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		// what type of buffer will this be?
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		// what type of usage (press F1, read the docs)
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		// how big in bytes each element in the buffer is.
		bufferDesc.ByteWidth = verts.size() * sizeof(Vertex_pos3nor3uv2);

		// this struct is created just to set a pointer to the
		// data containing the vertices.
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = verts.data();

		m_vertexCount = verts.size();
		m_stride = sizeof(Vertex_pos3nor3uv2);


		// create a Vertex Buffer
		HRESULT hr = gDevice->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);

		m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	//If there's a material, load texture (diffuse)
	if (mtllib.size() > 0)
	{
		std::ifstream MTLFile;
		//open
		std::string mtlfile = std::string("Resources/" + mtllib);
		MTLFile.open(mtlfile);

		assert(MTLFile.is_open() && "mtl file not open");

		std::string input;
		while (!MTLFile.eof())
		{
			MTLFile >> input;

			//-----------------
			// Diffuse Texture
			//-----------------
			if (input == "map_Kd")
			{
				useTexture = true;

				MTLFile >> input;
				string narrow_string("\\Resources" + input);
				std::wstring wide_string = std::wstring(narrow_string.begin(), narrow_string.end());
				//this->LoadTexture(wide_string.c_str());

				//Create texture
				Texture* tex = new Texture();
				tex->LoadFromFile(gDevice, wide_string.c_str());
				Application::m_smTextures.insert({ input, tex });
				this->AssignTexture(input);

				break;
			}
			else if (input == "Ks")
			{
				MTLFile >> material.KsR >> material.KsG >> material.KsB;
			}
			else if (input == "Ka")
			{
				MTLFile >> material.KaR >> material.KaG >> material.KaB;
			}
			else if (input == "Kd")
			{
				MTLFile >> material.KdR >> material.KdG >> material.KdB;
			}
			else if (input == "Ns")
			{
				MTLFile >> material.Ns;
			}
		}

		(useTexture) ? 
			material.UseTexture =  1.0f 
		  : material.UseTexture = -1.0f;

		MTLFile.close();
	}
}

void Model::LoadOBJIndexed(const char * name)
{
	m_name = name;

	std::ifstream OBJFile;
	//Open file
	string poop = string(string("Resources/") + name + ".obj");
	OBJFile.open(poop);

	assert(OBJFile.is_open() && "obj file not open");

	std::vector<Vertex_pos3> positions;
	std::vector<Vertex_pos3> normals;
	std::vector<Vertex_uv2> uvs;
	std::vector<Vertex_pos3nor3uv2> verts;
	std::vector<UINT> indicies;
	std::string mtllib;
	bool useTexture = false;
	//Parse vertex, UV and Normal information from the file
	if (OBJFile.is_open())
	{
		std::string input;
		while (!OBJFile.eof())
		{
			OBJFile >> input;
			if (input == "v")
			{
				Vertex_pos3 v;
				OBJFile >> v.x >> v.y >> v.z;
				positions.push_back(v);
			}
			else if (input == "vt")
			{
				Vertex_uv2 v;
				OBJFile >> v.u >> v.v;
				uvs.push_back(v);
			}
			else if (input == "vn")
			{
				Vertex_pos3 v;
				OBJFile >> v.x >> v.y >> v.z;
				normals.push_back(v);
			}
			else if (input == "f")
			{

				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

				std::string line;

				//get the three verts as strings ("X/X/X")
				std::vector<std::string> vStrings;
				std::getline(OBJFile, line);
				split(line, ' ', vStrings);

				//for each vertex
				for (int i = 1; i < vStrings.size(); i++) //v[0] is empty, so we start at [1]
				{
					std::vector<std::string> vIndicies;
					vIndicies.clear();

					Vertex_pos3nor3uv2 vertex;
					ZeroMemory(&vertex, sizeof(vertex));

					//split vertex. We now have index of v, vt and vn as strings.
					split(vStrings[i], '/', vIndicies);

					//positions
					vertex.posX = positions[std::stoi(vIndicies[0]) - 1].x;
					vertex.posY = positions[std::stoi(vIndicies[0]) - 1].y;
					vertex.posZ = positions[std::stoi(vIndicies[0]) - 1].z;

					//UVS
					vertex.u = uvs[std::stoi(vIndicies[1]) - 1].u;
					vertex.v = uvs[std::stoi(vIndicies[1]) - 1].v;

					//normals
					vertex.norX = normals[std::stoi(vIndicies[2]) - 1].x;
					vertex.norY = normals[std::stoi(vIndicies[2]) - 1].y;
					vertex.norZ = normals[std::stoi(vIndicies[2]) - 1].z;

					verts.push_back(vertex);
				}
				//DWORD num1, num2, num3;
				//std::string garbage;
				//OBJFile >> 
				//	num1 >>
				//	garbage >> 
				//	num2 >>
				//	garbage >> 
				//	num3 >>
				//	garbage;

				//indicies.push_back(num1 - 1);
				//indicies.push_back(num2 - 1);
				//indicies.push_back(num3 - 1);
			}

			else if (input == "mtllib")
			{
				OBJFile >> mtllib;
			}

		}
	}

	//Create vertex buffer
	{
		// Describe the Vertex Buffer
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		// what type of buffer will this be?
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		// what type of usage (press F1, read the docs)
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		// how big in bytes each element in the buffer is.
		bufferDesc.ByteWidth = verts.size() * sizeof(Vertex_pos3nor3uv2);

		// this struct is created just to set a pointer to the
		// data containing the vertices.
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = verts.data();

		m_vertexCount = verts.size();
		m_stride = sizeof(Vertex_pos3nor3uv2);


		// create a Vertex Buffer
		HRESULT hr = gDevice->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);

		m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	//If there's a material, load texture (diffuse)
	if (mtllib.size() > 0)
	{
		std::ifstream MTLFile;
		//open
		std::string mtlfile = std::string("Resources/" + mtllib);
		MTLFile.open(mtlfile);

		assert(MTLFile.is_open() && "mtl file not open");

		std::string input;
		while (!MTLFile.eof())
		{
			MTLFile >> input;

			//-----------------
			// Diffuse Texture
			//-----------------
			if (input == "map_Kd")
			{
				useTexture = true;

				MTLFile >> input;
				string narrow_string("\\Resources" + input);
				std::wstring wide_string = std::wstring(narrow_string.begin(), narrow_string.end());
				//this->LoadTexture(wide_string.c_str());

				//Create texture
				Texture* tex = new Texture();
				tex->LoadFromFile(gDevice, wide_string.c_str());
				Application::m_smTextures.insert({ input, tex });
				this->AssignTexture(input);

				break;
			}
			else if (input == "Ks")
			{
				MTLFile >> material.KsR >> material.KsG >> material.KsB;
			}
			else if (input == "Ka")
			{
				MTLFile >> material.KaR >> material.KaG >> material.KaB;
			}
			else if (input == "Kd")
			{
				MTLFile >> material.KdR >> material.KdG >> material.KdB;
			}
			else if (input == "Ns")
			{
				MTLFile >> material.Ns;
			}
		}

		(useTexture) ?
			material.UseTexture = 1.0f
			: material.UseTexture = -1.0f;

		MTLFile.close();
	}
}

void Model::AssignTexture(std::string name)
{
	texture = Application::m_smTextures[name];
}

void Model::LoadTexture(const wchar_t * filename)
{
	//texture = new Texture
	//texture.LoadFromFile(gDevice, filename);
}

void Model::IncreaseRotation(XMVECTOR rotation)
{
	m_rotation = DirectX::XMVectorAdd(m_rotation, rotation);
}

void Model::IncreaseScale(XMVECTOR scale)
{
	m_scale = DirectX::XMVectorAdd(m_scale, scale);
}

void Model::IncreaseTranslation(XMVECTOR translation)
{
	m_translation = DirectX::XMVectorAdd(m_translation, translation);
}










//
//Model& Model::operator=(const Model &other)
//{
//	this->gDevice = other.gDevice;
//	this->gDeviceContext = other.gDeviceContext;
//	this->m_shaders = other.m_shaders;
//	this->
//}











void Model::CreateDebugPlane()
{
	// Array of Structs (AoS)
	Vertex_pos3col3 verts[4] =
	{
		-0.5f, -0.5f, 0.0f,	//v0 pos
		0.0f, 0.0f, 1.0,	//v0 UV

		-0.5f, 0.5f, 0.0f, //v2
		0.0f, 0.0f,	1.0,//v2 UV

		0.5f, -0.5f, 0.0f,	//v1
		0.0f, 0.0f, 1.0,//v1 UV

		0.5f, 0.5f, 0.0f,   //v4 
		0.0f, 0.0f, 1.0, //v4 UV
	};
	
	// Describe the Vertex Buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	// what type of buffer will this be?
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// what type of usage (press F1, read the docs)
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	// how big in bytes each element in the buffer is.
	bufferDesc.ByteWidth = sizeof(verts);

	// this struct is created just to set a pointer to the
	// data containing the vertices.
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = verts;

	m_vertexCount = 4;
	m_stride = sizeof(float) * 6;
	

	// create a Vertex Buffer
	HRESULT hr = gDevice->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);

	m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	
	hr = gDevice->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);
}

Model::Model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Buffer* materialCbuffer)
	: gDevice(device), gDeviceContext(deviceContext), m_materialCBuffer(materialCbuffer)
{
	m_ModelMatrix = DirectX::XMMatrixIdentity();
}

Model::~Model()
{
	SAFE_RELEASE(m_pVertexBuffer)
	SAFE_RELEASE(m_pIndexBuffer)
	SAFE_RELEASE(gDevice)
	SAFE_RELEASE(gDeviceContext)
}
