#include "Camera.h"

Camera::Camera() {
    _points.clear();
    _pointIndex = 0;
    _isUsingWayPoints = false;
    _useLookTo = false;
    _lookAtTarget = false;

    _farDepth = 0;
    _nearDepth = 0;

    _angle = Vector3D(0.0f, 0.0f, 0.0f);

    _rotateSpeed = 0;
    _translateSpeed = 0;

    _windowHeight = 0;
    _windowWidth = 0;

    _view = {};
    _projection = {};
}

Camera::Camera(Vector3D position, Vector3D at, Vector3D up,
    int windowWidth, int windowHeight, 
    float nearDepth, float farDepth,
    float rotateSpeed, float translateSpeed) : _useLookTo(false), 
    _rotateSpeed(rotateSpeed), _translateSpeed(translateSpeed) {

    SetPos(position);
    SetLookAt(at);
    SetUp(up);
    
    Reshape(windowWidth, windowHeight, nearDepth, farDepth);
}

Camera::~Camera() {}

void Camera::Load() {

}

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

Vector3D Camera::GetOffset() {
    return _offset;
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

void Camera::GetVectors(XMFLOAT4X4& target, 
    Vector3D& translation, Vector3D& rotation, Vector3D& scale) {
    XMVECTOR vScale, vQuat, vTrans, vAxis;
    float radians;

    if (XMMatrixDecompose(&vScale, &vQuat, &vTrans, XMLoadFloat4x4(&target))) {
        // get scale
        scale = (
            Vector3D(
                XMVectorGetX(vScale),
                XMVectorGetY(vScale),
                XMVectorGetZ(vScale)
            )
        );

        // get translation
        translation = (
            Vector3D(
                XMVectorGetX(vTrans),
                XMVectorGetY(vTrans),
                XMVectorGetZ(vTrans)
            )
        );

        // get angle
        XMQuaternionToAxisAngle(&vAxis, &radians, vQuat);
        rotation = Vector3D(
            XMVectorGetX(vAxis) * radians,
            XMVectorGetY(vAxis) * radians,
            XMVectorGetZ(vAxis) * radians
        );


    }
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
    float step = forward / 1000;
    //*_translateSpeed;
    if (!_useLookTo) {
        Vector3D dist = Vector3D(_eye - _at).normalization();
        Vector3D result = _eye - (dist * (step * _translateSpeed));

        if (result.distance(_at) > 1)
            _eye = result;

        /*
        _eye.z += step;
        SetView();
        */
        
    } else {
        _eye += _to * (step * _translateSpeed);

        //Translate(_to * step);
    }
}

void Camera::MoveSidewards(float sideward) {
    float step = 0;
        
    step = (sideward / 1000);

    if (!_useLookTo) {
        // [ E3 ]
        
        Vector3D vup = _up.normalization();
        XMVECTOR upNormal = XMVectorSet(vup.x, vup.y, vup.z, 0.0f);
        XMVECTOR pos = XMVectorSet(_eye.x, _eye.y, _eye.z, 0.0f);
        XMMATRIX rotation = 
            XMMatrixRotationNormal(upNormal, step * _rotateSpeed);
        XMVECTOR result = XMVector3Transform(pos, rotation);
        Vector3D neweye = Vector3D(
            XMVectorGetX(result),
            XMVectorGetY(result),
            XMVectorGetZ(result)
        );
        
        _eye = neweye;

    } else {
        Vector3D eforward = _to.normalization();
        Vector3D eupwards = _up.normalization();
        Vector3D ecross = eforward.cross_product(eupwards);

        _eye -= ecross * (step * _translateSpeed);
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
    int windowWidth, int windowHeight,
    float nearDepth, float farDepth) {

    _windowWidth = windowWidth;
    _windowHeight = windowHeight;
    
    _nearDepth = nearDepth;
    _farDepth = farDepth;

    SetProjection(XM_PIDIV2, float(_windowWidth / _windowHeight), _nearDepth, _farDepth);
}

Vector3D Camera::Rotate(float angle, Vector3D axis, Vector3D origin) {
    Vector3D vangle = axis.normalization();
    XMVECTOR normal = XMVectorSet(vangle.x, vangle.y, vangle.z, 0.0f);
    XMVECTOR pos = XMVectorSet(origin.x, origin.y, origin.z, 0.0f);
    XMMATRIX rotation =
        XMMatrixRotationNormal(normal, angle * _rotateSpeed);
    XMVECTOR result = XMVector3Transform(pos, rotation);
    /*
    Vector3D neweye = Vector3D(
        XMVectorGetX(result),
        XMVectorGetY(result),
        XMVectorGetZ(result)
    );
    */
    return Vector3D(
        XMVectorGetX(result),
        XMVectorGetY(result),
        XMVectorGetZ(result)
    );
}

void Camera::SetLookAt(Vector3D at) {
    _at = at;
}

void Camera::SetLookTo(Vector3D to) {
    _to = to;
}

void Camera::SetOffset(Vector3D offset) {
    _offset = offset;
}

void Camera::SetPos(Vector3D pos) {
    _eye = pos;
}

void Camera::SetProjection
(float fovAngle, float ratio, float nearDepth, float farDepth) {
    XMStoreFloat4x4(
        &_projection, 
        XMMatrixPerspectiveFovLH(fovAngle, ratio, nearDepth, farDepth)
    );
}

void Camera::SetSpeed(float rotate, float translate) {
    _rotateSpeed = rotate;
    _translateSpeed = translate;
}

void Camera::SetView() {
    XMVECTOR eye = XMVectorSet(_eye.x, _eye.y, _eye.z, 0.0f);
    XMVECTOR at = XMVectorSet(_at.x, _at.y, _at.z, 0.0f);
    XMVECTOR to = XMVectorSet(_to.x, _to.y, _to.z, 0.0f);
    XMVECTOR up = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);

    if (!_useLookTo)
        SetView(XMMatrixLookAtLH(eye, at, up));
    else {
        SetView(
            XMMatrixMultiply(
                XMMatrixLookToLH(eye, to, up),
                XMMatrixRotationRollPitchYaw(_angle.x,_angle.y,_angle.z)
            )
        );
    }
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
    SetView(
        XMMatrixMultiply(GetView(), XMMatrixTranslation(xPos, yPos, zPos))
    );
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
            if (_pointIndex++ >= (int)_points.size() - 1)
                _pointIndex = 0;
        }
    }

    if (_followPlayer) {

    }

    SetView();

    //GetVectors(_view,_eye,_angle);

}

void Camera::UseLookTo(bool state) {
    _useLookTo = state;
}

bool Camera::UseLookTo() {
    return _useLookTo;
}

void Camera::UseWayPoints(bool state) {
    _isUsingWayPoints = state;
    if (state) _pointIndex = 0;
}