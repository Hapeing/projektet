#include "Scene.h"
#define DEFAULT_DIRECTORY std::string("C:\\Users\\Finoli\\Desktop\\")
#define WDEFAULT_DIRECTORY std::wstring(L"C:\\Users\\Finoli\\Desktop\\")

bool Scene::LoadScene(string file)
{
	std::ifstream sceneFile(file);

	assert(sceneFile.is_open());

	string input;
	while (!sceneFile.eof())
	{
		sceneFile >> input;
		
		if (input == "model")
		{
			string name;
			sceneFile >> name;

			//Check if model already exists
			unordered_map<string, Model*>::const_iterator got = m_modelMap->find(name);
			if (got == m_modelMap->end())
			{
				float x, y, z;
				sceneFile >> x >> y >> z;
				//Load
				Model* model = new Model(m_device, m_deviceContext, m_materialBuffer);
				model->LoadOBJ(name.c_str());
				m_modelMap->insert({ name, model });

				model = new Model(*model);
				model->IncreaseTranslation(XMVectorSet(x, y, z, 0.0));

				m_models.push_back(model);
			}
			else
			{
				float x, y, z;
				sceneFile >> x >> y >> z;
				Model* model = new Model(*got->second);
				model->IncreaseTranslation(XMVectorSet(x, y, z, 0.0));
				m_models.push_back(model);
			}
		}
		else if (input == "light")
		{
			float x, y, z, r, g, b, a;
			sceneFile >> x >> y >> z >> r >> g >> b >> a;
			PointLight light = { x, y, z, 0.0, r, g, b, a };
			m_lights.push_back(light);
		}
		else if (input == "deferred")
		{
			bool use;
			sceneFile >> use;
			m_useDeferredRendering = use;
		}
	}

	sceneFile.close();
	return false;
}


Scene::Scene(ID3D11Device * device, ID3D11DeviceContext * deviceContext, ID3D11Buffer * buffer, std::unordered_map<string, Model*>& map)
	: m_device(device), m_deviceContext(deviceContext), m_materialBuffer(buffer), m_modelMap(&map)
{

}

Scene::~Scene()
{
}
