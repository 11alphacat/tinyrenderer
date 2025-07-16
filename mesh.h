#pragma once

#include <vector>
#include <string>
#include "common.h"

struct Submesh {
    std::string sFilename {};

    std::vector<comGL::vec3> vertices;
    std::vector<comGL::vec3> textures;
    std::vector<comGL::vec3> normals;
    std::vector<uint> indicesv;
    std::vector<uint> indicest;
    std::vector<uint> indicesn;

    bool BuildSubmesh();
    void DrawSubmesh(TGAImage &framebuffer, TGAImage &zbuffer) const;
    
};