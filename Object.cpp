#include "Object.h"

Object::Object() :
	_pVertexBuffer(nullptr),
	_pIndexBuffer(nullptr),
	_pConstantBuffer(nullptr),
	_vertices(nullptr),
	_indices(nullptr),
	_pIndexCount(0), _pVertexCount(0) 
{
	XMStoreFloat4x4(&_obj, XMMatrixIdentity());
}

Object::~Object() {
	if (_pConstantBuffer) _pConstantBuffer->Release();
	if (_pVertexBuffer) _pVertexBuffer->Release();
	if (_pIndexBuffer) _pIndexBuffer->Release();

	_vertices = nullptr;
	delete(_vertices);
	_indices = nullptr;
	delete(_indices);
}

HRESULT Object::CreateVertexBuffer(ID3D11Device* device, SimpleVertex vertices[], UINT count) {
	HRESULT hr = HRESULT();{};
	if (vertices == nullptr || count < 1) {
		hr = E_INVALIDARG;
		return hr;
	}

	_pVertexCount = count;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = _pVertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = device->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

	return hr;
}

HRESULT Object::CreateIndexBuffer(ID3D11Device* device, WORD indices[], UINT size) {
	HRESULT hr = HRESULT();
	if (indices == nullptr || size < 1)
		hr = E_INVALIDARG;
	return hr;

	_pIndexCount = size / sizeof(WORD);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = device->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

	return hr;
}