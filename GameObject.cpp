#include "GameObject.h"
#include "DDSTextureLoader.h"

GameObject::GameObject() {
	_textureRV = nullptr;
	_model = new ModelObject();
}

GameObject::~GameObject() {
	CleanUp();
}

void GameObject::CopyObject(ID3D11Device& device, GameObject& target) {
	// only need to copy a reference to the ModelObject
	/*
	target._mesh = this->_mesh;
	target._vertexCount = this->_vertexCount;
	target._indexCount = this->_indexCount;

	target.CalcNormals(target._mesh.Vertices, target._mesh.Indices);

	target.CreateVertexBuffer(device);
	target.CreateIndexBuffer(device);
	*/
}

void GameObject::CreateTexture(ID3D11Device& device, string filepath) {
	HRESULT hr = CreateDDSTextureFromFile(&device, GetWC(filepath) , nullptr, &_textureRV);
	if (FAILED(hr))
		return;
}

void GameObject::Draw(ID3D11DeviceContext* context, ID3D11Buffer* buffer, ConstantBufferLite& cbl) {
	cbl.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&_matrix));				// set object matrix

	if(_textureRV != nullptr)
		context->PSSetShaderResources(0, 1, &_textureRV);						// set texture

	context->IASetVertexBuffers
	(0, 1, &_model->_vertexBuffer, &_model->_stride, &_model->_offset);	// set vertex buffer
	context->IASetIndexBuffer(_model->_indexBuffer, DXGI_FORMAT_R32_UINT, 0);		// set index buffer

	context->UpdateSubresource(buffer, 0, nullptr, &cbl, 0, 0);				// update device with new buffer data

	context->DrawIndexed(_model->_indexCount, 0, 0);									// draw obj
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


void GameObject::CleanUp() {
	if (_wc) { _wc = nullptr; delete _wc; }
	if (_model) delete _model;
}

const wchar_t* GameObject::GetWC(const string str) {
	const char* c = str.c_str();
	const size_t cSize = strlen(c) + 1;
	size_t wcSize = 0;
	_wc = new wchar_t[cSize];
	mbstowcs_s(&wcSize, _wc, cSize, c, cSize-1);

	return _wc;
}