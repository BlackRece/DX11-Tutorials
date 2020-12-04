#pragma once
#include "Structures.h"

class PlaneGenerator {
public:
	Vertex _plane;
	MeshArray _meshData;

	unsigned int _faceCount;
	unsigned int _vertexCount, _indexCount;
	unsigned int _row, _col;

	Vector3D position;

	PlaneGenerator() :
		_faceCount(0), _row(0), _col(0),
		_vertexCount(0), _indexCount(0) {}

	void CreateGrid(float width, float depth, 
		unsigned int row, unsigned int col, 
		MeshArray& meshData);
	void CreateIndices(MeshArray& meshData);

	static void CalcNormals(
		std::vector<Vertex> verts,
		std::vector<unsigned int> inds
	);

	void TextureCube(std::vector<unsigned int> inds);

	float GetHeight(float x, float z) const
		{ return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z)); }
};

