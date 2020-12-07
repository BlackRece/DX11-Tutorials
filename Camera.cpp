#include "Camera.h"

Camera::Camera() {}
Camera::Camera(Vector3D position, Vector3D at, Vector3D up,
    float windowWidth, float windowHeight,
    float nearDepth, float farDepth,
    float rotateSpeed, float translateSpeed) :
    _rotateSpeed(rotateSpeed), _translateSpeed(translateSpeed) {

    SetPos(position);
    SetLookAt(at);
    SetUp(up);
    
    SetView();
    
    Reshape(windowWidth, windowHeight, nearDepth, farDepth);
}

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up,
    float windowWidth, float windowHeight,
    float nearDepth, float farDepth) {

    Camera(
        Vector3D(position.x, position.y, position.z),
        Vector3D(at.x,at.y,at.z),
        Vector3D(up.x,up.y,up.z),
        windowWidth, windowHeight,
        nearDepth, farDepth);
}

Camera::~Camera() {}

Vector3D Camera::GetLookAt() {
    return _at;
}

Vector3D Camera::GetPos() {
    return _eye;
}

XMMATRIX Camera::GetProjection() {
    return XMLoadFloat4x4(&_projection);
}

Vector3D Camera::GetUp() {
    return _up;
}

XMMATRIX Camera::GetView() {
    return XMLoadFloat4x4(&_view);
}

XMMATRIX Camera::GetViewProj() {
    return XMMatrixMultiply(GetView(), GetProjection());
}

void Camera::Reshape(
    float windowWidth, float windowHeight,
    float nearDepth, float farDepth) {

    _windowWidth = windowWidth;
    _windowHeight = windowHeight;
    
    _nearDepth = nearDepth;
    _farDepth = farDepth;

    SetProjection(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth);
}

/*
void Rotate(float xAxis, float yAxis, float zAxis) {
    Rotate(Vector3D(xAxis, yAxis, zAxis));
}
void Rotate(Vector3D angles) {
    RotateX(angles.x);
    RotateY(angles.y);
    RotateZ(angles.z);
}
void RotateX(float xAxis) {
    //rotate eye and at in opposite dorections
}
void RotateY(float yAxis);
void RotateZ(float zAxis);
*/

void Camera::SetLookAt(Vector3D at) {
    _at = at;
}

void Camera::SetPos(Vector3D pos) {
    _eye = pos;
}

void Camera::SetProjection(float fovAngle, float ratio, float nearDepth, float farDepth) {
    XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(fovAngle, ratio, nearDepth, farDepth));
}

void Camera::SetView() {
    XMVECTOR eye = XMVectorSet(_eye.x, _eye.y, _eye.z, 0.0f);
    XMVECTOR at = XMVectorSet(_at.x, _at.y, _at.z, 0.0f);
    XMVECTOR up = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);

    XMStoreFloat4x4(&_view, XMMatrixLookAtLH(eye, at, up));
}

void Camera::SetUp(Vector3D up) {
    _up = up;
}

void Camera::Translate(float xAxis, float yAxis, float zAxis) {
    Translate(Vector3D(xAxis, yAxis, zAxis));
}

void Camera::Translate(Vector3D position) {
    Vector3D diff = _eye - position;
    _eye += diff;
    _at += diff;

}

void Camera::Update() {
    SetView();
    Reshape(_windowWidth, _windowHeight, _nearDepth, _farDepth);
}