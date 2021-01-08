#include "Vector3D.h"

Vector3D::Vector3D() {
    x = 0;
    y = 0;
    z = 0;
}

Vector3D::Vector3D(float x1, float y1, float z1) {
    x = x1;
    y = y1;
    z = z1;
}

Vector3D::Vector3D(const Vector3D& vec) {
    x = vec.x;
    y = vec.y;
    z = vec.z;
}

//addition
Vector3D Vector3D ::operator+(const Vector3D& vec) {
    //Returns a new vector summing the values for each component with the 
    //corresponding component in the added vector
    return Vector3D(x + vec.x, y + vec.y, z + vec.z);
}

Vector3D& Vector3D ::operator+=(const Vector3D& vec) {
    //Returns ‘this’ pointer (i.e. self-reference summing the values for 
    //each component with the corresponding component in the added vector

    x += vec.x;
    y += vec.y;
    z += vec.z;
    return *this;
}

//substraction//
Vector3D operator-(const Vector3D& lhs, const Vector3D& rhs) {
    //similar to addition
    return Vector3D(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

Vector3D Vector3D::operator-(const Vector3D& vec) {
    //similar to addition
    return Vector3D(x - vec.x, y - vec.y, z - vec.z);
}

Vector3D& Vector3D::operator-=(const Vector3D& vec) {
    //similar to addition
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    return *this;
}

//scalar multiplication
Vector3D Vector3D ::operator*(float value) {
    //similar to subtraction
    return Vector3D(x * value, y * value, z * value);
}
Vector3D& Vector3D::operator*=(float value) {
    //similar to subtraction
    x *= value;
    y *= value;
    z *= value;
    return *this;
}

//scalar division
Vector3D Vector3D ::operator/(float value) {
    assert(value != 0); //prevent divide by 0
    //similar to multiplication
    return Vector3D(x / value, y / value, z / value);
}
Vector3D& Vector3D ::operator/=(float value) {
    assert(value != 0);
    //similar to multiplication
    x /= value;
    y /= value;
    z /= value;
    return *this;
}
Vector3D& Vector3D::operator=(const Vector3D& vec) {
    //similar to addition
    x = vec.x;
    y = vec.y;
    z = vec.z;
    return *this;
}

bool Vector3D::operator==(const Vector3D& vec) {
    if (
        x == vec.x &&
        y == vec.y &&
        z == vec.z
        ) {
        return true;
    }
    else return false;
}

//Dot product (angle)
float Vector3D::dot_product(const Vector3D& vec) {
    //returns (x1*x2 + y1*y2 + x1*z2) 
    //where these are the terms from each vector 

    /* alternative solution 
    float dot = ((x * vec.x) + (y * vec.y) + (z * vec.z));
    Vector3D v = vec;
    float bLen = v.magnitude();
    float result = acos(dot/(magnitude()*bLen));
    */

    return ((x * vec.x) + (y * vec.y) + (z * vec.z));
}

//cross product
Vector3D Vector3D::cross_product(const Vector3D& vec) {
    //Calculate the terms (ni,nj,nk) using the dot product formula 
    //Then use to construct a vector using those terms and return

    // as an example using vec to represent second vector
    // the term ni in the output (new)vector is calculated as 

    // float ni=y*vec.z-z*vec.y;
    Vector3D result = {
        (y*vec.z)-(z*vec.y),
        (x*vec.z)-(z*vec.x),
        (x*vec.y)-(y*vec.x)
    };

    return result;
}

float Vector3D::magnitude() {
    //return square root of sum of the squared components
    return sqrtf((x * x) + (y * y) + (z * z));
}

Vector3D Vector3D::ComputeNormal(
    const Vector3D& p0,
    const Vector3D& p1,
    const Vector3D& p2
) {
    Vector3D u = p1 - p0;
    Vector3D v = p2 - p0;
    Vector3D result = u.cross_product(v);

    return result.normalization();
}

float Vector3D::square() {
    //sum of the squared components
    return x * x + y * y + z * z;
}

Vector3D Vector3D::normalization() {
    float len = this->magnitude();
    Vector3D result = Vector3D(
        (x==0)? 0 : x/len,
        (y==0)? 0 : y/len,
        (z==0)? 0 : z/len
    );

    return result;
}

float Vector3D::distance(const Vector3D& vec) {
    //same as mag? = NOPE!!
    //d = ((x2-x1)^2 + (y2-y1)^2 + (z2-z1)^2)^0.5

    Vector3D dif = {
        vec.x-x,
        vec.y-y,
        vec.z-z
    };

    return powf((dif.x * dif.x) + (dif.y * dif.y) + (dif.z * dif.z), 0.5);
}
float Vector3D::show_X() {
    return x;
}
float Vector3D::show_Y() {
    return y;
}
float Vector3D::show_Z() {
    return z;
}
void Vector3D::disp() {
    cout << x << " " << y << " " << z << endl;
}

float Vector3D::AngleTo(const Vector3D& vec) {
    float PI = (float)3.1415926535897932384626433;
    float angle = atan2f(x - vec.x, z - vec.z)* (180/PI);
    return angle * (180 / PI);
}

//returns X angle (YZ Dot product)
float Vector3D::AngleX(const Vector3D& vec) {
    return ((y * vec.y) + (z * vec.z));
}

//returns Y angle (XZ Dot product)
float Vector3D::AngleY(const Vector3D& vec) {
    return ((x * vec.x) + (z * vec.z));
}//returns Z angle (XY Dot product)

float Vector3D::AngleZ(const Vector3D& vec) {
    return ((x * vec.x) + (y * vec.y));
}