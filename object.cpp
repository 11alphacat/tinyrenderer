#include "object.h"
#include <iostream>

bool Object::Initialize(const std::vector<std::string> &vsFilenameList)
{
    for (const auto &filename : vsFilenameList) {
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
    constexpr X::XMVECTOR  Light {  1,  1, 1, 0 }; // light direction
    constexpr X::XMVECTOR    Eye {  1,  1, 3, 1 }; // camera position
    constexpr X::XMVECTOR Center {  0,  0, 0, 1 };  // camera direction
    constexpr X::XMVECTOR     Up {  0,  1, 0, 0 };  // camera up vector
    X::XMVECTOR LightN = X::XMVector3Normalize(Light);

    /*
            v.x += 1.0;     v.y += 1.0;     v.z += 1.0;
            v.x *= (width >> 1);
            v.y *= (height >> 1);
            v.z *= (255.0 / 2); z ∈ [0, 255]
    */
    // 1. model to world(shift and scale)
    const X::XMMATRIX Model = X::XMMatrixTranslation(1.0, 1.0, 1.0) 
                                        * X::XMMatrixScaling(width >> 1, height >> 1, 255.0 / 2);
    
    // 2. world to camera view
    const X::XMMATRIX LookatRH = X::XMMatrixLookAtRH(Eye, Center, Up);
    const X::XMMATRIX ModelView = Model * LookatRH;
    const X::XMMATRIX ModelViewInvT = X::XMMatrixTranspose(X::XMMatrixInverse(nullptr, ModelView));

    // 3. Projection transform (STILL HAS PROBLEMS)
    // const X::XMMATRIX PerpRH = X::XMMatrixPerspectiveRH(
    //     static_cast<float>(width), 
    //     static_cast<float>(height),
    //     1.0f, // NearZ
    //     256.0f // FarZ
    // );
    float f = X::XMVectorGetX(X::XMVector3Length(Eye-Center));
    const X::XMMATRIX PerpRH = {
        1,  0,  0,  0, 
        0,  1, 0,  0, 
        0,  0,  1,  0, 
        0,  0,  -1.0f/f,  1
    };

    const X::XMMATRIX OrthRH = X::XMMatrixOrthographicRH(
        static_cast<float>(width), 
        static_cast<float>(height),
        1.0f, // NearZ
        256.0f // FarZ
    );
    
    // Finally. put them all together
    const X::XMMATRIX ModelViewProj = ModelView * PerpRH;

    X::XMVECTOR V {};
    X::XMVECTOR N {};
    float intensity { .0 };
    for (auto &submesh: vSubmeshList) {
        X::XMStoreFloat3( &(submesh.lightN), LightN);
        // part of vertex shader
        for ( auto &v : submesh.vertices ) {
            V = X::XMLoadFloat4(&v);
        
            // V = X::XMVector3Transform(V,ModelViewProj);
            V = X::XMVector3Transform(V,ModelViewProj);

            V /= X::XMVectorSplatW(V); 
            X::XMStoreFloat4(&v,V);  
        }

#if defined(NO_NORMAL_MAP)
        // compute vertext intensity
        for ( const auto &n : submesh.normals ) {
            N = X::XMLoadFloat4(&n);
            // N = X::XMVector3Transform(N, ModelViewInvT);
            intensity = X::XMVectorGetX(X::XMVector3Dot(N, LightN));
            intensity = intensity < 0 ? 0 : intensity;
            submesh.intensities.push_back(intensity);
        }
#endif

    }

    for ( auto &submesh: vSubmeshList ) {
        submesh.DrawSubmesh(framebuffer, zbuffer);
    }

    return;
}