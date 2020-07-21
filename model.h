#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "myVector.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<Vector<double>> verts_;
    std::vector<std::vector<Vector<int>> > faces_; // attention, this Vector<int> means vertex/uv/normal
    std::vector<Vector<double>> norms_;
    std::vector<Vector<double>> uv_;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    Vector<double> normal(int iface, int nthvert);
    Vector<double> normal(Vector<double> uv);
    Vector<double> vert(int i);
    Vector<double> vert(int iface, int nthvert);
    Vector<double> uv(int iface, int nthvert);
    TGAColor diffuse(Vector<double> uv);
    float specular(Vector<double> uv);
    std::vector<int> face(int idx);
};
#endif //__MODEL_H__

