#pragma once
#include <d3d11.h>
#include <assert.h>
#include <vector>
#include "Model.h"
#include "Camera.h"
#include "Shader.h"

class DeferredRenderer
{
private:
	
	ID3D11Device*           m_device;
	ID3D11DeviceContext*    m_deviceContext;

	ID3D11RenderTargetView* m_rt_BACKBUFFER;
	ID3D11RenderTargetView* m_rt_NORMAL_RGB;
	ID3D11RenderTargetView* m_rt_DIFFUSE_RGB;
	ID3D11RenderTargetView* m_rt_SPECULAR_RGB_POWER_A;
	ID3D11RenderTargetView* m_rt_POSITION_RGB;

	ID3D11ShaderResourceView* m_sr_NORMAL_RGB;
	ID3D11ShaderResourceView* m_sr_DIFFUSE_RGB;
	ID3D11ShaderResourceView* m_sr_SPECULAR_RGB_POWER_A;
	ID3D11ShaderResourceView* m_sr_POSITION_RGB;

	ID3D11DepthStencilView*   m_DepthStencil;

	Shader* m_GeometryPS;
	Shader* m_GeometryVS;
	Shader* m_LightingPS;
	Shader* m_LightingVS;

	//ID3D11PixelShader*      m_GeometryPS;
	//ID3D11VertexShader*     m_GeometryVS;
	//ID3D11PixelShader*      m_LightingPS;
	//ID3D11VertexShader*     m_LightingVS;

	//Contains world, worldview and worldviewproj matrices
	ID3D11Buffer* m_WVPBuffer;
	ID3D11Buffer* m_LightBuffer;
	ID3D11Buffer* m_CameraBuffer;


	//?
	Camera* camera;

	UINT m_width  = 0;
	UINT m_height = 0;
public:


	//CALL IN ORDER FROM OUTSIDE
	void PrepareForGeometryPass(float camPitch, float camYaw);
	void PrepareForLightingPass();

	void RenderGeometry(vector<Model*> &models);
	void RenderLights();

	void SetBuffers(ID3D11Buffer* wvp, ID3D11Buffer* light, ID3D11Buffer* camera);
	void SetShaders(Shader* PS1, Shader* PS2, Shader* VS1, Shader* VS2);
	void SetRenderTargets();
	void SetCamera(Camera* camera);

	HRESULT Initialize(ID3D11RenderTargetView* backbuffer);
	DeferredRenderer(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, UINT width, UINT height);
	~DeferredRenderer();
};

