#pragma once
#ifndef __MY_VECTOE__
#define __MY_VECTOE__
#include <iostream>
#include<vector>
#include <stdio.h>
#include <cassert>


template <typename T>
class  Matrix;

template <typename T>
class Vector {
public:
    std::vector<T> data;

    Vector() {};

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

    Vector<T> homogeneous() {
        data.resize(4);
        data[3] = 1;
        return *this;
    }

    const T& operator[] (const int idx) const {
        return data[idx];
    }

    void print() {
        std::cout << "(";
        for (unsigned int i = 0; i < data.size(); i++) {
            std::cout << data[i];
            if (i < data.size() - 1)
                std::cout << ", ";
        }
        std::cout << ")"<<std::endl;
    }

    int len() {
        return data.size();
    }

    Matrix<T> toMatrix(bool col = true) {
        int vLen = data.size();
        Matrix<T> mat(vLen, 1);
        if (col) {
            for (int i = 0; i < vLen; i++) {
                mat[i][0] = data[i];
            }
        }
        else {
            mat.resize(1, vLen);
            for (int i = 0; i < vLen; i++) {
                mat[0][i] = data[i];
            }
        }
        return mat;
    }

    double norm() {
        T sum = 0;
        for (unsigned int i = 0; i < data.size(); i++) {
            sum += data[i] * data[i];
        }
        return std::sqrt(sum); 
    }
    Vector<T> normalize() {
        *this = (*this) * (1.0 / norm()); 
        return *this;
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
    Matrix() {
        row = 0;
        col = 0;
    };
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

    void resize(int rows, int cols){
        row = rows;
        col = cols;
        m.resize(rows);
        for (int i = 0; i < rows; i++) {
            m[i].resize(cols);
        }
    }

    void identity() {
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col; j++)
                m[i][j] = (i == j);
    }

    void print()
    {
        for (int i = 0; i < row; i++) {
            std::cout << "|";
            for (int j = 0; j < col; j++) {
                std::cout << m[i][j];
                if (j < col - 1)
                    std::cout << ' ';
            }
            std::cout << "|" << std::endl;
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
                T sum = 0.0;
                for (int k = 0; k < leftCol; k++) {
                    sum += this->m[i][k] * b.m[k][j];
                }
                temp[i][j] = sum;
            }
        }

        return temp;
    }
    
    Vector<T> toVector() {
        int max = col > row ? col : row;
        Vector<T> vec(max);
        if (col == 1) {
            for (int i = 0; i < max; i++) {
                vec[i] = m[i][0];
            }
        }
        return vec;
    }

};


#endif