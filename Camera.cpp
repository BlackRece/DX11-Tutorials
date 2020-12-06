#include "Camera.h"

CameraC::CameraC() :
    rotateSpeed(1.0f), translateSpeed(1.0f) {

}

XMMATRIX CameraC::GetProjection() {
    return XMLoadFloat4x4(&_projection);
}

XMMATRIX CameraC::GetView() {
    return XMLoadFloat4x4(&_view);
}

XMMATRIX CameraC::GetViewProj() {
    return XMMatrixMultiply(GetView(), GetProjection());
}

void CameraC::SetProjection(float fovAngle, float ratio, float nearClip, float farClip) {
    XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(fovAngle, ratio, nearClip, farClip));
}

void CameraC::Translate(float xAxis, float yAxis, float zAxis) {
    Translate(Vector3D(xAxis, yAxis, zAxis));
}

void CameraC::Translate(Vector3D position) {
    Vector3D diff = _eye - position;
    _eye += diff;
    _at += diff;

    Update();
}

void CameraC::Update() {
    XMVECTOR eye = XMVectorSet(_eye.x, _eye.y, _eye.z, 0.0f);
    XMVECTOR at = XMVectorSet(_at.x, _at.y, _at.z, 0.0f);
    XMVECTOR up = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);

    XMStoreFloat4x4(&_view, XMMatrixLookAtLH(eye, at, up));
}
