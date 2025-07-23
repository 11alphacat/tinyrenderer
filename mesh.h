#pragma once

#include <vector>
#include <string>
#include "XMath.h"
struct Submesh {
    std::string sFilename {};

    std::vector<X::XMFLOAT4> vertices;
    std::vector<X::XMFLOAT4> textures;
    std::vector<X::XMFLOAT4> normals;
    std::vector<uint> indicesv;
    std::vector<uint> indicest;
    std::vector<uint> indicesn;

    bool BuildSubmesh();
    void DrawSubmesh(TGAImage &framebuffer, TGAImage &zbuffer) const;
    
};