#include "PlaneGenerator.h"

void PlaneGenerator::CreateGrid(float width, float depth,
	unsigned int row, unsigned int col, MeshArray& meshData) {
	
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
			//meshData.Vertices[i * col + j].Position = Vector3D(x, z, 0.0f);
			meshData.Vertices[i * col + j].Position = Vector3D(x, 0.0f, z);

			//used for lighting.
			meshData.Vertices[i * col + j].Normal = Vector3D(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * col + j].TangentU = Vector3D(1.0f, 0.0f, 0.0f);

			//used for texturing.
			meshData.Vertices[i * col + j].TexC.x = j * du;
			meshData.Vertices[i * col + j].TexC.y = i * dv;
		}
	}
}

void PlaneGenerator::CreateVerticalGrid(float width, float height, float depth,
	unsigned int row, unsigned int col, MeshArray& meshData) {

	_row = row;
	_col = col;

	_vertexCount = row * col;
	_faceCount = (row - 1) * (col - 1) * 2;

	//
	// Create the vertices.
	//
	float halfWidth = 0.5f * width;
	float halfheight = 0.5f * height;
	//float halfDepth = 0.5f * depth;
	float dx = width / (col - 1);
	float dy = height / (row - 1);
	//float dz = depth;
	float du = 1.0f / (col - 1);
	float dv = 1.0f / (row - 1);

	meshData.Vertices.resize(_vertexCount);

	for (unsigned int i = 0; i < row; ++i) {
		float y = halfheight - i * dy;
		for (unsigned int j = 0; j < col; ++j) {
			float x = -halfWidth + j * dx;
			meshData.Vertices[i * col + j].Position = Vector3D(x, y, 0.0f);

			//used for lighting.
			meshData.Vertices[i * col + j].Normal = Vector3D(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * col + j].TangentU = Vector3D(1.0f, 0.0f, 0.0f);

			//used for texturing.
			meshData.Vertices[i * col + j].TexC.x = j * du;
			meshData.Vertices[i * col + j].TexC.y = i * dv;
		}
	}
}

void PlaneGenerator::CreateIndices(MeshArray& meshData) {
	//
	// Create the indices.
	//
	meshData.Indices.resize(_faceCount * 3); // 3 indices per face

	//Dyslexic Markers
	unsigned int TL, TR, BL, BR;

	// Iterate over each quad and compute indices.
	unsigned int k = 0;
	for (unsigned int i = 0; i < _row-1 ; ++i) {
		for (unsigned int j = 0; j < _col-1 ; ++j) {
			TL = i * _col + j;
			TR = i * _col + j + 1;
			BL = (i + 1) * _col + j;
			BR = (i + 1) * _col + j + 1;

			meshData.Indices[k + 0] = TL;
			meshData.Indices[k + 1] = TR;
			meshData.Indices[k + 2] = BL;

			meshData.Indices[k + 3] = BL;
			meshData.Indices[k + 4] = TR;
			meshData.Indices[k + 5] = BR;

			k += 6; // next quad
		}
	}

	_indexCount = meshData.Indices.size();

	// DEBUG
	std::cout << "Index count: " << _indexCount << std::endl;
}

/// <summary>
/// Vertex Normal Averaging
/// </summary>
/// <param name="verts">An array of vertices (verts). 
/// Each vertex has a position component (pos) and a normal component (normal).</param>
/// <param name="inds">An array of indices (inds).</param>
/// <returns></returns>
void PlaneGenerator::CalcNormals(
	std::vector<Vertex> verts,
	std::vector<unsigned int> inds
) {
	int faces = inds.size() / 3;

	for (int i = 0; i < faces; i++) {
		//indices of the ith triangle
		unsigned int i0 = inds[i * 3 + 0];
		unsigned int i1 = inds[i * 3 + 1];
		unsigned int i2 = inds[i * 3 + 2];

		//vertices of ith triangle
		Vector3D v0 = verts[i0].Position;
		Vector3D v1 = verts[i1].Position;
		Vector3D v2 = verts[i2].Position;

		//compute face normal
		Vector3D faceNormal = 
			faceNormal.ComputeNormal(v0, v1, v2);

		// This triangle shares the following three vertices,
		// so add this face normal into the average of these
		// vertex normals.
		verts[i0].Normal += faceNormal;
		verts[i1].Normal += faceNormal;
		verts[i2].Normal += faceNormal;
	}

	// For each vertex v, we have summed the face normals of all
	// the triangles that share v, so now we just need to normalize.
	for (unsigned int i = 0; i < verts.size(); ++i)
		verts[i].Normal.normalization();
}

void PlaneGenerator::TextureCube(std::vector<unsigned int>inds) {
	for (unsigned int i = 0; i < inds.size(); i++){}
}