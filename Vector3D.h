#pragma once
#include<iostream>
#include<math.h>
#include<assert.h>

using namespace std;

class Vector3D {
public:
	/* these cause uncaught STUPID errors 
	const float PI = (float)acos(-1);
	const float RAD = 180 / PI;
	const float DEG = PI / 180;
	*/
	float x, y, z;

	//Constructors
	Vector3D();  //constructor	
	Vector3D(float x1, float y1, float z1 = 0);  //construct with values.
	Vector3D(const Vector3D& vec); //copy constructor

	//Arithemetic Operators – note use of overloading
	Vector3D operator+(const Vector3D& vec);   //addition
	Vector3D& operator+=(const Vector3D& vec);//assign new result to vector

	Vector3D operator- (const Vector3D& vec); //substraction
	//Vector3D operator- (const Vector3D& lhs, const Vector3D& rhs);
	Vector3D& operator-= (const Vector3D& vec);//assign new result to vector

	Vector3D operator*(float value);    //multiplication
	Vector3D& operator*=(float value);  //assign new result to vector.

	Vector3D operator/(float value);    //division
	Vector3D& operator/=(float value);  //assign new result to vector

	Vector3D& operator=(const Vector3D& vec);
	bool operator==(const Vector3D& vec);

	//Vector operations
	float dot_product(const Vector3D& vec); //scalar dot_product
	Vector3D cross_product(const Vector3D& vec); //cross_product
	Vector3D normalization();   //normalized vector

	float square(); //gives square of the vector
	float distance(const Vector3D& vec); //distance between two vectors
	float magnitude();  //magnitude of the vector

	Vector3D ComputeNormal(
		const Vector3D& p0,
		const Vector3D& p1,
		const Vector3D& p2
	);

	//Display operations 
	float show_X(); //return x
	float show_Y(); //return y
	float show_Z(); //return z
	void disp();    //display value of vectors

	//Angle operations
	float AngleTo(const Vector3D& vec);
	float AngleX(const Vector3D& vec);
	float AngleY(const Vector3D& vec);
	float AngleZ(const Vector3D& vec);
};

