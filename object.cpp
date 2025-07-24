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
    constexpr X::XMVECTOR    Eye { -1, 0, 2, 1 }; // camera position
    constexpr X::XMVECTOR Center {  0, 0, 0, 1 };  // camera direction
    constexpr X::XMVECTOR     Up {  0, 1, 0, 0 };  // camera up vector

    /*
            v.x += 1.0;     v.y += 1.0;     v.z += 1.0;
            v.x *= (width >> 1);
            v.y *= (height >> 1);
            v.z *= (255.0 / 2); z ∈ [0, 255]
    */
    // 1. model to world(shift and scale)
    const X::XMMATRIX ModelView = X::XMMatrixTranslation(1.0, 1.0, 1.0) 
                                        * X::XMMatrixScaling(width >> 1, height >> 1, 255.0 / 2); 

    // 2. world to camera view
    const X::XMMATRIX LookatRH = X::XMMatrixLookAtRH(Eye, Center, Up);

    // 3. Projection transform
    const X::XMMATRIX PerpRH = X::XMMatrixPerspectiveRH(
        static_cast<float>(width), 
        static_cast<float>(height),
        1.0f, // NearZ
        256.0f // FarZ
    );
    const X::XMMATRIX OrthRH = X::XMMatrixOrthographicRH(
        static_cast<float>(width), 
        static_cast<float>(height),
        1.0f, // NearZ
        256.0f // FarZ
    );
    
    
    // Finally. put them all together
    const X::XMMATRIX WorldViewProj = ModelView * LookatRH;

    X::XMVECTOR V {};
    for (auto &submesh: vSubmeshList) {
        for (auto &v: submesh.vertices) {
            V = X::XMLoadFloat4(&v);
        
            V = X::XMVector3Transform(V,WorldViewProj);

            V /= X::XMVectorSplatW(V); 
            X::XMStoreFloat4(&v,V);
        }
    }

    for ( auto &submesh: vSubmeshList) {
        submesh.DrawSubmesh(framebuffer, zbuffer);
    }

    return;
}