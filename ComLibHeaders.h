#pragma once
#include <cstdint>
#include <DirectXMath.h>
using DirectX::XMFLOAT4X4;
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
	EDITED_POINTLIGHT_COLOR     = 9
};
struct sPrimaryHeader
{
	MessageType type;
};

struct sNewMeshHeader
{
	uint32_t nameLength;
	uint32_t vertCount;
	XMFLOAT4X4 worldTransform;
};

struct sEditedMeshTransform
{
	uint32_t nameLength;
	XMFLOAT4X4 worldTransform;
};

struct sVector3
{
	float x, y, z;
};