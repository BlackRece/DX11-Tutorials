#pragma once
#include "Vector3D.h";
#include <vector>

class PlaneGenerator {
public:
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
	};

	struct MeshData {
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
	};

	Vertex _plane;
	MeshData _meshData;

	unsigned int _faceCount;
	unsigned int _vertexCount, _indexCount;
	unsigned int _row, _col;

	Vector3D position;

	PlaneGenerator() :
		_faceCount(0), _row(0), _col(0),
		_vertexCount(0), _indexCount(0) {}

	void CreateGrid(float width, float depth, 
		unsigned int row, unsigned int col, 
		MeshData& meshData);
	void CreateIndices(MeshData& meshData);

	float GetHeight(float x, float z) const
		{ return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z)); }
};

