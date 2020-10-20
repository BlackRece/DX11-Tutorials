#pragma once

#include <Windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#include "Containers.h"

//using namespace DirectX;

class Object {
public:
	ID3D11Buffer*	_pVertexBuffer;
	ID3D11Buffer*	_pIndexBuffer;
	ID3D11Buffer*	_pConstantBuffer;

	XMFLOAT4X4		_obj;

	SimpleVertex*	_vertices;
	WORD*			_indices;

	UINT			_pIndexCount;
	UINT			_pVertexCount;

public:
	Object();
	~Object();
	void SetVertexCount(UINT vertexCount) { _pVertexCount = vertexCount; }
	void SetIndexCount(UINT indexCount) { _pIndexCount = indexCount; }
	HRESULT CreateVertexBuffer(ID3D11Device* device, SimpleVertex vertices[], UINT count);
	HRESULT CreateIndexBuffer(ID3D11Device* device, WORD indices[], UINT size);
};

