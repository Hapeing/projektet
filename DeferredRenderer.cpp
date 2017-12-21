#include "DeferredRenderer.h"

void DeferredRenderer::SetBuffers(ID3D11Buffer * wvp, ID3D11Buffer * light, ID3D11Buffer * camera)
{
	m_WVPBuffer    = wvp;
	m_LightBuffer  = light;
	m_CameraBuffer = camera;
}

void DeferredRenderer::SetShaders(Shader* PS1, Shader* PS2, Shader* VS1, Shader* VS2)
{
	m_GeometryPS = PS1;
	m_GeometryVS = VS1;
	m_LightingPS = PS2;
	m_LightingVS = VS2;
}

void DeferredRenderer::SetRenderTargets()
{
	ID3D11RenderTargetView* arr[4] =
	{
		m_rt_NORMAL_RGB,
		m_rt_DIFFUSE_RGB,
		m_rt_SPECULAR_RGB_POWER_A,
		m_rt_POSITION_RGB
	};

	m_deviceContext->OMSetRenderTargets(4, arr, m_DepthStencil);
}

void DeferredRenderer::SetCamera(Camera * cam)
{
	camera = cam;
}

void DeferredRenderer::RenderGeometry(std::unordered_map<std::string, Model*> &models)
{
	m_GeometryPS->Bind();
	m_GeometryVS->Bind();


	for (auto& model : models)
	{
		//Update models world
		model.second->Update();
		//Update Camera and WVP Cbuffer
		camera->SetWorldMatrix(model.second->GetWorldMatrix());
		//camera.SetWorldMatrix(DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(0), DirectX::XMMatrixRotationY(0)));



		struct WVP
		{
			XMFLOAT4X4 wvp;
			XMFLOAT4X4 w;
			XMFLOAT4X4 it_w;
		};
		WVP wvp;

		//Set WVP
		wvp.wvp = camera->GetWVPMatrix();

		//Get world
		XMMATRIX it_w = camera->GetWorldMatrix();

		//Set world
		XMStoreFloat4x4(&wvp.w, DirectX::XMMatrixTranspose(it_w));

		//Calc and set inverse transpose of world
		it_w = XMMatrixTranspose(XMMatrixInverse(&XMMatrixDeterminant(it_w), it_w));
		XMStoreFloat4x4(&wvp.it_w, it_w);

		m_deviceContext->UpdateSubresource(m_WVPBuffer, 0, NULL, &wvp, 0, 0);

		//draw the model
		model.second->Draw();
	}

}

void DeferredRenderer::RenderLights()
{
	m_LightingPS->Bind();
	m_LightingVS->Bind();

	//Remove any v-buffers and layouts
	ID3D11Buffer* nullBuffer = { nullptr };
	ID3D11InputLayout* nullLayout = { nullptr };
	m_deviceContext->IASetInputLayout(nullLayout);
	m_deviceContext->Draw(3, 0);
}

void DeferredRenderer::PrepareForGeometryPass(float camPitch, float camYaw)
{
	//Set G-buffer targets
	SetRenderTargets();

	m_deviceContext->ClearDepthStencilView(m_DepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	
	//Clear render targets (redundant?)
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_deviceContext->ClearRenderTargetView(m_rt_NORMAL_RGB, clearColor);
	m_deviceContext->ClearRenderTargetView(m_rt_DIFFUSE_RGB, clearColor);
	m_deviceContext->ClearRenderTargetView(m_rt_SPECULAR_RGB_POWER_A, clearColor);
	m_deviceContext->ClearRenderTargetView(m_rt_POSITION_RGB, clearColor);

	//Do camera stuff
	//Update camera ---------------------
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0.0);
	XMVECTOR camTarget = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);
	camera->SetCameraTarget(camera->GetCameraPosition() + camTarget);
	camera->SetCameraUp(XMVector3TransformCoord(camera->GetCameraUp(), XMMatrixRotationY(camYaw)));
	camera->Update();

	//Update camera cbuffer
	{
		XMFLOAT4 data;
		XMStoreFloat4(&data, camera->GetCameraPosition());
		m_deviceContext->UpdateSubresource(m_CameraBuffer, 0, NULL, &data, 0, 0);
	}
	//-----------------------------------

	//Do other stuff
	m_deviceContext->VSSetConstantBuffers(0, 1, &m_WVPBuffer);
}

void DeferredRenderer::PrepareForLightingPass()
{
	//m_deviceContext->ClearDepthStencilView(m_DepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	float clearColor[4] = { 0.0f, 0.0f, 0.5f, 1.0f };
	m_deviceContext->ClearRenderTargetView(m_rt_BACKBUFFER, clearColor);
	m_deviceContext->OMSetRenderTargets(1, &m_rt_BACKBUFFER, m_DepthStencil);

	m_deviceContext->PSSetConstantBuffers(0, 1, &m_LightBuffer);
	m_deviceContext->PSSetConstantBuffers(1, 1, &m_CameraBuffer);

	ID3D11ShaderResourceView* arr[4] = 
	{
		m_sr_NORMAL_RGB,
		m_sr_DIFFUSE_RGB,
		m_sr_SPECULAR_RGB_POWER_A,
		m_sr_POSITION_RGB
	};

	m_deviceContext->PSSetShaderResources(0, 4, arr);
}

DeferredRenderer::~DeferredRenderer()
{
	m_rt_NORMAL_RGB          ->Release();
	m_rt_DIFFUSE_RGB         ->Release();
	m_rt_SPECULAR_RGB_POWER_A->Release();
	m_rt_POSITION_RGB        ->Release();

	m_sr_NORMAL_RGB          ->Release();
	m_sr_DIFFUSE_RGB         ->Release();
	m_sr_SPECULAR_RGB_POWER_A->Release();
	m_sr_POSITION_RGB        ->Release();
}

HRESULT DeferredRenderer::Initialize(ID3D11RenderTargetView* backbuffer)
{
	m_rt_BACKBUFFER = backbuffer;
	HRESULT hr = S_OK;

	assert(m_width && m_height);

	ID3D11Texture2D* norm2D;
	ID3D11Texture2D* diff2D;
	ID3D11Texture2D* spec2D;
	ID3D11Texture2D* pos2D;

	//Tex2d desc
	D3D11_TEXTURE2D_DESC desc2D = {};
	desc2D.ArraySize = 1;
	desc2D.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc2D.Format    = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc2D.Width     = m_width;
	desc2D.Height    = m_height;
	desc2D.Usage     = D3D11_USAGE_DEFAULT;
	desc2D.MipLevels = 1;
	desc2D.SampleDesc.Count = 1;
	desc2D.SampleDesc.Quality = 0;
	


	//RTV desc
	D3D11_RENDER_TARGET_VIEW_DESC descRTV = {};
	descRTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	descRTV.Format        = DXGI_FORMAT_R32G32B32A32_FLOAT;

	//SRV desc
	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV = {};
	descSRV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MipLevels = 1;
	descSRV.Texture2D.MostDetailedMip = 0;

	
	

	{
		hr = m_device->CreateTexture2D(&desc2D, NULL, &norm2D);
		hr = m_device->CreateTexture2D(&desc2D, NULL, &diff2D);
		hr = m_device->CreateTexture2D(&desc2D, NULL, &spec2D);
		hr = m_device->CreateTexture2D(&desc2D, NULL, &pos2D);
	}

	{
		hr = m_device->CreateRenderTargetView(norm2D, &descRTV, &m_rt_NORMAL_RGB);
		hr = m_device->CreateRenderTargetView(diff2D, &descRTV, &m_rt_DIFFUSE_RGB);
		hr = m_device->CreateRenderTargetView(spec2D, &descRTV, &m_rt_SPECULAR_RGB_POWER_A);
		hr = m_device->CreateRenderTargetView(pos2D,  &descRTV, &m_rt_POSITION_RGB);
	}

	{
		hr = m_device->CreateShaderResourceView(norm2D, &descSRV, &m_sr_NORMAL_RGB);
		hr = m_device->CreateShaderResourceView(diff2D, &descSRV, &m_sr_DIFFUSE_RGB);
		hr = m_device->CreateShaderResourceView(spec2D, &descSRV, &m_sr_SPECULAR_RGB_POWER_A);
		hr = m_device->CreateShaderResourceView(pos2D,  &descSRV, &m_sr_POSITION_RGB);
	}
	norm2D->Release();
	diff2D->Release();
	spec2D->Release();
	pos2D ->Release();

	return hr;
}

//-----------------------------------------------
// CTOR
DeferredRenderer::DeferredRenderer
(	ID3D11Device* device, 
	ID3D11DeviceContext* deviceContext, 
	ID3D11DepthStencilView* depthStencilView, 
	UINT width,
	UINT height
)
  : m_width(width), 
	m_height(height),
	m_DepthStencil(depthStencilView),
	m_device(device),
	m_deviceContext(deviceContext){}
//------------------------------------------------

