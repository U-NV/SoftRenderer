#pragma once
#ifndef __MY_VECTOE__
#define __MY_VECTOE__
#include <iostream>
#include <stdio.h>

class Vector {
public:
	double x;
	double y;
	double z;
	double w;
	int len;
	Vector() :x(0), y(0), z(0), w(0),len(4) {};
	Vector(double X, double Y) :x(X), y(Y), z(0), w(0), len(2) {};
	Vector(double X, double Y, double Z) :x(X), y(Y), z(Z), w(0), len(3) {};
	Vector(double X, double Y, double Z, double W) :x(X), y(Y), z(Z), w(W), len(4) {};

	Vector operator+(const Vector& b);
	Vector operator-(const Vector& b);
	Vector operator*(const int& a);
	Vector operator/(const int& a);
	int operator*(const Vector& b);
};
//class  Matrix
//{
//public:
//	 Matrix();
//	~ Matrix();
//
//	Matrix operator+(const Matrix& b);
//	Matrix operator-(const Matrix& b);
//	Matrix operator*(const int& a);
//	Matrix operator/(const int& a);
//	int operator*(const Vector& b);
//
//private:
//	double m[4][4];
//};
//
//Matrix::Matrix()
//{
//}
//
//Matrix::~Matrix()
//{
//}

#endif