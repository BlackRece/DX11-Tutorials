#pragma once
#include "Structures.h"

class Lighting {
public:
	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	XMFLOAT4 ambientMaterial;
	XMFLOAT4 ambientLight;

	XMFLOAT4 SpecularMaterial;
	XMFLOAT4 SpecularLight;
	float SpecularPower;
	XMFLOAT3 EyePosW;			// Camera position in world space

	Lighting();
	Lighting(XMFLOAT3 lightDir, XMFLOAT4 difMat, XMFLOAT4 difLight);
	
};

