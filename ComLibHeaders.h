#pragma once
#include <cstdint>
#include <DirectXMath.h>
using DirectX::XMFLOAT4X4;

enum CAMERA_TYPE
{
	PERSPECTIVE,
	ORTHOGRAPHIC
};

enum MessageType
{
	NEW_MESH                    = 0,
	EDITED_MESH_TOPOLOGY        = 1,
	EDITED_MESH_TRANSFORM       = 2,
	EDITED_MESH_TEXTURE         = 3,

	NEW_CAMERA                  = 4,
	EDITED_CAMERA_TRANSFORM     = 5,
	ACTIVE_CAMERA_CHANGED       = 6,

	NEW_POINTLIGHT              = 7,
	EDITED_POINTLIGHT_TRANSFORM = 8,
	EDITED_POINTLIGHT_COLOR     = 9,

	MESH_REMOVED				= 10
};
struct sPrimaryHeader
{
	MessageType type;
};

struct sNewMeshHeader
{
	XMFLOAT4X4 worldTransform;
	uint32_t nameLength;
	uint32_t vertCount;
};

struct sMeshRemoved
{
	uint32_t nameLength;
};

struct sEditedCameraTransform
{
	CAMERA_TYPE type;

	float posX, posY, posZ;
	float tarX, tarY, tarZ;
	float upX, upY, upZ;

	float fovY, nearZ, farZ;
};

struct sEditedTopology
{
	uint32_t nameLength;
	uint32_t vertCount;
};

struct sEditedMeshTransform
{
	XMFLOAT4X4 worldTransform;
	uint32_t nameLength;
};

struct sVector3
{
	float x, y, z;
};