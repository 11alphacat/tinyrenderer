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
        textures.emplace_back(u, v, w,0);
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

    return ok;
}


static constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
static constexpr TGAColor green   = {  0, 255,   0, 255};
static constexpr TGAColor red     = {  0,   0, 255, 255};
static constexpr TGAColor blue    = {255, 128,  64, 255};
static constexpr TGAColor yellow  = {  0, 200, 255, 255};

void Submesh::DrawSubmesh(TGAImage &framebuffer, TGAImage &zbuffer) const
{
// std::cout << "Drawing submesh: " << sFilename << std::endl;
// std::cout << "Number of vertices: " << vertices.size() << std::endl;
// std::cout << "Number of indices: " << indicesv.size() << std::endl;
    for (size_t i = 0; i < indicesv.size(); i += 3) {

#ifdef WIREFRAME 
        // warning: use indicesv[i] rather than i as index!
        GL::line(vertices[indicesv[i]],vertices[indicesv[i+1]], framebuffer, red);
        GL::line(vertices[indicesv[i+1]],vertices[indicesv[i+2]], framebuffer, red);
        GL::line(vertices[indicesv[i+2]], vertices[indicesv[i]], framebuffer, red);
        
#else
        comGL::triangle( 
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

#endif
    } // for loop
    
    return;
}