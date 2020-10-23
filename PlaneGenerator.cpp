#include "PlaneGenerator.h"

void PlaneGenerator::CreateGrid(float width, float depth,
	unsigned int row, unsigned int col, MeshData& meshData) {
	
	_row = row;
	_col = col;

	_vertexCount = row * col;
	_faceCount = (row - 1) * (col - 1) * 2;

	//
	// Create the vertices.
	//
	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;
	float dx = width / (col - 1);
	float dz = depth / (row - 1);
	float du = 1.0f / (col - 1);
	float dv = 1.0f / (row - 1);

	meshData.Vertices.resize(_vertexCount);

	for (unsigned int i = 0; i < row; ++i) {
		float z = halfDepth - i * dz;
		for (unsigned int j = 0; j < col; ++j) {
			float x = -halfWidth + j * dx;
			meshData.Vertices[i * col + j].Position = Vector3D(x, 0.0f, z);

			// Ignore for now, used for lighting.
			meshData.Vertices[i * col + j].Normal = Vector3D(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * col + j].TangentU = Vector3D(1.0f, 0.0f, 0.0f);

			// Ignore for now, used for texturing.
			meshData.Vertices[i * col + j].TexC.x = j * du;
			meshData.Vertices[i * col + j].TexC.y = i * dv;
		}
	}
}

void PlaneGenerator::CreateIndices(MeshData& meshData) {
	//
	// Create the indices.
	//
	meshData.Indices.resize(_faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	unsigned int k = 0;
	for (unsigned int i = 0; i < _row - 1; ++i) {
		for (unsigned int j = 0; j < _col - 1; ++j) {
			meshData.Indices[k] = i * _col + j;
			meshData.Indices[k + 1] = i * _col + j + 1;
			meshData.Indices[k + 2] = (i + 1) * _col + j;
			meshData.Indices[k + 3] = (i + 1) * _col + j;
			meshData.Indices[k + 4] = i * _col + j + 1;
			meshData.Indices[k + 5] = (i + 1) * _col + j + 1;
			k += 6; // next quad
		}
	}

	_indexCount = meshData.Indices.size();
}

