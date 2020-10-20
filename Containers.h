#pragma once
#include <directxmath.h>

using namespace DirectX;

struct SimpleVertex {
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ConstantBuffer {
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};
