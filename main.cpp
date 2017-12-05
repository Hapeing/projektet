//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//	   - modified by FLL
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <ctime>
#include <math.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dinput.h>
#include "bth_image.h"
#include "Application.h"
#include "WICTextureLoader\WICTextureLoader.h"


#define WIDTH 1920
#define HEIGHT 1080

using DirectX::XMMATRIX;
using DirectX::XMVECTOR;

#define SAFE_RELEASE(x) {if (x!=nullptr) x->Release();}

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

__declspec(align (16)) float;
#define ALIGN16 __declspec( align(16) )

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);



int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster

	Application app(WIDTH, HEIGHT);
	if (wndHandle)
	{
		HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

		app.CreateDirect3DContext(wndHandle);
		app.SetViewport(); 
		app.CreateShaders();
		app.CreateConstantBuffers();
		app.CreateModels();
		app.InitDirectInput(hInstance);
		app.CreateDeferredRenderer();
		//app.CreateScenes();

		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				app.Update(hInstance);
				app.Render();
			}
		}

		DestroyWindow(wndHandle);
	}

	return (int) msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.lpszClassName = L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, WIDTH, HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

//HRESULT CreateDirect3DContext(HWND wndHandle)
//{
//	// create a struct to hold information about the swap chain
//	DXGI_SWAP_CHAIN_DESC scd;
//
//	// clear out the struct for use
//	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
//
//	// fill the swap chain description struct
//	scd.BufferCount = 1;                                    // one back buffer
//	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
//	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
//	scd.OutputWindow = wndHandle;                           // the window to be used
//	scd.SampleDesc.Count = 1;                               // how many multisamples
//	scd.Windowed = TRUE;                                    // windowed/full-screen mode
//
//	// create a device, device context and swap chain using the information in the scd struct
//	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
//		D3D_DRIVER_TYPE_HARDWARE,
//		NULL,
//		NULL,
//		NULL,
//		D3D11_CREATE_DEVICE_DEBUG,
//		D3D11_SDK_VERSION,
//		&scd,
//		&gSwapChain,
//		&gDevice,
//		NULL,
//		&gDeviceContext);
//
//	if (SUCCEEDED(hr))
//	{
//		// get the address of the back buffer
//		ID3D11Texture2D* pBackBuffer = nullptr;
//		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
//
//		// use the back buffer address to create the render target
//		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
//		pBackBuffer->Release();
//
//		// set the render target as the back buffer
//		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, depthStencilView);
//	}
//	return hr;
//}