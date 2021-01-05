#pragma once
#include <DirectXMath.h>
#include "Structures.h"
#include "OBJLoader.h"
#include "PlaneGenerator.h"

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
	// [ B3 ]
	HRESULT CreateVertexBuffer(ID3D11Device& device);
	// [ B3 ]
	HRESULT CreateIndexBuffer(ID3D11Device& device);

	// [ B2, B3 ]
	HRESULT CreatePlane(ID3D11Device& device, Vector3D dims,
		int rows, int cols, bool isHorizontal = true);

	// [ B3 ]
	void ImportVertices(VertexTextures* src, int srcSize);
	// [ B3 ]
	void ImportIndices(WORD* src, int srcSize, bool calcNorms = true);

	// [ B2 ]
	void LoadOBJ(char* filename, ID3D11Device* _pd3dDevice, bool invertTexCoords = true);

private:
	void CalcNormals(
		std::vector<Vertex> verts,
		std::vector<unsigned short> inds
	);

	void CleanUp();
};

