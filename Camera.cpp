#include "Camera.h"

Camera::Camera() {
    _points.clear();
    _pointIndex = 0;
    _isUsingWayPoints = false;
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

void Camera::AddWayPoint(Vector3D newPoint) {
    WayPoint point = WayPoint();
    point.eye = newPoint;
    AddWayPoint(point);
}

void Camera::AddWayPoint(WayPoint newPoint) {
    _points.push_back(newPoint);
}

Vector3D Camera::GetAngle() {
    return _angle;
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

Vector3D Camera::GetRotation() {
    Vector3D angle;
    XMVECTOR scale, quat, trans, axis;
    float radians;

    if (XMMatrixDecompose(&scale, &quat, &trans, XMLoadFloat4x4(&_view))) {
        XMQuaternionToAxisAngle(&axis, &radians, quat);
        angle = Vector3D(
            XMVectorGetX(axis) * radians,
            XMVectorGetY(axis) * radians,
            XMVectorGetZ(axis) * radians
        );
    }
    /*
    if ((_view._11 == 1.0f) || (_view._11 == -1.0f)) {
        angle.x = atan2f(_view._13, _view._34);
    } else {
        angle.x = atan2f(-_view._31, _view._11);
        angle.y = asin(_view._21);
        angle.z = atan2f(-_view._23, _view._22);
    }
    */

    return angle;
}

Vector3D Camera::GetTranslation() {
    Vector3D pos;
    XMVECTOR scale, quat, trans;

    if (XMMatrixDecompose(&scale, &quat, &trans, XMLoadFloat4x4(&_view))) {
        pos = Vector3D(
            XMVectorGetX(trans),
            XMVectorGetY(trans),
            XMVectorGetZ(trans)
        );
    }

    return pos;
}

/*
//Returns the Yaw, Pitch, and Roll components of this matrix. This
//function only works with pure rotation matrices.
void GetRotation(float *v) const		{
    //yaw=v[0], pitch=v[1], roll=v[2]
    //Note, we use the cosf function rather than sinf just in case the
    //angles are greater than [-1,+1]
    v[1]  = -asinf(_32); //pitch
    float cp = cosf(v[1]); 			//_22 = cr * cp;
    float cr = _22 / cp;
    v[2] = acosf(cr);
    //_33 = cy * cp;			
    float cy = _33 / cp;			
    v[0] = acosf(cy);		
}		
//creates a rotation matrix based on euler angles Y * P * R		
//in the same order as DirectX.		
void Rotate(const float *v)		{
    //yaw=v[0], pitch=v[1], roll=v[2]
    float cy = cosf(v[0]);
    float cp = cosf(v[1]);
    float cr = cosf(v[2]);
    float sp = sinf(v[1]);
    float sr = sinf(v[2]);
    float sy = sinf(v[0]);
    _11  = cy * cr+ sr * sp * sy;
    _12 = sr * cp;
    _13 = cr * -sy + sr * sp * cy;
    _21 = -sr * cy + cr * sp * sy;
    _22 = cr * cp;
    _23 = -sr * -sy + cr * sp * cy;
    _31 = cp * sy;
    _32 = -sp;
    _33 = cy * cp;		
}
*/

Vector3D Camera::GetUp() {
    return _up;
}

XMFLOAT4X4 Camera::GetView4x4(){
    return _view;
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
        _eye += _to * step;

        Translate(_to * step);
    }
    
}

void Camera::MoveSidewards(float sideward) {
    if (!_useLookTo) {
        RotateY(sideward * _rotateSpeed);
    } else {
        Translate(sideward * _rotateSpeed, 0.0f, 0.0f);
    }
}

void Camera::MoveTo(Vector3D point, float speed) {
    //float dist = _eye.distance(point);

    Vector3D diff = Vector3D(_eye - point).normalization();
    _eye -= diff * (speed * 10);
    //Translate(_eye + (diff * speed));
    SetView();
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
    _angle.x = xAxis;
    SetView(XMMatrixMultiply(GetView(), XMMatrixRotationX(xAxis)));
}
void Camera::RotateY(float yAxis) {
    _angle.y = yAxis;
    SetView(XMMatrixMultiply(GetView(), XMMatrixRotationY(yAxis)));
}
void Camera::RotateZ(float zAxis) {
    _angle.z = zAxis;
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

void Camera::Translate(float xPos, float yPos, float zPos) {
    SetPos(Vector3D(xPos, yPos, zPos));
    SetView(XMMatrixMultiply(GetView(), XMMatrixTranslation(xPos, yPos, zPos)));
}

void Camera::Translate(Vector3D pos) {
    Translate(pos.x, pos.y, pos.z);
}

void Camera::Update() {
    if (_isUsingWayPoints) {
        if (_points.size() < 1) 
            // list of waypoints is empty!!
            return;

        //Vector3D dist = 
        if (_eye.distance(_points[_pointIndex].eye) > 0.01f) {
            // waypoint not reached
            MoveTo(_points[_pointIndex].eye, _translateSpeed);

        } else {
            // close enough to waypoint
            _eye = _points[_pointIndex].eye;
            if (_pointIndex++ >= _points.size() - 1)
                _pointIndex = 0;
        }
    }
}

void Camera::UseLookTo(bool state) {
    _useLookTo = state;
}

void Camera::UseWayPoints(bool state) {
    _isUsingWayPoints = state;
    if (state) _pointIndex = 0;
}