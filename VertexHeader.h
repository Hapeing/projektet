#pragma once

struct Vertex_uv2
{
	float u, v;
};

struct Vertex_pos3
{
	float x, y, z;
};

struct Vertex_pos3uv2
{
	float posX, posY, posZ;
	float u, v;
};

struct Vertex_pos3col3
{
	float posX, posY, posZ;
	float r, g, b;
};

struct Vertex_pos3nor3uv2
{
	float posX, posY, posZ;
	float norX, norY, norZ;
	float u, v;
};

