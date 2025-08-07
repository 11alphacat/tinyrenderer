#pragma once

#include <vector>
#include <string>
#include "XMath.h"
struct Submesh {
    std::string sFilename {};

    std::vector<X::XMFLOAT4> vertices {};
    std::vector<X::XMFLOAT3> textures {};
    std::vector<X::XMFLOAT4> normals {};
    std::vector<uint> indicesv {};
    std::vector<uint> indicest {};
    std::vector<uint> indicesn {};

    std::vector<float> intensities {};
    X::XMFLOAT3 lightN {}; 

    TGAImage diffusemap {};
    TGAImage normalmap {};
    TGAImage specularmap {};

    bool BuildSubmesh();    
    void DrawSubmesh(TGAImage &framebuffer, TGAImage &zbuffer);
    void LoadTexture(const std::string& filename, const char *suffix, TGAImage &img);

private:
    void DrawTriangle(TGAImage &framebuffer, TGAImage &zbuffer) const;
    bool fragment(X::FXMVECTOR bar, TGAColor& color) const;
    X::XMFLOAT4 va_a {}, va_b {}, va_c {}; // varying triangle coordinates
    X::XMFLOAT3X3 va_tex {}; // varying texture coordinates: <-ta-> /br <-tb-> /br <-tc->
    // X::XMFLOAT2 va_ta {}, va_tb {}, va_tc {}; // varying texture coordinates
#if defined(NO_NORMAL_MAP)
    X::XMFLOAT3 va_intensity {}; 
#endif
};