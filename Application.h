#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include <unordered_map>
#include "Camera.h"
#include <string>
#include <ctime>
#include <dinput.h>
#include "Texture.h"
#include "Model.h"

using std::vector;
typedef std::unordered_map<std::string, Texture*> TextureMap;

class Application
{
private:

	HWND* hWnd;

	UINT width, height = 0;

	ID3D11Device*              gDevice             = nullptr;
	ID3D11DeviceContext*       gDeviceContext      = nullptr;

	IDXGISwapChain*            gSwapChain          = nullptr;
	ID3D11RenderTargetView*    gBackbufferRTV      = nullptr;

	ID3D11DepthStencilView*    depthStencilView    = nullptr;
	ID3D11Texture2D*           depthStencilBuffer  = nullptr;
	ID3D11DepthStencilState*   depthStencilState   = nullptr;

	ID3D11UnorderedAccessView* gUAV                = nullptr;
	ID3D11UnorderedAccessView* gUAV2		= nullptr;
	ID3D11ShaderResourceView* gSRV                 = nullptr;


	//Models
	vector<Model*> m_vModels;
	std::unordered_map<std::string, Model*> m_mModels;
	//Shaders
	std::unordered_map<std::string, Shader*> m_vShaders;
	//Constant buffers
	std::unordered_map<std::string, ID3D11Buffer*> m_vBuffers;

	// DirectInput ------------------------------------------------
	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;

	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;

	float rotx = 0;
	float rotz = 0;
	float camPitch = 0.0f;
	float camYaw = 0.0f;

	XMMATRIX Rotationx;
	XMMATRIX Rotationz;
	// -----------------------------------------------------------
	Camera camera;
	std::clock_t time;
	float timeElapsed = 0.0f;
	float lastTimeElapsed = 0.0f;
public:
	//Textures
	static TextureMap m_smTextures;


	void CreateConstantBuffer(ID3D11Buffer** buf, size_t size);

	HRESULT CreateDirect3DContext(HWND wndHandle);
	void CreateConstantBuffers();
	void CreateShaders();
	void CreateModels();
	void SetViewport();
	void CreateDepthStencilBuffer();
	void CreateRasterizer();
	void Update(HINSTANCE hInstance);
	void Render();

	
	// DirectInput
	bool InitDirectInput(HINSTANCE hInstance);
	void DetectInput(double time);

	Application(UINT width, UINT height);
	~Application();
};

