#pragma once
#include "Shader.h"

class SSAO
{
private:
	UINT m_width;
	UINT m_height;

	Shader m_CS;

public:
	void DoSSAO();
	void BuildSamples();
	void CreateResources();
	SSAO();
	~SSAO();
};

