#include "Vector3D.h"

Vector3D::Vector3D()  //constructor
{
    x = 0;
    y = 0;
    z = 0;
}

Vector3D::Vector3D(float x1, float y1, float z1)  //construct with values.
{
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
Vector3D Vector3D ::operator-(const Vector3D& vec) {
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

//Dot product
float Vector3D::dot_product(const Vector3D& vec) {
    //returns (x1*x2 + y1*y2 + x1*z2) where these are the terms from
    // each vector 
    return 0.0f;
}

//cross product
Vector3D Vector3D::cross_product(const Vector3D& vec) {
    //Calculate the terms (ni,nj,nk) using the dot product formula 
    //Then use to construct a vector using those terms and return

    // as an example using vec to represent second vector
    // the term ni in the output (new)vector is calculated as 

// float ni=y*vec.z-z*vec.y;

    return *this;
}

float Vector3D::magnitude() {
    //return square root of sum of the squared components
    return 0.0f;
}
float Vector3D::square() {
    return x * x + y * y + z * z;
}
Vector3D Vector3D::normalization() {
    return *this;
}
float Vector3D::distance(const Vector3D& vec) {
    return 0.0f;
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
