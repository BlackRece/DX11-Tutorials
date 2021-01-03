#include "GameObject.h"
#include "DDSTextureLoader.h"

GameObject::GameObject() {
	_vertexBuffer = nullptr;
	_indexBuffer = nullptr;

	_vertexCount = 0;
	_indexCount = 0;

	_textureRV = nullptr;
}

GameObject::~GameObject() {
	CleanUp();
}

void GameObject::CopyObject(ID3D11Device& device, GameObject& target) {
	//GameObject tmp;

	target._mesh = this->_mesh;
	target._vertexCount = this->_vertexCount;
	target._indexCount = this->_indexCount;

	target.CalcNormals(target._mesh.Vertices, target._mesh.Indices);

	target.CreateVertexBuffer(device);
	target.CreateIndexBuffer(device);
}

HRESULT GameObject::CreateVertexBuffer(ID3D11Device& device) {
	HRESULT hr = HRESULT();
	if (_mesh.Vertices.size() < 1) {
		hr = E_NOT_SET;
		return hr;
	}

	_stride = sizeof(Vertex);
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

	hr = device.CreateBuffer(&bd, &InitData, &_vertexBuffer);

	return hr;
}

HRESULT GameObject::CreateIndexBuffer(ID3D11Device& device) {
	HRESULT hr = HRESULT();
	if (_mesh.Indices.size() < 1){
		hr = E_NOT_SET;
		return hr;
	}

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));

	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = _mesh.Indices.size() * sizeof(unsigned int);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	ZeroMemory(&iInitData, sizeof(iInitData));
	iInitData.pSysMem = &_mesh.Indices[0];
	hr = device.CreateBuffer(&ibd, &iInitData, &_indexBuffer);

	return hr;
}

void GameObject::ImportVertices(VertexTextures* src, int srcSize) {
	if (src == nullptr || srcSize < 1) return;

	_vertexCount = (srcSize / sizeof(VertexTextures));

	for (int i = 0; i < (int)_vertexCount; i++) 
		_mesh.Vertices.push_back(Vertex(src[i].Pos, src[i].TexC));
}

void GameObject::ImportIndices(WORD* src, int srcSize, bool calcNorms) {
	if (src == nullptr || srcSize < 1) return;
	
	_indexCount = srcSize / sizeof(WORD);

	for (unsigned int i = 0; i < (int)_indexCount; i++) {
		_mesh.Indices.push_back(src[i]);
	}

	if(calcNorms) CalcNormals(_mesh.Vertices, _mesh.Indices);
}

void GameObject::CreateTexture(ID3D11Device& device, string filepath) {
	CreateDDSTextureFromFile(&device, GetWC(filepath) , nullptr, &_textureRV);
}

void GameObject::Draw(ID3D11DeviceContext* context, ID3D11Buffer* buffer, ConstantBufferLite& cbl) {
	cbl.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&_matrix));				// set object matrix

	if(_textureRV != nullptr)
		context->PSSetShaderResources(0, 1, &_textureRV);						// set texture

	context->IASetVertexBuffers(0, 1, &_vertexBuffer, &_stride, &_offset);	// set vertex buffer
	context->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);		// set index buffer

	context->UpdateSubresource(buffer, 0, nullptr, &cbl, 0, 0);				// update device with new buffer data

	context->DrawIndexed(_indexCount, 0, 0);									// draw obj
}

void GameObject::Update(float deltaTime) {
	XMStoreFloat4x4(&_matrix,
		XMMatrixMultiply(XMMatrixIdentity(),
			XMMatrixScaling(_scale.x, _scale.y, _scale.z) *
			XMMatrixRotationRollPitchYaw(_angle.x, _angle.y, _angle.z) *
			XMMatrixTranslation(_pos.x,_pos.y,_pos.z)
		)
	);
}

void GameObject::LookTo(Vector3D camUp, Vector3D camRight) {
	Vector3D vertexPos =
		_pos +
		camRight * _scale.x +
		camUp * _scale.y;

	_pos = vertexPos;

}

void GameObject::LookTo(Vector3D target) {
	// Calculate the rotation that needs to be applied to the billboard model to face the current camera position using the arc tangent function.
	//angle = atan2(modelPosition.x - cameraPosition.x, modelPosition.z - cameraPosition.z) * (180.0 / D3DX_PI);
	float angle = _pos.AngleTo(target);

	_angle.y = angle;
}

/// <summary>
/// Vertex Normal Averaging
/// </summary>
/// <param name="verts">An array of vertices (verts). 
/// Each vertex has a position component (pos) and a normal component (normal).</param>
/// <param name="inds">An array of indices (inds).</param>
/// <returns></returns>
void GameObject::CalcNormals(
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

void GameObject::CleanUp() {
	if (_vertexBuffer != nullptr) _vertexBuffer->Release();
	if (_indexBuffer != nullptr)_indexBuffer->Release();

	if (_wc) { _wc = nullptr; delete _wc; }
}

const wchar_t* GameObject::GetWC(const string str) {
	const char* c = str.c_str();
	const size_t cSize = strlen(c) + 1;
	size_t wcSize = 0;
	_wc = new wchar_t[cSize];
	mbstowcs_s(&wcSize, _wc, cSize, c, cSize-1);

	return _wc;
}