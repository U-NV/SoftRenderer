#include "myVector.h"
template <> template <> 
vec<3, int>  ::vec(const vec<3, double>& v) : x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}

template <> template <> 
vec<3, double>::vec(const vec<3, int>& v) : x(v.x), y(v.y), z(v.z) {}

template <> template <> 
vec<2, int>  ::vec(const vec<2, double>& v) : x(int(v.x + .5f)), y(int(v.y + .5f)) {}

template <> template <> 
vec<2, double>::vec(const vec<2, int>& v) : x(v.x), y(v.y) {}






