#include "object.h"
#include <iostream>

bool Object::Initialize(const std::vector<std::string> &vszFilenameList)
{
    
    for (const auto &filename : vszFilenameList) {
        Submesh submesh;
        submesh.sFilename = filename;
        if (!submesh.BuildSubmesh()) {
            std::cerr << "Failed to build submesh from file: " << filename << std::endl;
            return false;
        }
        vSubmeshList.emplace_back(std::move(submesh));
    }
    return true;
}

void Object::Render(TGAImage &framebuffer, TGAImage &zbuffer) 
{
    // TODO: transform
    for (auto &submesh: vSubmeshList) {
        for (auto &v: submesh.vertices) {
            v += 1.0;
            v *= comGL::vec3 {
                static_cast<double>(framebuffer.width())  / 2.0,
                static_cast<double>(framebuffer.height()) / 2.0,
                255.0 / 2.0
            };
        }
    }

    for ( auto &submesh: vSubmeshList) {
        submesh.DrawSubmesh(framebuffer, zbuffer);
    }

    return;
}