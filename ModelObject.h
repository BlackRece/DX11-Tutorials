#pragma once
#include <DirectXMath.h>
#include "Structures.h"

using namespace DirectX;

class ModelObject {
public:

	//model
	MeshArray					_mesh;
	ID3D11Buffer*				_vertexBuffer;			//VertexBuffer;
	ID3D11Buffer*				_indexBuffer;			//IndexBuffer;
	UINT						_vertexCount;			//VertexCount;
	UINT						_indexCount;			//IndexCount;

	UINT						_stride;				//vertex buffer stride
	UINT						_offset;				//vertex buffer offset

public:
	ModelObject();
	~ModelObject();

	// Model
	HRESULT CreateVertexBuffer(ID3D11Device& device);
	HRESULT CreateIndexBuffer(ID3D11Device& device);

	void ImportVertices(VertexTextures* src, int srcSize);
	void ImportIndices(WORD* src, int srcSize, bool calcNorms = true);


private:
	void CalcNormals(
		std::vector<Vertex> verts,
		std::vector<unsigned int> inds
	);

	void CleanUp();
};

