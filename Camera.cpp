#include "Camera.h"

Camera::Camera() {
    _points.clear();
    _pointIndex = 0;
}
Camera::Camera(Vector3D position, Vector3D at, Vector3D up,
    float windowWidth, float windowHeight,
    float nearDepth, float farDepth,
    float rotateSpeed, float translateSpeed) : _useLookTo(false), 
    _rotateSpeed(rotateSpeed), _translateSpeed(translateSpeed) {

    SetPos(position);
    SetLookAt(at);
    SetUp(up);
    
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

void Camera::AddWayPoint(WayPoint newPoint) {
    _points.push_back(newPoint);
}

Vector3D Camera::GetLookAt() {
    return _at;
}

Vector3D Camera::GetLookTo() {
    return _to;
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

Camera::WayPoint Camera::GetWayPoint(int index) {
    return _points[index];
}

void Camera::MoveForward(float forward) {
    float step = forward * _translateSpeed;
    if (!_useLookTo) {
        _eye.z += step;
        SetView();
        
    } else {
        Translate(_to * (forward * _translateSpeed));
    }
    
}

void Camera::MoveSidewards(float sideward) {
    if (!_useLookTo) {
        RotateY(sideward * _rotateSpeed);
    } else {
        Translate(sideward * _rotateSpeed, 0.0f, 0.0f);
    }
}

void Camera::Reshape(
    float windowWidth, float windowHeight,
    float nearDepth, float farDepth) {

    _windowWidth = windowWidth;
    _windowHeight = windowHeight;
    
    _nearDepth = nearDepth;
    _farDepth = farDepth;

    SetView();

    SetProjection(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth);
}

void Camera::Rotate(float xAxis, float yAxis, float zAxis) {
    Rotate(Vector3D(xAxis, yAxis, zAxis));
}
void Camera::Rotate(Vector3D angles) {
    RotateX(angles.x);
    RotateY(angles.y);
    RotateZ(angles.z);
}

void Camera::RotateX(float xAxis) {
    SetView(XMMatrixMultiply(GetView(), XMMatrixRotationX(xAxis)));
}
void Camera::RotateY(float yAxis) {
    SetView(XMMatrixMultiply(GetView(), XMMatrixRotationY(yAxis)));
}
void Camera::RotateZ(float zAxis) {
    SetView(XMMatrixMultiply(GetView(), XMMatrixRotationZ(zAxis)));
}

void Camera::SetLookAt(Vector3D at) {
    _at = at;
}

void Camera::SetLookTo(Vector3D to) {
    _to = to;
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
    XMVECTOR to = XMVectorSet(_to.x, _to.y, _to.z, 0.0f);
    XMVECTOR up = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);

    if (!_useLookTo)
        SetView(XMMatrixLookAtLH(eye, at, up));
    else
        SetView(XMMatrixLookToLH(eye, to, up));
}
void Camera::SetView(XMFLOAT4X4 view) {
    _view = view;
}

void Camera::SetView(XMMATRIX view) {
    XMStoreFloat4x4(&_view, view);
}

void Camera::SetUp(Vector3D up) {
    _up = up;
}

void Camera::Translate(float xAxis, float yAxis, float zAxis) {
    SetView(XMMatrixMultiply(GetView(), XMMatrixTranslation(xAxis, yAxis, zAxis)));
}

void Camera::Translate(Vector3D pos) {
    Translate(pos.x, pos.y, pos.z);
}

void Camera::Update() {
    
}

void Camera::UseLookTo(bool state) {
    _useLookTo = state;
}