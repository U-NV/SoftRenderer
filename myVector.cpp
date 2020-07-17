#include "myVector.h"

Vector2i Vector2i::operator+(const Vector2i& b)
{
    Vector2i temp;
    temp.x = this->x + b.x;
    temp.y = this->y + b.y;
    return temp;
}
Vector2i Vector2i::operator-(const Vector2i& b)
{
    Vector2i temp;
    temp.x = this->x - b.x;
    temp.y = this->y - b.y;
    return temp;
}
Vector2i Vector2i::operator*(const int& a)
{
    Vector2i temp;
    temp.x = this->x*a;
    temp.y = this->y*a;
    return temp;
}

int Vector2i::operator*(const Vector2i& b)
{
    return this->x*b.x + this->y*b.y;
}

std::ostream& operator<<(std::ostream& out, Vector2i& b)
{
    out << b.x << "," << b.y;
    return out;
    // TODO: 在此处插入 return 语句
}
