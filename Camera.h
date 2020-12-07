#pragma once
#include <directxmath.h>
#include "Vector3D.h"

using namespace DirectX;

class Camera {
private:
	// Private attributes to store the camera position and view 
	// volume

	Vector3D _eye;
	Vector3D _at;
	Vector3D _up;

	float _farDepth;
	float _nearDepth;
	float _windowWidth;
	float _windowHeight;

	// camera movement speeds
	float _rotateSpeed;
	float _translateSpeed;

	// attributes to hold the view and projection matrices which 
	// will be passed to the shader

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

public:

	//Constructor and destructor for the camera
	Camera();
	Camera(
		Vector3D position, Vector3D at, Vector3D up,
		float windowWidth, float windowHeight,
		float nearDepth, float farDepth,
		float rotateSpeed = 1.0f, float translateSpeed = 1.0f);
	Camera(
		XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, 
		float windowWidth, float windowHeight, 
		float nearDepth, float farDepth);
	~Camera();

	// you will need a set of functions to set and return the 
	// position, lookat and up attributes
	// You will need functions to get the View, Projection and 
	// combined ViewProjection matrices
	Vector3D GetLookAt();
	Vector3D GetPos();
	XMMATRIX GetProjection();
	Vector3D GetUp();
	XMMATRIX GetView();
	XMMATRIX GetViewProj();

	/*
	void Rotate(float xAxis, float yAxis, float zAxis);
	void Rotate(Vector3D angles);
	void RotateX(float xAxis);
	void RotateY(float yAxis);
	void RotateZ(float zAxis);
	*/

	// A function to reshape the camera volume if the window is 
	// resized.
	void Reshape(
		float windowWidth, float windowHeight, 
		float nearDepth, float farDepth
	);

	void SetLookAt(Vector3D at);
	void SetPos(Vector3D pos);
	void SetProjection(
		float fovAngle, float ratio, 
		float nearDepth, float farDepth
	); 
	void SetView();
	void SetUp(Vector3D up);

	void Translate(float xAxis, float yAxis, float zAxis);
	void Translate(Vector3D position);

	// update function to make the current view and projection 
	// matrices
	void Update();
};


