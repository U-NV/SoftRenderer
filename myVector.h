#pragma once
#ifndef __MY_VECTOE__
#define __MY_VECTOE__
#include <iostream>
#include<vector>
#include <stdio.h>
#include <cassert>

template <typename T>
class Vector {
public:
	T x;
	T y;
	T z;
	T w;
	int len;
	Vector() :x(0), y(0), z(0), w(0),len(4) {};
	Vector(T X, T Y) :x(X), y(Y), z(0), w(0), len(2) {};
	Vector(T X, T Y, T Z) :x(X), y(Y), z(Z), w(0), len(3) {};
	Vector(T X, T Y, T Z, T W) :x(X), y(Y), z(Z), w(W), len(4) {};

    Vector<T> operator+(const Vector<T>& b)
    {
        Vector<T> temp;
        temp.x = this->x + b.x;
        temp.y = this->y + b.y;
        temp.z = this->z + b.z;
        temp.w = this->w + b.w;
        return temp;
    }

    Vector<T> operator-(const Vector<T>& b)
    {
        Vector<T> temp;
        temp.x = this->x - b.x;
        temp.y = this->y - b.y;
        temp.z = this->z - b.z;
        temp.w = this->w - b.w;
        return temp;
    }

    Vector<T> operator*(const T& b)
    {
        Vector<T> temp;
        temp.x = this->x * b;
        temp.y = this->y * b;
        temp.z = this->z * b;
        temp.w = this->w * b;
        return temp;
    }

    Vector<T> operator/(const T& b)
    {
        Vector<T> temp;
        temp.x = this->x / b;
        temp.y = this->y / b;
        temp.z = this->z / b;
        temp.w = this->w / b;
        return temp;
    }

    T operator*(const Vector<T>& b)
    {
        return this->x * b.x + this->y * b.y + this->z * b.z + this->w * b.w;
    }
};


template <typename T>
class  Matrix
{
public:
    Matrix(int rows, int cols) :row(rows), col(cols)
    {
        m.resize(rows);
        for (int i = 0; i < row; i++) {
            m[i].resize(cols);
            for (int j = 0; j < col; j++) {
                m[i][j] = 0;
            }
        }
    }

    Matrix<T> identity() {
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col; j++)
                m[i][j] = (i == j);
        return this;
    }

    void print()
    {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                std::cout << m[i][j];
            }
            std::cout << std::endl;
        }
    }

    ~Matrix()
    {
    }

    Matrix<T> operator+(const Matrix<T>& b)
    {
        Matrix<T> temp(row, col);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                temp.m[i][j] = this->m[i][j] + b.m[i][j];
            }
        }
        return temp;
    }

    Matrix<T> operator-(const Matrix<T>& b)
    {
        Matrix<T> temp(row, col);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                temp.m[i][j] = this->m[i][j] - b.m[i][j];
            }
        }
        return temp;
    }

    Matrix<T> operator*(const T& a)
    {
        Matrix<T> temp(row, col);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                temp.m[i][j] = this->m[i][j] * a;
            }
        }
        return temp;
    }

    Matrix<T> operator/(const T& a)
    {
        Matrix<T> temp(row, col);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                temp.m[i][j] = this->m[i][j] / a;
            }
        }
        return temp;
    }

    Matrix<T> operator*(const Matrix<T>& b)
    {
        int leftRow = this->row;
        int leftCol = this->col;

        int rightRow = b.row;
        int rightCol = b.col;

        assert(leftCol == rightRow);
        Matrix<T> temp(leftRow, rightCol);
        for (int i = 0; i < leftRow; i++) {
            for (int j = 0; j < rightCol; j++) {
                int sum = 0;
                for (int k = 0; k < leftCol; k++) {
                    sum += this->m[i][k] * b.m[k][j];
                }
                temp.m[i][j] = sum;
            }
        }

        return temp;
    }

private:
	int row;
	int col;
	std::vector< std::vector<T>> m;
};



#endif