#pragma once
#include <directxmath.h>

#include "Vector3D.h"

using namespace DirectX;

class CameraC {
public:
	Vector3D _eye;		//cam pos
	Vector3D _at;		//cam dir
	Vector3D _up;		//cam up

	XMFLOAT4X4					_view;
	XMFLOAT4X4					_projection;

	float rotateSpeed;
	float translateSpeed;

	CameraC();

	/*
	XMFLOAT3 EyeToFloat3() {
		return XMFLOAT3{ Eye.m128_f32[0], Eye.m128_f32[1], Eye.m128_f32[2] };
	}
	*/

	void Update();
	void Rotate(float xAxis, float yAxis, float zAxis);
	void Rotate(Vector3D angles);
	void RotateX(float xAxis);
	void RotateY(float yAxis);
	void RotateZ(float zAxis);

	void Translate(float xAxis, float yAxis, float zAxis);
	void Translate(Vector3D position);

	void Scale(float xAxis, float yAxis, float zAxis);
	void Scale(Vector3D sizes);

	XMMATRIX GetView();
	XMMATRIX GetProjection();
	XMMATRIX GetViewProj();

	void SetProjection(float fovAngle, float ratio, float nearClip, float farClip);
};

