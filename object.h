#pragma once

#include "mesh.h"
#include "tgaimage.h"

class Object {
    std::vector<Submesh> vSubmeshList {};

    // todo
    // comGL::Matrix transform = world * view * projection;
public:
    bool Initialize(const std::vector<std::string> &vszFilenameList);
    // void Transform(const comGL::Matrix &transform) {
    //     // Apply transformation to all submeshes
    //     for (auto &submesh : vSubmeshList) {
    //         // submesh.ApplyTransform(transform);
    //     }
    // }

    void Render(TGAImage &framebuffer, TGAImage &zbuffer) ;
};