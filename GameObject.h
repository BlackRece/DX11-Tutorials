#pragma once
#include <DirectXMath.h>
#include "Structures.h"

using namespace DirectX;

class GameObject {
public:
	//transform
	XMFLOAT4X4					_matrix;				//world space
	Vector3D					_pos;					//translation
	Vector3D					_scale;					//scale
	Vector3D					_angle;					//rotation

	//model
	MeshArray					_mesh;
	ID3D11Buffer*				_vertexBuffer;			//VertexBuffer;
	ID3D11Buffer*				_indexBuffer;			//IndexBuffer;
	UINT						_vertexCount;			//VertexCount;
	UINT						_indexCount;			//IndexCount;

	UINT						_stride;				//vertex buffer stride
	UINT						_offset;				//vertex buffer offset

	//texture
	ID3D11ShaderResourceView*	_textureRV;

private:
	wchar_t* _wc;

public:
	GameObject();
	~GameObject();

	void CopyObject(ID3D11Device& device, GameObject& target);
	void LookTo(Vector3D camUp, Vector3D camRight);
	void LookTo(Vector3D target);

	// Model
	HRESULT CreateVertexBuffer(ID3D11Device& device);
	HRESULT CreateIndexBuffer(ID3D11Device& device);

	void ImportVertices(VertexTextures* src, int srcSize);
	void ImportIndices(WORD* src, int srcSize, bool calcNorms = true);

	// Texture
	void CreateTexture(ID3D11Device& device, string filepath);

	// Render
	void Draw(ID3D11DeviceContext* context, ID3D11Buffer* buffer, ConstantBufferLite& cbl);
	void Update(float deltaTime);

private:

	void CalcNormals(
		std::vector<Vertex> verts,
		std::vector<unsigned int> inds
	);

	void CleanUp();
	const wchar_t* GetWC(const string str);

};

