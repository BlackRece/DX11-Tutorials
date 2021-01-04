#pragma once
#include <DirectXMath.h>
#include "Structures.h"
#include "OBJLoader.h"

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

	void LoadOBJ(char* filename, ID3D11Device* _pd3dDevice, bool invertTexCoords = true);

private:
	void CalcNormals(
		std::vector<Vertex> verts,
		std::vector<unsigned short> inds
	);

	void CleanUp();
};

