#include "Lighting.h"


Lighting::Lighting() {
	// Light direction from surface (XYZ)
	lightDirection = XMFLOAT3(0.25f, 0.5f, -1.0f);
	// Diffuse material properties (RGBA)
	diffuseMaterial = XMFLOAT4(0.5f, 0.5f, 0.7f, 1.0f);
	// Diffuse light colour (RGBA)
	diffuseLight = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);

}

Lighting::Lighting(XMFLOAT3 lightDir, XMFLOAT4 difMat, XMFLOAT4 difLight) {
	// Light direction from surface (XYZ)
	lightDirection = lightDir;
	// Diffuse material properties (RGBA)
	diffuseMaterial = difMat;
	// Diffuse light colour (RGBA)
	diffuseLight = difLight;
}
