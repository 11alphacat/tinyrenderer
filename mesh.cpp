#include <iostream>
#include "mesh.h"
#include "XMath.h"

bool Submesh::BuildSubmesh() 
{
    int ok = 1;

    std::ifstream in { sFilename };
    if (!in) {
        std::cerr << "Cannot open file: " << sFilename << std::endl;
        ok = 0;
        in.close();
        return ok;
    }
    
    std::string start,end;

    float x,y,z;
    while (in >> start && start != "#" ) {
        in >> x >> y >> z;
        vertices.emplace_back(x, y, z,1);
    }
    uint numVertices;
    in >> numVertices >> end;

    // Check Vertice number
    if (numVertices != vertices.size()) {
        std::cerr << "Number of vertices does not match!" << std::endl;
        ok = 0;
        in.close();
        return ok;
    }

    float u,v,w;
    while ( in >> start && start != "#") {
        in >> u >> v >> w;
        textures.emplace_back(u, v, w);
    }
    uint numTextures;
    in >> numTextures >> end >> end;

    // Check Texture number
    if (numTextures != textures.size()) {
        std::cerr << "Number of textures does not match!" << std::endl;
        ok = 0;
        in.close();
        return ok;
    }

    float nx,ny,nz;
    while ( in >> start && start != "#") {
        in >> nx >> ny >> nz;
        normals.emplace_back(nx, ny, nz,0);
    }

    uint numNormals;
    in >> numNormals >> end >> end;

    // Check Normal number
    if (numNormals != normals.size()) {
        std::cerr << "Number of normals does not match!" << std::endl;
        ok = 0;
        in.close();
        return ok;
    }

    in >> start >> end;
    in >> start >> end;
    
    std::string vtn;
    uint vi,ti,ni;
    uint lines(0);
    while (in >> start && start != "#") {
        ++lines;
        for ( int _ = 0; _ != 3; ++_) {
            in >> vtn;
            sscanf(vtn.c_str(),"%d/%d/%d",&vi,&ti,&ni);
            indicesv.push_back(--vi);
            indicest.push_back(--ti);
            indicesn.push_back(--ni);
#ifdef DEBUG
            std::cout << "indicesv: " << vi << " ";
            std::cout << "indicest: " << ti << " ";
            std::cout << "indicesn: " << ni << std::endl;
#endif
        }
    }

    uint numFaces;
    in >> numFaces >> end;
    in >> end;

    // Check Face number
    if (numFaces != lines) {
        std::cerr << "Number of faces does not match!" << std::endl;

#ifdef DEBUG
        std::cout << "numFaces: " << numFaces << std::endl;
        std::cout << "lines: " << lines << std::endl;
        std::cout << "indicesv.size(): " << indicesv.size() << std::endl;
        std::cout << "indicest.size(): " << indicest.size() << std::endl;
        std::cout << "indicesn.size(): " << indicesn.size() << std::endl;
#endif
        ok = 0;
        in.close();
        return ok;
    }

    in.close();

    LoadTexture(sFilename, "_diffuse.tga", diffusemap);
    LoadTexture(sFilename, "_nm.tga",      normalmap);
    LoadTexture(sFilename, "_spec.tga",    specularmap);

    return ok;
}

void Submesh::LoadTexture(const std::string& filename, const char *suffix, TGAImage &img) 
{
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot!=std::string::npos) {
        texfile = texfile.substr(0,dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}

static constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
static constexpr TGAColor green   = {  0, 255,   0, 255};
static constexpr TGAColor red     = {  0,   0, 255, 255};
static constexpr TGAColor blue    = {255, 128,  64, 255};
static constexpr TGAColor yellow  = {  0, 200, 255, 255};

void Submesh::DrawSubmesh(TGAImage &framebuffer, TGAImage &zbuffer)
{
// std::cout << "Drawing submesh: " << sFilename << std::endl;
// std::cout << "Number of vertices: " << vertices.size() << std::endl;
// std::cout << "Number of indices: " << indicesv.size() << std::endl;
assert(indicesv.size() == indicesn.size());

    for (size_t i = 0; i < indicesv.size(); i += 3) {   // for each primitive
#if defined(WIREFRAME) 
        // warning: use indicesv[i] rather than i as index!
        GL::line(vertices[indicesv[i]],vertices[indicesv[i+1]], framebuffer, red);
        GL::line(vertices[indicesv[i+1]],vertices[indicesv[i+2]], framebuffer, red);
        GL::line(vertices[indicesv[i+2]], vertices[indicesv[i]], framebuffer, red);

#elif defined(RANDOM_COLOR)
        GL::triangle( 
            vertices[indicesv[i]],
            vertices[indicesv[i+1]],
            vertices[indicesv[i+2]], 
            zbuffer, framebuffer, 
            {   static_cast<uint8_t>(std::rand()%256), 
                static_cast<uint8_t>(std::rand()%256), 
                static_cast<uint8_t>(std::rand()%256), 
                255 
            }
        );

#elif defined(CUSTOMED_COLOR)
        float intensityA  = intensities[indicesn[i]];
        float intensityB  = intensities[indicesn[i+1]];
        float intensityC  = intensities[indicesn[i+2]];

        TGAColor lightColor = white;
        TGAColor colorA = {
            static_cast<uint8_t>(intensityA * lightColor[0]),
            static_cast<uint8_t>(intensityA * lightColor[1]),
            static_cast<uint8_t>(intensityA * lightColor[2]),
            255
        };
        TGAColor colorB = {
            static_cast<uint8_t>(intensityB * lightColor[0]),
            static_cast<uint8_t>(intensityB * lightColor[1]),
            static_cast<uint8_t>(intensityB * lightColor[2]),
            255
        };
        TGAColor colorC = {
            static_cast<uint8_t>(intensityC * lightColor[0]),
            static_cast<uint8_t>(intensityC * lightColor[1]),   
            static_cast<uint8_t>(intensityC * lightColor[2]),
            255
        };

        GL::triangle3color(
            vertices[indicesv[i]],
            vertices[indicesv[i+1]],
            vertices[indicesv[i+2]],
            zbuffer, framebuffer, 
        );
        
#else
        // default: fragment shader
        va_a = vertices[indicesv[i]];
        va_b = vertices[indicesv[i+1]];
        va_c = vertices[indicesv[i+2]];

        X::XMFLOAT3& ta = textures[indicest[i]];
        X::XMFLOAT3& tb = textures[indicest[i+1]];
        X::XMFLOAT3& tc = textures[indicest[i+2]];
        va_tex = X::XMFLOAT3X3 {
            ta.x, ta.y, ta.z,
            tb.x, tb.y, tb.z,
            tc.x, tc.y, tc.z
        };

#if defined(NO_NORMAL_MAP)
        va_intensity = X::XMFLOAT3 {
            intensities[indicesn[i]],
            intensities[indicesn[i+1]],
            intensities[indicesn[i+2]]
        };
#endif
        DrawTriangle(framebuffer, zbuffer);

#endif
    } // End of for loop
    
    return;
}

void inline Submesh::DrawTriangle(TGAImage &framebuffer, TGAImage &zbuffer) const
{
    int ax { static_cast<int>(va_a.x) }, ay { static_cast<int>(va_a.y) }, az { static_cast<int>(va_a.z) };
    int bx { static_cast<int>(va_b.x) }, by { static_cast<int>(va_b.y) }, bz { static_cast<int>(va_b.z) };
    int cx { static_cast<int>(va_c.x) }, cy { static_cast<int>(va_c.y) }, cz { static_cast<int>(va_c.z) };

    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle   
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    float total_area = GL::signed_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

    // #pragma omp parallel for 
    // can't use the above instruction !!! very slow!!
    // zbuffer.set(x, y, {z}); framebuffer.set(x, y, color) ==> multi-thread race !!
    TGAColor color;
    X::XMVECTOR bar;
    for (int x=bbminx; x!=bbmaxx; ++x) {
        for (int y=bbminy; y!=bbmaxy; ++y) {
            float alpha = GL::signed_area(x, y, bx, by, cx, cy) / total_area;
            float beta  = GL::signed_area(x, y, cx, cy, ax, ay) / total_area;
            float gamma = GL::signed_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);

            // depth test
            if (z < zbuffer.get(x, y)[0]) { // it seems that .obj file is in RH coordinate system
                continue; 
            }
            zbuffer.set(x, y, {z});

            bar = X::XMVectorSet(alpha, beta, gamma, 0);
            if ( !fragment( bar, color ) ) {
                framebuffer.set(x, y, color);
            }
        }
    }

    return;    
}

/*
* return pixel color based on barycentric coordinates and texture mapping  
*/
bool Submesh::fragment(X::FXMVECTOR bar, TGAColor& color) const
{
    // interpolate texture coordinates
    X::XMVECTOR Tex = X::XMVector3Transform(bar, X::XMLoadFloat3x3(&va_tex));
    float tx = X::XMVectorGetX(Tex), ty = X::XMVectorGetY(Tex);

#if defined(NO_NORMAL_MAP)
    // interpolate intensity (when normal map is not used)
    X::XMVECTOR Intensity = X::XMLoadFloat3(&va_intensity);
    float intensity = X::XMVectorGetX( X::XMVector3Dot(bar, Intensity) );

#else
    TGAColor normal = normalmap.get( static_cast<int>(tx * normalmap.width()),
            static_cast<int>(ty * normalmap.height()));
    X::XMVECTOR N = X::XMVectorSet(normal[2], normal[1], normal[0], 0);
    N = X::XMVectorDivide( X::XMVectorSubtract( N, X::XMVectorSet(128, 128, 128, 0) ), X::XMVectorSet(128, 128, 128, 1) );
    float intensity = X::XMVectorGetX( X::XMVector3Dot( N, X::XMLoadFloat3(&lightN) ) );
    intensity = intensity < 0 ? 0 : intensity;
    
#endif
    color = diffusemap.get( static_cast<int>(tx * diffusemap.width()), 
            static_cast<int>(ty * diffusemap.height()) );

    color[0] *= intensity;
    color[1] *= intensity;
    color[2] *= intensity;

    return false;
}