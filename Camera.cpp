#include "Camera.h"



Camera::Camera(unsigned int w, unsigned int h, CAMERA_TYPE type) : vp_w(w), vp_h(h), cameraType(type)
{
	DefaultForward = XMVectorSet(0.0, 0.0, 1.0, 0.0);
	DefaultRight   = XMVectorSet(1.0, 0.0, 0.0, 0.0);

	//Camera information
	camPosition    = DirectX::XMVectorSet(0.0, 10, -2.5, 0.0);
	camTarget      = DirectX::XMVectorSet(0.0, 0.0, 0.0, 0.0);
	camUp          = DirectX::XMVectorSet(0.0, 1.0, 0.0, 0.0);

	//Set the View matrix
	camView        = DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp);

	//Set the Projection matrix
	if (cameraType == CAMERA_TYPE::PERSPECTIVE)
	{
		camProjection = DirectX::XMMatrixPerspectiveFovLH(0.4f*3.1415f, (float)w / h, 0.1f, 1000.0f);
	}
	else
	{
		DirectX::XMMatrixOrthographicLH(w, h, 0.1f, 1000.0f);
	}
}

void Camera::Init(unsigned int w, unsigned int h, CAMERA_TYPE type)
{
	vp_w = w;
	vp_h = h;
	cameraType = type;

	DefaultForward = XMVectorSet(0.0, 0.0, 1.0, 0.0);
	DefaultRight   = XMVectorSet(1.0, 0.0, 0.0, 0.0);

	//Camera information
	camPosition    = DirectX::XMVectorSet(0.0, 1.5, -4.0, 0.0);
	camTarget      = DirectX::XMVectorSet(0.0, 0.0, 1.0, 0.0);
	camUp          = DirectX::XMVectorSet(0.0, 1.0, 0.0, 0.0);

	//Set the View matrix
	camView        = DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp);

	//Set the Projection matrix
	if (cameraType == CAMERA_TYPE::PERSPECTIVE)
	{
		camProjection = DirectX::XMMatrixPerspectiveFovLH(0.4f*3.1415f, (float)w / h, 0.1f, 1000.0f);
	}
	else
	{
		camProjection = DirectX::XMMatrixOrthographicLH(1, 1, 0.1f, 1000.0f);
	}
}

void Camera::Update()
{
	//Update camview
	//camView = DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp);
}

void Camera::UpdateCameraForwardRight(XMMATRIX matrix)
{
	Forward = XMVector3Transform(DefaultForward, matrix);
	Right   = XMVector3Transform(DefaultRight,   matrix);
}

void Camera::SetView(XMMATRIX & matrix)
{
	camView = matrix;
}

void Camera::SetProjection(XMMATRIX & matrix)
{
	camProjection = matrix;
}

void Camera::SetCameraUp(XMVECTOR up)
{
	camUp = up;
}

XMVECTOR Camera::GetCameraUp()
{
	return camUp;
}

XMVECTOR Camera::GetCameraTarget()
{
	return camTarget;
}

XMVECTOR Camera::GetCameraPosition()
{
	return camPosition;
}

void Camera::SetCameraTarget(XMVECTOR target)
{
	camTarget = target;
}

void Camera::SetCameraPosition(XMVECTOR position)
{
	camPosition = position;
}

void Camera::MoveCamera(XMVECTOR translation)
{
	camPosition = DirectX::XMVectorAdd(camPosition, translation);
}

void Camera::MoveCameraForwardsBackwards(float amount)
{
	camPosition += Forward*amount;
}

void Camera::MoveCameraRightLeft(float amount)
{
	camPosition += Right*amount;
}

void Camera::SetWorldMatrix(XMMATRIX &in_world)
{
	World = in_world;
}

XMFLOAT4X4 Camera::GetWVPMatrix()
{
	XMFLOAT4X4 WVP;
	DirectX::XMStoreFloat4x4(&WVP, DirectX::XMMatrixTranspose(World*camView*camProjection));
	return WVP;
}

Camera::~Camera()
{
}
