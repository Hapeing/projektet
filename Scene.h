#pragma once



#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Model.h"

using std::vector;
using std::string;
using std::unordered_map;

struct PointLight
{
	float x, y, z, w;
	float r, g, b, a;
};

class Scene
{
private:
	ID3D11Device*                       m_device               = nullptr;
	ID3D11DeviceContext*                m_deviceContext        = nullptr;
	ID3D11Buffer*                       m_materialBuffer       = nullptr;

	//Models in this scene
	vector<Model*>                      m_models;

	//Lights in this scene
	vector<PointLight>                  m_lights;

	std::unordered_map<string, Model*>* m_modelMap;
	
	bool                                m_useDeferredRendering = false;
public:
	
	bool LoadScene(string file);
	void RenderScene();
	Scene(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Buffer* buffer, std::unordered_map<string, Model*>& map);
	~Scene();
};

