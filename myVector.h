#pragma once
#ifndef __MY_VECTOE__
#define __MY_VECTOE__
#include <iostream>
#include <stdio.h>
class Vector2i {
public:
	int x;
	int y;
	Vector2i() :x(0), y(0) {};
	Vector2i(int X, int Y) :x(X), y(Y) {};

	Vector2i operator+(const Vector2i& b); 
	Vector2i operator-(const Vector2i& b);
	Vector2i operator*(const int& a);
	int operator*(const Vector2i& b);

};
std::ostream& operator<<(std::ostream& out, Vector2i& b);

class Vector3 {
public:
	double x;
	double y;
	double z;
	Vector3() :x(0), y(0), z(0) {};
	Vector3(double X, double Y, double Z) :x(X), y(Y) , z(Z) {};
};

class Vector4 {
public:
	double x;
	double y;
	double z;
	double w;
	Vector4() :x(0), y(0), z(0), w(0) {};
	Vector4(double X, double Y, double Z, double W) :x(X), y(Y), z(Z), w(W) {};
};

template <size_t DIM, typename T> struct vec {
	vec() { for (size_t i = DIM; i--; data_[i] = T()); }
	T& operator[](const size_t i) { assert(i < DIM); return data_[i]; }
	const T& operator[](const size_t i) const { assert(i < DIM); return data_[i]; }
private:
	T data_[DIM];
};

template <typename T> struct vec<2, T> {
	vec() : x(T()), y(T()) {}
	vec(T X, T Y) : x(X), y(Y) {}
	template <class U> vec<2, T>(const vec<2, U>& v);
	T& operator[](const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
	const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }

	T x, y;
};

#endif