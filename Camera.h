#pragma once
#include <vector>
#include <directxmath.h>
#include "Vector3D.h"

using namespace DirectX;

class Camera {
public:
	struct WayPoint {
		Vector3D eye;
		Vector3D to;
		Vector3D up;
	};
private:
	// Private attributes to store the camera position and view 

	// volume
	Vector3D _eye;
	Vector3D _at;
	Vector3D _to;
	Vector3D _up;
	Vector3D _offset;

	Vector3D _angle;

	std::vector<WayPoint> _points;
	int _pointIndex;

	bool _useLookTo;

	float _farDepth;
	float _nearDepth;
	int _windowWidth;
	int _windowHeight;

	// camera movement speeds
	float _rotateSpeed;
	float _translateSpeed;

	// attributes to hold the view and projection matrices which 
	// will be passed to the shader

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

public:
	bool _followPlayer;
	bool _lookAtTarget;
	bool _isUsingWayPoints;

	//Constructor and destructor for the camera
	Camera();
	Camera(
		Vector3D position, Vector3D at, Vector3D up,
		int windowWidth, int windowHeight,
		float nearDepth, float farDepth,
		float rotateSpeed = 0.0001f, float translateSpeed = 0.001f);
	Camera(
		XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, 
		float windowWidth, float windowHeight, 
		float nearDepth, float farDepth);
	~Camera();

	void Load();

	void AddWayPoint(WayPoint newPoint);
	void AddWayPoint(Vector3D newPoint);
	WayPoint GetWayPoint(int index);

	// you will need a set of functions to set and return the 
	// position, lookat and up attributes
	// You will need functions to get the View, Projection and 
	// combined ViewProjection matrices
	Vector3D GetAngle();
	Vector3D GetLookAt();
	Vector3D GetLookTo();
	Vector3D GetOffset();
	Vector3D GetPos();
	XMMATRIX GetProjection();
	Vector3D GetUp();
	void GetVectors(XMFLOAT4X4& target,
		Vector3D& translation, Vector3D& rotation, Vector3D& scale = Vector3D());
	XMFLOAT4X4 GetView4x4();
	XMMATRIX GetView();
	XMMATRIX GetViewProj();

	void MoveForward(float forward);
	void MoveSidewards(float sideward);
	void MoveTo(Vector3D point, float speed);

	void Rotate(float xAxis, float yAxis, float zAxis);
	void Rotate(Vector3D angles);
	void RotateX(float xAxis);
	void RotateY(float yAxis);
	void RotateZ(float zAxis);

	// A function to reshape the camera volume if the window is 
	// resized.
	void Reshape(
		int windowWidth, int windowHeight, 
		float nearDepth, float farDepth
	);

	void SetLookAt(Vector3D at);
	void SetLookTo(Vector3D to);
	void SetOffset(Vector3D offset);
	void SetPos(Vector3D pos);
	void SetProjection(
		float fovAngle, float ratio, 
		float nearDepth, float farDepth
	); 
	void SetSpeed(float rotate, float translate);
	void SetView();
	void SetView(XMFLOAT4X4 view);
	void SetView(XMMATRIX view);
	void SetUp(Vector3D up);

	void Translate(float xAxis, float yAxis, float zAxis);
	void Translate(Vector3D pos);

	// update function to make the current view and projection 
	// matrices
	void Update();
	void UseLookTo(bool state);
	bool UseLookTo();
	void UseWayPoints(bool state);
};


