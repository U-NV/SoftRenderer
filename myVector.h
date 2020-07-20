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
    std::vector<T> data;

	Vector(int size){
        data.resize(size);
    };
	Vector(T X, T Y){
        data.resize(2);
        data[0] = X;
        data[1] = Y;
    };
	Vector(T X, T Y, T Z){
        data.resize(3);
        data[0] = X;
        data[1] = Y;
        data[2] = Z;
    };
	Vector(T X, T Y, T Z, T W){
        data.resize(4);
        data[0] = X;
        data[1] = Y;
        data[2] = Z;
        data[1] = W;
    };
    T& operator[] (const int idx) {
        return data[idx];
    }

    const T& operator[] (const int idx) const {
        return data[idx];
    }

    Vector<T> operator+(const Vector<T>& b)
    {
        int len = this->data.size();
        assert(len == b.data.size());
        Vector<T> temp(len);
        for (int i = 0; i < len; i++) {
            temp[i] = this->data[i] + b[i];
        }
        return temp;
    }

    Vector<T> operator-(const Vector<T>& b)
    {
        int len = this->data.size();
        assert(len == b.data.size());
        Vector<T> temp(len);
        for (int i = 0; i < len; i++) {
            temp[i] = this->data[i] - b[i];
        }
        return temp;
    }

    Vector<T> operator*(const T& b)
    {
        int len = this->data.size();
        Vector<T> temp(len);
        for (int i = 0; i < len; i++) {
            temp[i] = this->data[i] * b;
        }
        return temp;
    }

    Vector<T> operator/(const T& b)
    {
        int len = this->data.size();
        Vector<T> temp(len);
        for (int i = 0; i < len; i++) {
            temp[i] = this->data[i] / b;
        }
        return temp;
    }

    T operator*(const Vector<T>& b)
    {
        int len = this->data.size();
        assert(len == b.data.size());
        T sum = 0;
        for (int i = 0; i < len; i++) {
            sum += this->data[i] * b[i];
        }
        return sum;
    }
    
    T operator/(const Vector<T>& b)
    {
        int len = this->data.size();
        assert(len == b.data.size());
        Vector<T> temp(len);
        T sum = 0;
        for (int i = 0; i < len; i++) {
            sum += this->data[i] / b[i];
        }
        return sum;
    }
};


template <typename T>
class  Matrix
{
public:
    int row;
    int col;
    std::vector< std::vector<T>> m;


    std::vector<T>& operator[] (const int idx) {
        assert(idx < row);
        return m[idx];
    }

    const std::vector<T>& operator[] (const int idx) const {
        assert(idx < row);
        return m[idx];
    }

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
                std::cout << m[i][j]<<',';
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


};



#endif