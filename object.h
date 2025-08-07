#pragma once

#include "mesh.h"
#include "tgaimage.h"
#include "XMath.h"

class Object {
    std::vector<Submesh> vSubmeshList {};

    const int width { 1024 }, height { 1024 };

public:
    bool Initialize(const std::vector<std::string> &vsFilenameList);
    // void Transform(const comGL::Matrix &transform) {
    //     // Apply transformation to all submeshes
    //     for (auto &submesh : vSubmeshList) {
    //         // submesh.ApplyTransform(transform);
    //     }
    // }

    void Render(TGAImage &framebuffer, TGAImage &zbuffer);
};