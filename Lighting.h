#pragma once
#include "Structures.h"

class Lighting {
public:
	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	Lighting();
	Lighting(XMFLOAT3 lightDir, XMFLOAT4 difMat, XMFLOAT4 difLight);
	
};

