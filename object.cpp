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
            // Mention: temporary implementation, will use matrix for calculation later
            // v += 1.0;
            X::XMVECTOR V = X::XMLoadFloat4(&v);
            V += X::XMVectorSet(1,1,1,0);
            // v *= comGL::vec3 {
            //     static_cast<double>(framebuffer.width())  / 2.0,
            //     static_cast<double>(framebuffer.height()) / 2.0,
            //     255.0 / 2.0
            // };
            V *= X::XMVectorSet(static_cast<float>(framebuffer.width() >> 1),
                                static_cast<float>(framebuffer.height() >> 1),
                                255.0 / 2.0,
                                1.0);
            X::XMStoreFloat4(&v,V);
        }
    }

    for ( auto &submesh: vSubmeshList) {
        submesh.DrawSubmesh(framebuffer, zbuffer);
    }

    return;
}