#pragma once
#ifndef __MY_VECTOE__
#define __MY_VECTOE__
#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>
#include<algorithm>

template<size_t DimCols, size_t DimRows, typename T> class mat;

template <size_t DIM, typename T> struct vec {
    vec() { for (size_t i = DIM; i--; data_[i] = T()); }
    T& operator[](const size_t i) { assert(i < DIM); return data_[i]; }
    const T& operator[](const size_t i) const { assert(i < DIM); return data_[i]; }
private:
    T data_[DIM];
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<2, T> {
    vec() : x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    template <class U> vec<2, T>(const vec<2, U>& v);
    T& operator[](const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }

    T x, y;
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<3, T> {
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    template <class U> vec<3, T>(const vec<3, U>& v);
    T& operator[](const size_t i) { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    const T& operator[](const size_t i) const { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    float norm() { return std::sqrt(x * x + y * y + z * z); }
    vec<3, T>& normalize(T l = 1) { *this = (*this) * (l / norm()); return *this; }

    T x, y, z;
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DIM, typename T> T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
    T ret = T();
    for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]);
    return ret;
}


template<size_t DIM, typename T>vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = DIM; i--; lhs[i] += rhs[i]);
    return lhs;
}

template<size_t DIM, typename T>vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
    return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = DIM; i--; lhs[i] *= rhs);
    return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = DIM; i--; lhs[i] /= rhs);
    return lhs;
}

template<size_t LEN, size_t DIM, typename T>
vec<LEN, T> embed(const vec<DIM, T>& v, T fill = 1) {
    vec<LEN, T> ret;
    for (size_t i = LEN; i--; ret[i] = (i < DIM ? v[i] : fill));
    return ret;
}

template<size_t LEN, size_t DIM, typename T>
vec<LEN, T> proj(const vec<DIM, T>& v) {
    vec<LEN, T> ret;
    for (size_t i = LEN; i--; ret[i] = v[i]);
    return ret;
}

template <typename T> vec<3, T> cross(vec<3, T> a, vec<3, T> b) {
    return vec<3, T>(a.y * b.z - a.z * b.y, 
                     a.z * b.x - a.x * b.z, 
                     a.x * b.y - a.y * b.x);
}

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM, T>& v) {
    for (unsigned int i = 0; i < DIM; i++) {
        out << v[i] << " ";
    }
    return out;
}

/////////////////////////////////////////////////////////////////////////////////

template<size_t DIM, typename T> struct dt {
    static T det(const mat<DIM, DIM, T>& src) {
        T ret = 0;
        for (size_t i = DIM; i--; ret += src[0][i] * src.cofactor(0, i));
        return ret;
    }
};

template<typename T> struct dt<1, T> {
    static T det(const mat<1, 1, T>& src) {
        return src[0][0];
    }
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DimRows, size_t DimCols, typename T> class mat {
    vec<DimCols, T> rows[DimRows];
public:
    mat() {}

    vec<DimCols, T>& operator[] (const size_t idx) {
        assert(idx < DimRows);
        return rows[idx];
    }

    const vec<DimCols, T>& operator[] (const size_t idx) const {
        assert(idx < DimRows);
        return rows[idx];
    }

    vec<DimRows, T> col(const size_t idx) const {
        assert(idx < DimCols);
        vec<DimRows, T> ret;
        for (size_t i = DimRows; i--; ret[i] = rows[i][idx]);
        return ret;
    }

    void set_col(size_t idx, vec<DimRows, T> v) {
        assert(idx < DimCols);
        for (size_t i = DimRows; i--; rows[i][idx] = v[i]);
    }

    static mat<DimRows, DimCols, T> identity() {
        mat<DimRows, DimCols, T> ret;
        for (size_t i = DimRows; i--; )
            for (size_t j = DimCols; j--; ret[i][j] = (i == j));
        return ret;
    }

    T det() const {
        return dt<DimCols, T>::det(*this);
    }

    mat<DimRows - 1, DimCols - 1, T> get_minor(size_t row, size_t col) const {
        mat<DimRows - 1, DimCols - 1, T> ret;
        for (size_t i = DimRows - 1; i--; )
            for (size_t j = DimCols - 1; j--; ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1]);
        return ret;
    }

    T cofactor(size_t row, size_t col) const {
        return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
    }

    mat<DimRows, DimCols, T> adjugate() const {
        mat<DimRows, DimCols, T> ret;
        for (size_t i = DimRows; i--; )
            for (size_t j = DimCols; j--; ret[i][j] = cofactor(i, j));
        return ret;
    }

    mat<DimRows, DimCols, T> invert_transpose() {
        mat<DimRows, DimCols, T> ret = adjugate();
        T tmp = ret[0] * rows[0];
        return ret / tmp;
    }

    mat<DimRows, DimCols, T> invert() {
        return invert_transpose().transpose();
    }

    mat<DimCols, DimRows, T> transpose() {
        mat<DimCols, DimRows, T> ret;
        for (size_t i = DimCols; i--; ret[i] = this->col(i));
        return ret;
    }
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DimRows, size_t DimCols, typename T>
vec<DimRows, T> operator*(const mat<DimRows, DimCols, T>& lhs, const vec<DimCols, T>& rhs) {
    vec<DimRows, T> ret;
    for (size_t i = DimRows; i--; ret[i] = lhs[i] * rhs);
    return ret;
}

template<size_t R1, size_t C1, size_t C2, typename T>
mat<R1, C2, T> operator*(const mat<R1, C1, T>& lhs, const mat<C1, C2, T>& rhs) {
    mat<R1, C2, T> result;
    for (size_t i = R1; i--; )
        for (size_t j = C2; j--; result[i][j] = lhs[i] * rhs.col(j));
    return result;
}

template<size_t DimRows, size_t DimCols, typename T>
mat<DimCols, DimRows, T> operator/(mat<DimRows, DimCols, T> lhs, const T& rhs) {
    for (size_t i = DimRows; i--; lhs[i] = lhs[i] / rhs);
    return lhs;
}

template <size_t DimRows, size_t DimCols, class T>
std::ostream& operator<<(std::ostream& out, mat<DimRows, DimCols, T>& m) {
    for (size_t i = 0; i < DimRows; i++) out << m[i] << std::endl;
    return out;
}

/////////////////////////////////////////////////////////////////////////////////

typedef vec<2, float> Vec2f;
typedef vec<2, int>   Vec2i;
typedef vec<3, float> Vec3f;
typedef vec<3, int>   Vec3i;
typedef vec<4, float> Vec4f;
typedef mat<4, 4, float> Matrix;


//#include <iostream>
//#include<vector>
//#include <stdio.h>
//#include <cassert>
//
//
//template <typename T>
//class  Matrix;
//
//template <typename T>
//class Vector {
//public:
//    std::vector<T> data;
//
//    Vector() {};
//
//	Vector(int size){
//        data.resize(size);
//    };
//	Vector(T X, T Y){
//        data.resize(2);
//        data[0] = X;
//        data[1] = Y;
//    };
//	Vector(T X, T Y, T Z){
//        data.resize(3);
//        data[0] = X;
//        data[1] = Y;
//        data[2] = Z;
//    };
//	Vector(T X, T Y, T Z, T W){
//        data.resize(4);
//        data[0] = X;
//        data[1] = Y;
//        data[2] = Z;
//        data[3] = W;
//    };
//    T& operator[] (const int idx) {
//        return data[idx];
//    }
//
//    Vector<T> homogeneous() {
//        data.resize(4);
//        data[3] = 1;
//        return *this;
//    }
//
//    const T& operator[] (const int idx) const {
//        return data[idx];
//    }
//
//    void print() {
//        std::cout << "(";
//        for (unsigned int i = 0; i < data.size(); i++) {
//            std::cout << data[i];
//            if (i < data.size() - 1)
//                std::cout << ", ";
//        }
//        std::cout << ")"<<std::endl;
//    }
//
//    int len() {
//        return data.size();
//    }
//
//    Matrix<T> toMatrix(bool col = true) {
//        int vLen = data.size();
//        Matrix<T> mat(vLen, 1);
//        if (col) {
//            for (int i = 0; i < vLen; i++) {
//                mat.m[i][0] = data[i];
//            }
//        }
//        else {
//            mat.resize(1, vLen);
//            for (int i = 0; i < vLen; i++) {
//                mat.m[0][i] = data[i];
//            }
//        }
//        return mat;
//    }
//
//    double norm() {
//        T sum = 0;
//        for (unsigned int i = 0; i < data.size(); i++) {
//            sum += data[i] * data[i];
//        }
//        return std::sqrt(sum); 
//    }
//    Vector<T> normalize() {
//        *this = (*this) * (1.0 / norm()); 
//        return *this;
//    }
//
//    Vector<T> operator+(const Vector<T>& b)
//    {
//        int len = this->data.size();
//        assert(len == b.data.size());
//        for (int i = 0; i < len; i++) {
//            this->data[i] = this->data[i] + b[i];
//        }
//        return *this;
//    }
//
//    Vector<T> operator-(const Vector<T>& b)
//    {
//        int len = this->data.size();
//        assert(len == b.data.size());
//        for (int i = 0; i < len; i++) {
//            this->data[i] = this->data[i] - b[i];
//        }
//        return *this;
//    }
//
//    Vector<T> operator*(const T& b)
//    {
//        size_t len = this->data.size();
//        Vector<T> result(len);
//        for (size_t i = 0; i < len; i++) {
//            result.data[i] = this->data[i] * b;
//        }
//        return result;
//    }
//
//
//    Vector<T> operator/(const T& b)
//    {
//        int len = this->data.size();
//        Vector<T> temp(len);
//        for (int i = 0; i < len; i++) {
//            temp.data[i] = this->data[i] / b;
//        }
//        return temp;
//    }
//
//    T operator*(const Vector<T>& b)
//    {
//        int len = this->data.size();
//        assert(len == b.data.size());
//        T sum = T();
//        for (int i = 0; i < len; i++) {
//            sum += this->data[i] * b[i];
//        }
//        return sum;
//    }
//
//    T operator/(const Vector<T>& b)
//    {
//        int len = this->data.size();
//        assert(len == b.data.size());
//        T sum = 0;
//        for (int i = 0; i < len; i++) {
//            sum += this->data[i] / b[i];
//        }
//        return sum;
//    }
//
//    
//};
//
//
//template <typename T>
//class  Matrix
//{
//public:
//    int row;
//    int col;
//    std::vector< std::vector<T>> m;
//    /*std::vector<T>& operator[] (const int idx) {
//        assert(idx < row);
//        return m[idx];
//    }
//
//    const std::vector<T>& operator[] (const int idx) const {
//        assert(idx < row);
//        return m[idx];
//    }*/
//    Matrix() {
//        row = 0;
//        col = 0;
//    };
//    Matrix(int rows, int cols) :row(rows), col(cols)
//    {
//        m.resize(rows);
//        for (int i = 0; i < row; i++) {
//            m[i].resize(cols);
//            for (int j = 0; j < col; j++) {
//                m[i][j] = 0;
//            }
//        }
//    }
//
//    void resize(int rows, int cols){
//        row = rows;
//        col = cols;
//        m.resize(rows);
//        for (int i = 0; i < rows; i++) {
//            m[i].resize(cols);
//        }
//    }
//
//    void identity() {
//        for (int i = 0; i < row; i++)
//            for (int j = 0; j < col; j++)
//                m[i][j] = (i == j);
//    }
//
//    void print()
//    {
//        for (int i = 0; i < row; i++) {
//            std::cout << "|";
//            for (int j = 0; j < col; j++) {
//                std::cout << m[i][j];
//                if (j < col - 1)
//                    std::cout << ' ';
//            }
//            std::cout << "|" << std::endl;
//        }
//    }
//
//    ~Matrix()
//    {
//    }
//
//    Matrix<T> operator*(const Matrix<T>& b)
//    {
//        int leftRow = this->row;
//        int leftCol = this->col;
//
//        int rightRow = b.row;
//        int rightCol = b.col;
//
//        assert(leftCol == rightRow);
//
//        Matrix<T> temp(leftRow, rightCol);
//        T sum = 0.0;
//        for (int i = 0; i < leftRow; i++) {
//            for (int j = 0; j < rightCol; j++) {
//                sum = 0.0;
//                for (int k = 0; k < leftCol; k++) {
//                    sum += this->m[i][k] * b.m[k][j];
//                }
//                temp.m[i][j] = sum;
//            }
//        }
//
//        return temp;
//    }
//    
//    Vector<T> operator*(const Vector<T>& b)
//    {
//        int leftRow = this->row;
//        int leftCol = this->col;
//        int right = b.data.size();
//        assert(right == 4 && leftRow = 4);
//
//        T result[4] = {0};
//        for (int i = 0; i < 4; i++) {
//            result[i] = 0;
//            for (int j = 0; j < 4; j++) {
//                result[i] += b.data[j] * this->m[i][j];
//            }
//        }
//        return Vector<T>(result[0], result[1], result[2], result[3]);
//    }
//
//    Vector<T> toVector() {
//        int max = col > row ? col : row;
//        Vector<T> vec(max);
//        if (col == 1) {
//            for (int i = 0; i < max; i++) {
//                vec.data[i] = m[i][0];
//            }
//        }
//        return vec;
//    }
//
//
//
//    //Matrix<T> operator+(const Matrix<T>& b)
//    //{
//    //    Matrix<T> temp(row, col);
//    //    for (int i = 0; i < row; i++) {
//    //        for (int j = 0; j < col; j++) {
//    //            temp.m[i][j] = this->m[i][j] + b.m[i][j];
//    //        }
//    //    }
//    //    return temp;
//    //}
//
//    //Matrix<T> operator-(const Matrix<T>& b)
//    //{
//    //    Matrix<T> temp(row, col);
//    //    for (int i = 0; i < row; i++) {
//    //        for (int j = 0; j < col; j++) {
//    //            temp.m[i][j] = this->m[i][j] - b.m[i][j];
//    //        }
//    //    }
//    //    return temp;
//    //}
//
//    //Matrix<T> operator*(const T& a)
//    //{
//    //    Matrix<T> temp(row, col);
//    //    for (int i = 0; i < row; i++) {
//    //        for (int j = 0; j < col; j++) {
//    //            temp.m[i][j] = this->m[i][j] * a;
//    //        }
//    //    }
//    //    return temp;
//    //}
//
//    //Matrix<T> operator/(const T& a)
//    //{
//    //    Matrix<T> temp(row, col);
//    //    for (int i = 0; i < row; i++) {
//    //        for (int j = 0; j < col; j++) {
//    //            temp.m[i][j] = this->m[i][j] / a;
//    //        }
//    //    }
//    //    return temp;
//    //}
//
//};


#endif