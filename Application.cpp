#include "Application.h"
#define TWO_PI 6.28318531
#define SPEED_MODIFIER 0.0005
#define DEFAULT_DIRECTORY std::string("C:\\Users\\Finoli\\Desktop\\")
#define WDEFAULT_DIRECTORY std::wstring(L"C:\\Users\\Finoli\\Desktop\\")


TextureMap Application::m_smTextures;

HRESULT Application::CreateDirect3DContext(HWND wndHandle)
{
	//Keep a reference to the window
	hWnd = &wndHandle;

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	
	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT  | DXGI_USAGE_UNORDERED_ACCESS;      // how swap chain is to be used
	scd.OutputWindow = wndHandle;                           // the window to be used
	scd.SampleDesc.Count = 1;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

															// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);

		//create DS
		CreateDepthStencilBuffer();

		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, depthStencilView);

		//UAV for compute shader
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
		ZeroMemory(&uavd, sizeof(uavd));

		uavd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavd.Texture2D.MipSlice = 0;
		
		HRESULT dHR = gDevice->CreateUnorderedAccessView(pBackBuffer, &uavd, &gUAV);


		//--------------------------------------------------------------------------
		// UNUSED, SRV FOR CS
		//--------------------------------------------------------------------------
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
		ZeroMemory(&srvd, sizeof(srvd));
		srvd.Buffer.NumElements = width * height;
		srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvd.Texture2D.MipLevels = 1;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dHR = gDevice->CreateShaderResourceView(pBackBuffer, &srvd, &gSRV);

		pBackBuffer->Release();
	}
	return hr;
}

void Application::CreateConstantBuffers()
{
	//Vertex transformations cbuffer
	{
		ID3D11Buffer* wvp_buffer;
		CreateConstantBuffer(&wvp_buffer, sizeof(DirectX::XMFLOAT4X4) * 3);

		struct tempData
		{
			DirectX::XMFLOAT4X4 temp[3];
		};
		tempData data;
		DirectX::XMStoreFloat4x4(&data.temp[0], DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&data.temp[1], DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&data.temp[2], DirectX::XMMatrixIdentity());

		//init wvp
		gDeviceContext->UpdateSubresource(wvp_buffer, 0, NULL, &data, 0, 0);

		m_vBuffers.insert({ "WVP", wvp_buffer });
	}
	
	//PS Light cbuffer
	{
		struct PointLight
		{
			float x, y, z, w;
			float r, g, b, a;
		};

		PointLight light = {
			0.0, 10.0, 0.0, 1.0, //pos
			.5, 1.0, 0.0, 1.0,	//greenish color
		};

		ID3D11Buffer* light_buffer;
		CreateConstantBuffer(&light_buffer, sizeof(DirectX::XMFLOAT4X4));

		//init wvp
		gDeviceContext->UpdateSubresource(light_buffer, 0, NULL, &light, 0, 0);

		//TODO:? make static
		m_vBuffers.insert({ "LIGHT", light_buffer });
	}

	//PS Camera cbuffer
	{
		XMFLOAT4 cameraPosition;
		XMStoreFloat4(&cameraPosition, camera.GetCameraPosition());
		ID3D11Buffer* camera_buffer;
		CreateConstantBuffer(&camera_buffer, sizeof(DirectX::XMFLOAT4));
		gDeviceContext->UpdateSubresource(camera_buffer, 0, NULL, &camera_buffer, 0, 0);
		m_vBuffers.insert({ "CAMERA", camera_buffer });
	}
}

void Application::CreateShaders()
{
	//Shader compilation

	//Create pointer and 'new' a shader object
	Shader* vs1 = new Shader(gDevice, gDeviceContext);
	//Call create, passing in shader file name and shader type
	vs1->Create(L"Vertex.hlsl", SHADER_TYPE::VERTEX_SHADER);
	//Insert into shader map for easy access 
	m_vShaders.insert({ "vs1", vs1 });

	Shader* ps1 = new Shader(gDevice, gDeviceContext);
	ps1->Create(L"Fragment.hlsl", SHADER_TYPE::PIXEL_SHADER);
	m_vShaders.insert({ "ps1", ps1 });

	Shader* gs1 = new Shader(gDevice, gDeviceContext);
	gs1->Create(L"GeometryShader.hlsl", SHADER_TYPE::GEOMETRY_SHADER);
	m_vShaders.insert({ "gs1", gs1 });

	Shader* cs1 = new Shader(gDevice, gDeviceContext);
	cs1->Create(L"ComputeShader.hlsl", SHADER_TYPE::COMPUTE_SHADER);
	m_vShaders.insert({ "cs1", cs1 });	
	
	//Shader* vs2 = new Shader(gDevice, gDeviceContext);
	//vs2->Create(L"VS_quad.hlsl", SHADER_TYPE::VERTEX_SHADER);
	//m_vShaders.insert({ "VS_quad", vs2 });

	//Shader* ps2 = new Shader(gDevice, gDeviceContext);
	//ps2->Create(L"PS_quad.hlsl", SHADER_TYPE::PIXEL_SHADER);
	//m_vShaders.insert({ "PS_quad", ps2 });
}

void Application::CreateModels()
{
	//Add some default textures

	//Create pointer, 'new' a Texture
	Texture* texCheckered = new Texture();
	//Call load (uses WICTextureLoader)
	texCheckered->LoadFromFile(gDevice, (WDEFAULT_DIRECTORY + L"checkered.png").c_str());
	//Insert into map for easy access
	m_smTextures.insert({ "checkered", texCheckered });

	Texture* texRed = new Texture();
	texRed->LoadFromFile(gDevice, (WDEFAULT_DIRECTORY + L"red.png").c_str());
	m_smTextures.insert({ "red", texRed });

	//Models

	//Create pointer, 'new' a Model
	Model* sphere = new Model(gDevice, gDeviceContext);
	//Call load, passing in directory and name. Name NEEDS to correspond to filename, i.e. sphere.obj
	sphere->LoadOBJ("C:\\Users\\Finoli\\Desktop\\", "sphere");
	//Set shaders to use (VS, PS, GS)
	sphere->SetShaders(m_vShaders["vs1"], m_vShaders["ps1"], nullptr);
	//Assign texture (OBJLoader will create and assign one if the .mtl specifies one
	sphere->AssignTexture("checkered");
	//Insert into vector
	m_vModels.push_back(sphere);
	//Insert into map for easy accessd
	m_mModels.insert({ "sphere",  sphere });

	//lamp
	//Add models
	//Model* model = new Model(gDevice, gDeviceContext);
	//model->LoadOBJ("C:\\Users\\Finoli\\Desktop\\", "lamp_mat");
	//model->SetShaders(m_vShaders["vs1"], m_vShaders["ps1"], nullptr);
	//model->IncreaseTranslation(DirectX::XMVectorSet(0.0, 6.0, 0.0, 0.0));
	////model->LoadTexture(L"C:\\Users\\Finoli\\Desktop\\checkered.png");
	//m_vModels.push_back(model);
	//m_mModels.insert({ "cube", model });

	//Model* model1 = new Model(gDevice, gDeviceContext);
	//model1->LoadOBJ("C:\\Users\\Finoli\\Desktop\\", "plane");
	//model1->SetShaders(m_vShaders["vs1"], m_vShaders["ps1"], nullptr);
	//model1->AssignTexture("red");
	//m_vModels.push_back(model1);
	//m_mModels.insert({ "plane",  model1 });

	//Model* model2 = new Model(gDevice, gDeviceContext);
	//model2->LoadOBJ("C:\\Users\\Finoli\\Desktop\\", "cube_small_uv");
	//model2->SetShaders(m_vShaders["vs1"], m_vShaders["ps1"], nullptr);
	//model2->AssignTexture("checkered");
	//m_vModels.push_back(model2);
	//m_mModels.insert({ "cube_uv",  model2 });

}

//Boring
void Application::SetViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);

	camera.Init(width, height);

	//whatever
	time = std::clock();
}

//Boring
void Application::CreateDepthStencilBuffer()
{
	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));

	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;


	HRESULT dHR = gDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);

	HRESULT dHR2 = gDevice->CreateDepthStencilView(depthStencilBuffer, &descDSV, &depthStencilView);
	//gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, depthStencilView);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	dHR = gDevice->CreateDepthStencilState(&dsDesc, &depthStencilState);
	gDeviceContext->OMSetDepthStencilState(depthStencilState, 1);
}

//Not used, default rasterizer
void Application::CreateRasterizer()
{
	//D3D11_RASTERIZER_DESC rd;
	//ZeroMemory(&rd, sizeof(rd));

	//rd.FillMode = D3D11_FILL_SOLID;
	//rd.CullMode = D3D11_CULL_BACK;
	//rd.FrontCounterClockwise = TRUE;
	//rd.DepthBias = 0;
	//rd.
}

//Update
//Updates time since last update and checks for input. (DetectInput(timeElapsed);)
void Application::Update(HINSTANCE hInstance)
{

	timeElapsed = ((std::clock() - time) / (float)CLOCKS_PER_SEC) - lastTimeElapsed;
	DetectInput(timeElapsed);
	lastTimeElapsed = timeElapsed;
}

//Render
//Some 'updatey' stuff done here
void Application::Render()
{
	gDeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//+-+-+-+-+ Preparations -+-+-+-+
	float clearColor[] = { .5, .5, .5, 1 };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);
	// Bind depth stencil state
	gDeviceContext->OMSetDepthStencilState(depthStencilState, 1);
	//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	


	//Update camera ---------------------
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0.0);
	XMVECTOR camTarget = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);
	camera.SetCameraTarget(camera.GetCameraPosition() + camTarget);
	camera.SetCameraUp(XMVector3TransformCoord(camera.GetCameraUp(), XMMatrixRotationY(camYaw)));
	camera.Update();

	//Update camera cbuffer
	{
		XMFLOAT4 data;
		XMStoreFloat4(&data, camera.GetCameraPosition());
		gDeviceContext->UpdateSubresource(m_vBuffers["CAMERA"], 0, NULL, &data, 0, 0);
	}
	//-----------------------------------


	//Update wvp
	for (auto& model : m_vModels)
	{
		//Update models world
		model->Update();
		//Update Camera and WVP Cbuffer
		camera.SetWorldMatrix(model->GetWorldMatrix());
		//camera.SetWorldMatrix(DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(0), DirectX::XMMatrixRotationY(0)));



		struct WVP
		{
			XMFLOAT4X4 wvp;
			XMFLOAT4X4 w;
			XMFLOAT4X4 it_w;
		};
		WVP wvp;
		
		//Set WVP
		wvp.wvp = camera.GetWVPMatrix();

		//Get world
		XMMATRIX it_w = camera.GetWorldMatrix();

		//Set world
		XMStoreFloat4x4(&wvp.w, DirectX::XMMatrixTranspose(it_w));

		//Calc and set inverse transpose of world
		it_w = XMMatrixTranspose(XMMatrixInverse(&XMMatrixDeterminant(it_w), it_w));
		XMStoreFloat4x4(&wvp.it_w, it_w);

		gDeviceContext->UpdateSubresource(m_vBuffers["WVP"], 0, NULL, &wvp, 0, 0);

		gDeviceContext->VSSetConstantBuffers(0, 1, &m_vBuffers["WVP"]);

		gDeviceContext->PSSetConstantBuffers(0, 1, &m_vBuffers["LIGHT"]);

		gDeviceContext->PSSetConstantBuffers(1, 1, &m_vBuffers["CAMERA"]); 

		//draw the model
		model->SetShadersAndDraw();

	}


	//-----------------------------------.
	//Post processing (gaussian blur) |
	//-----------------------------------´

	//Null views for unbinding 
	ID3D11UnorderedAccessView* nullUAV = { nullptr };
	ID3D11ShaderResourceView* nullSRV = { nullptr };
	ID3D11RenderTargetView* nullview  = { nullptr };
	ID3D11InputLayout* nullayout = { nullptr };

	//Unbind render target (i.e. back buffer)
	gDeviceContext->OMSetRenderTargets(0, &nullview, depthStencilView);
	//Set compute shader
	m_vShaders["cs1"]->Bind();
	//Set UAV (back buffer)
	gDeviceContext->CSSetUnorderedAccessViews(0, 1, &gUAV, 0);

	//Do blur
	//gDeviceContext->Dispatch((UINT)width / 32, (UINT)height / 32, 1);

	//Unbind UAV
	gDeviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);
	//Rebind back buffer as render target
	gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, depthStencilView);

	//Present to screen
	gSwapChain->Present(0, 0);
}

//Creates a basic constant buffer
void Application::CreateConstantBuffer(ID3D11Buffer** buf, size_t size)
{
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = size;
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	HRESULT hr = gDevice->CreateBuffer(&cbbd, NULL, buf);
}

//Boring, initializes Direct Input
bool Application::InitDirectInput(HINSTANCE hInstance)
{
	HRESULT hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard,
		&DIKeyboard,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysMouse,
		&DIMouse,
		NULL);

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(*hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	hr = DIMouse->SetCooperativeLevel(*hWnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true; //TODO
}

//Here keypresses are checked aswell as the mouse state. The camera's rotation (pitch/yaw) is updated accordingly
//for the first person camera.
void Application::DetectInput(double time)
{
	//Create a state for current mouse state
	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];

	//Make sure our application has access to our devices
	DIKeyboard->Acquire();
	DIMouse   ->Acquire();

	//Get the states of keyboard and mosue
	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	DIMouse   ->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	//If we press escape, close app (DOESNT WORK)
	if (keyboardState[DIK_ESCAPE] & 0x80)
		PostMessage(*hWnd, WM_DESTROY, 0, 0);

	//Camera movement (W, A, S, D)
	if (keyboardState[DIK_A] & 0x80)
	{
		camera.MoveCameraRightLeft(-SPEED_MODIFIER * time);
	}
	if (keyboardState[DIK_D] & 0x80)
	{
		camera.MoveCameraRightLeft(SPEED_MODIFIER * time);
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		camera.MoveCameraForwardsBackwards(SPEED_MODIFIER * time);
	}
	if (keyboardState[DIK_S] & 0x80)
	{
		camera.MoveCameraForwardsBackwards(-SPEED_MODIFIER * time);
	}
	//Camera rotation
	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseCurrState.lX * 0.001f;

		camPitch += mouseCurrState.lY * 0.001f;

		mouseLastState = mouseCurrState;
	}

	//Make sure we stay within 2PI radians and -2PI radians
	if (camYaw > TWO_PI) camYaw -= (float)TWO_PI;
	else if (camYaw < 0) camYaw = (float)TWO_PI + camYaw;
	if (camPitch > TWO_PI) camPitch -= (float)TWO_PI;
	else if (camPitch < 0) camPitch = (float)TWO_PI + camPitch;

	//Update our forward and right vectors (so we know which direction is currently forward/backwards and right/left)
	camera.UpdateCameraForwardRight(DirectX::XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0));
	
	//Update last mouse state
	mouseLastState = mouseCurrState;

	return;
}

//Constructor
Application::Application(UINT width, UINT height) : width(width), height(height)
{
}

Application::~Application()
{
	if (gDevice           ) gDevice->Release();
	if (gDeviceContext    ) gDeviceContext->Release();
	if (gSwapChain        ) gSwapChain->Release();
	if (gBackbufferRTV    ) gBackbufferRTV->Release();
	if (depthStencilView  ) depthStencilView->Release();
	if (depthStencilBuffer) depthStencilBuffer->Release();
	if (DIMouse           ) DIMouse->Release();
	if (DIKeyboard        ) DIKeyboard->Release();
	if (depthStencilState ) depthStencilState->Release();
}
