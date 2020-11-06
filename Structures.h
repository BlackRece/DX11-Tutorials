#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <vector>
#include "Vector3D.h"

using namespace DirectX;

struct SimpleVertex {
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ConstantBuffer {
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	float gTime;

	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightVecW;
};

struct VertexNormals {
	XMFLOAT3 Pos;
	XMFLOAT4 Normal;

};

struct Vertex {
	Vector3D Position;
	Vector3D Normal;
	Vector3D TangentU;		// angle?
	Vector3D TexC;			// uv's - ignore z

	Vertex() {}
	Vertex(
		const Vector3D& p,
		const Vector3D& n,
		const Vector3D& t,
		const Vector3D& uv) :
		Position(p), Normal(n),
		TangentU(t), TexC(uv) {}
	Vertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v) :
		Position(px, py, pz), Normal(nx, ny, nz),
		TangentU(tx, ty, tz), TexC(u, v) {}
	Vertex(XMFLOAT3 pos) :
		Position(pos.x, pos.y, pos.z) {}
};

struct MeshData {
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
};