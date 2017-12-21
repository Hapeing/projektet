#pragma once
#include <DirectXMath.h>
#include "ComLibHeaders.h"

using namespace DirectX;

class Camera
{
private:
	
	CAMERA_TYPE cameraType;

	unsigned int vp_w, vp_h = 0;

	XMVECTOR DefaultForward;
	XMVECTOR DefaultRight;

	XMVECTOR Forward;
	XMVECTOR Right;

	XMMATRIX	World;
	XMMATRIX	camView;
	XMMATRIX	camProjection;
	XMVECTOR	camPosition;
	XMVECTOR	camTarget;
	XMVECTOR	camUp;
public:



	Camera(unsigned int w, unsigned int h, CAMERA_TYPE type);
	Camera() {};
	void Init(unsigned int w, unsigned int h, CAMERA_TYPE type);
	
	void Update();

	void UpdateCameraForwardRight(XMMATRIX matrix);

	void SetView(XMMATRIX& matrix);
	void SetProjection(XMMATRIX& matrix);

	void SetCameraUp(XMVECTOR up);
	void SetCameraTarget(XMVECTOR target);
	void SetCameraPosition(XMVECTOR position);

	void MoveCamera(XMVECTOR translation);
	void MoveCameraForwardsBackwards(float amount);
	void MoveCameraRightLeft(float amount);

	XMMATRIX GetWorldMatrix() { return World; }

	XMVECTOR GetCameraUp();
	XMVECTOR GetCameraTarget();
	XMVECTOR GetCameraPosition();



	void SetWorldMatrix(XMMATRIX &in_world);
	XMFLOAT4X4 GetWVPMatrix();
	~Camera();
};

