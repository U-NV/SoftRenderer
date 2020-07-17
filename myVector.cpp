#include "myVector.h"

Vector Vector::operator+(const Vector& b)
{
    Vector temp;
    temp.x = this->x + b.x;
    temp.y = this->y + b.y;
    temp.z = this->z + b.z;
    temp.w = this->w + b.w;
    return temp;
}
Vector Vector::operator-(const Vector& b)
{
    Vector temp;
    temp.x = this->x - b.x;
    temp.y = this->y - b.y;
    temp.z = this->z - b.z;
    temp.w = this->w - b.w;
    return temp;
}
Vector Vector::operator*(const int& b)
{
    Vector temp;
    temp.x = this->x * b;
    temp.y = this->y * b;
    temp.z = this->z * b;
    temp.w = this->w * b;
    return temp;
}

Vector Vector::operator/(const int& b)
{
    Vector temp;
    temp.x = this->x / b;
    temp.y = this->y / b;
    temp.z = this->z / b;
    temp.w = this->w / b;
    return temp;
}

int Vector::operator*(const Vector& b)
{
    return this->x * b.x + this->y * b.y + this->z * b.z + this->w * b.w;
}