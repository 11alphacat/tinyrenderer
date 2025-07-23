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
    constexpr X::XMVECTOR    Eye{-1,0,2,1}; // camera position
    constexpr X::XMVECTOR Center{0,0,0,1};  // camera direction
    constexpr X::XMVECTOR     Up{0,1,0,0};  // camera up vector
    
    static X::XMMATRIX Perp     = X::XMMatrixPerspectiveRH(width,height,1,2);
    static X::XMMATRIX LookatRH = X::XMMatrixLookAtRH(Eye,Center,Up);
    static X::XMMATRIX M = Perp * LookatRH;
    
    X::XMVECTOR V {};
    for (auto &submesh: vSubmeshList) {
        for (auto &v: submesh.vertices) {
            // Mention: temporary implementation, will use matrix for calculation later
            v.x += 1.0;     v.y += 1.0;     v.z += 1.0;
            v.x *= (width >> 1);
            v.y *= (height >> 1);
            v.z *= (255.0 / 2);

            // V = X::XMLoadFloat4(&v);
            // V = X::XMVector4Transform(V,M);
            // X::XMStoreFloat4(&v,V);
        }
    }

    for ( auto &submesh: vSubmeshList) {
        submesh.DrawSubmesh(framebuffer, zbuffer);
    }

    return;
}