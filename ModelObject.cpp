#include "ModelObject.h"

ModelObject::ModelObject() {
	_vertexBuffer = nullptr;
	_indexBuffer = nullptr;

	_vertexCount = 0;
	_indexCount = 0;

	_mesh = MeshArray();
	_stride = 0;
	_offset = 0;
}

ModelObject::~ModelObject() {
	CleanUp();
}

// [ B3 ]
HRESULT ModelObject::CreateVertexBuffer(ID3D11Device& device) {
	if (_mesh.Vertices.size() < 1)
		return E_NOT_SET;
	else
		_vertexCount = _mesh.Vertices.size();

	_stride = sizeof(_mesh.Vertices[0]);
	_offset = 0;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = _mesh.Vertices.size() * _stride;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &_mesh.Vertices[0];

	return device.CreateBuffer(&bd, &InitData, &_vertexBuffer);
}

// [ B3 ]
HRESULT ModelObject::CreateIndexBuffer(ID3D11Device& device) {
	if (_mesh.Indices.size() < 1)
		return E_NOT_SET;
	else
		_indexCount = _mesh.Indices.size();

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));

	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = _mesh.Indices.size() * sizeof(_mesh.Indices[0]);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	ZeroMemory(&iInitData, sizeof(iInitData));
	iInitData.pSysMem = &_mesh.Indices[0];
	return device.CreateBuffer(&ibd, &iInitData, &_indexBuffer);
}

// [ B2, B3 ]
HRESULT ModelObject::CreatePlane(ID3D11Device& device, Vector3D dims,
	int rows, int cols, bool isHorizontal) {
	HRESULT hr = S_OK;
	
	PlaneGenerator plane = PlaneGenerator();
	plane.CreateVertices(dims.x, dims.y, dims.z, rows, cols, _mesh, isHorizontal);
	plane.CreateIndices(_mesh);

	_vertexCount = plane._vertexCount;
	_indexCount = plane._indexCount;

	hr = CreateVertexBuffer(device);
	if (FAILED(hr))
		return hr;

	hr = CreateIndexBuffer(device);
	if (FAILED(hr))
		return hr;

	return hr;
}

// [ B3 ]
void ModelObject::ImportVertices(VertexTextures* src, int srcSize) {
	if (src == nullptr || srcSize < 1) return;

	_vertexCount = (srcSize / sizeof(VertexTextures));

	for (int i = 0; i < (int)_vertexCount; i++)
		_mesh.Vertices.push_back(Vertex(src[i].Pos, src[i].TexC));
}

// [ B3 ]
void ModelObject::ImportIndices(WORD* src, int srcSize, bool calcNorms) {
	if (src == nullptr || srcSize < 1) return;

	_indexCount = srcSize / sizeof(WORD);

	for (unsigned int i = 0; i < (int)_indexCount; i++)
		_mesh.Indices.push_back(src[i]);

	if (calcNorms) CalcNormals(_mesh.Vertices, _mesh.Indices);
}

// [ B2 ]
void ModelObject::LoadOBJ(char* filename, ID3D11Device* _pd3dDevice, bool invertTexCoords) {
	MeshData obj = OBJLoader::Load(filename, _pd3dDevice, invertTexCoords);
	_vertexBuffer = obj.VertexBuffer;			//VertexBuffer;
	_indexBuffer = obj.IndexBuffer;				//IndexBuffer;
	_indexCount = obj.IndexCount;				//IndexCount;

	_stride = obj.VBStride;						//vertex buffer stride
	_offset = obj.VBOffset;						//vertex buffer offset
}

/// <summary>
/// Vertex Normal Averaging
/// </summary>
/// <param name="verts">An array of vertices (verts). 
/// Each vertex has a position component (pos) and a normal component (normal).</param>
/// <param name="inds">An array of indices (inds).</param>
/// <returns></returns>
void ModelObject::CalcNormals(std::vector<Vertex> verts, std::vector<unsigned short> inds) {
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

void ModelObject::CleanUp() {
	if (_vertexBuffer != nullptr) _vertexBuffer->Release();
	if (_indexBuffer != nullptr)_indexBuffer->Release();
}
