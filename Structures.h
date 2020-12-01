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

	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightVecW;

	float gTime;

	XMFLOAT4 AmbientMaterial;
	XMFLOAT4 AmbientLight;

	XMFLOAT4 SpecularMaterial;
	XMFLOAT4 SpecularLight;
	float SpecularPower;
	XMFLOAT3 EyePosW;			// Camera position in world space
};

struct ConstantBufferLite {
	XMMATRIX mWorld;
	XMMATRIX mViewProj;

	XMFLOAT4 mDiffuse;
	float	 mDiffuseAlpha;
	XMFLOAT3 mLightVecW;

	XMFLOAT4 mAmbient;

	XMFLOAT4 mSpecular;
	float	 SpecularPower;

	XMFLOAT3 EyePosW;			// Camera position in world space
};

// TODO: change to class
struct Camera {
	XMVECTOR Eye;		//cam pos
	XMVECTOR At;		//cam dir
	XMVECTOR Up;		//cam up

	XMFLOAT3 EyeToFloat3() {
		return XMFLOAT3{ Eye.m128_f32[0], Eye.m128_f32[1], Eye.m128_f32[2] };
	}
};

struct VertexNormals {
	XMFLOAT3 Pos;
	XMFLOAT4 Normal;
};

struct VertexTextures {
	XMFLOAT3 Pos;
	XMFLOAT3 Norm;
	XMFLOAT2 TexC;
};

struct Vertex {
	Vector3D Position;
	Vector3D Normal;
	Vector3D TexC;			// uv's - ignore z
	Vector3D TangentU;		// angle?

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
	Vertex(XMFLOAT3 pos, XMFLOAT2 uv) :
		Position(pos.x, pos.y, pos.z),
		TexC(uv.x, uv.y) {}
};

struct MeshData {
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
};