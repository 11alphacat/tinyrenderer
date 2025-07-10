#include "tgaimage.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>

static constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
static constexpr TGAColor green   = {  0, 255,   0, 255};
static constexpr TGAColor red     = {  0,   0, 255, 255};
static constexpr TGAColor blue    = {255, 128,  64, 255};
static constexpr TGAColor yellow  = {  0, 200, 255, 255};

namespace drawobj {
    struct Vertex {
        float x,y,z;    // position
        Vertex(float x, float y, float z) : x(x), y(y), z(z) {}
    };
    
    struct Texture {
        float u,v,w;      // texture coordinates
        Texture(float u, float v, float w) : u(u), v(v), w(w) {}
    };

    struct Normal {
        float nx,ny,nz;  
        Normal(float nx, float ny, float nz) : nx(nx), ny(ny), nz(nz) {}
    };

    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<Texture> textures;
        std::vector<Normal> normals;
        std::vector<uint> indicesv;
        std::vector<uint> indicest;
        std::vector<uint> indicesn;
    };
};

static inline void Line(int ax, int ay, int bx, int by, TGAImage &framebuffer, const TGAColor& color) {
    bool steep(false);
    if (std::abs(by-ay) > std::abs(bx-ax)) {
        std::swap(ax, ay);
        std::swap(bx, by);
        steep = true;
    }

    if (ax > bx) {          // make it left−to−right 
        std::swap(ax, bx);
        std::swap(ay, by);
    }   

    // watch this video to truly understand bresenham's algorithm ==> https://www.youtube.com/watch?v=CceepU1vIKo
    /*  Bresenham’s line-drawing algorithm
        py = y0 + k*(i+1), where k = dy/dx
        d0 = py - y; d1 = (y+1) - py
        if d1 < d0: y++
        DEFINE p = d1 - d0 = ... = 2y - 2y0 - 2k(i+1) + 1
        then p*dx = ... = 2(y-y0)dx - 2*i*dy - 2dy + dx, when p*dx < 0, y needs update
        that is: if 2dy - dx > 2(y-y0)dx - 2dy*i, y needs update
        DEFINE D and initialize it as D = 2*dy - dx
        Because y initialized as y0, i initialized as 0
        there is : 
            if ( D > 0) {
                y++; // now ( y - y0) != 0, updates D
                D -= 2dx;
            }      
            D += 2dy; // i updates so D updates
        
    */
    int y(ay);
    int desicion(2 * std::abs(by-ay) - (bx-ax));        // D := 2*dy - dx
    for ( int x = ax; x != bx; ++x ) {
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
        if (desicion > 0 ) {
            y += by > ay ? 1 : -1;                      // y++
            desicion -= 2 * (bx - ax);                  // D := D - 2*dx
        }
        desicion += 2 * std::abs(by-ay);                // D := D + 2*dy 
    }

    return;
}

static int ConstructMeshFromFile(const std::string &filename, drawobj::MeshData &mesh) {
    int ok = 1;

    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        ok = 0;
        in.close();
        return ok;
    }
    
    std::string start,end;

    float x,y,z;
    while (in >> start && start != "#" ) {
        in >> x >> y >> z;
        mesh.vertices.emplace_back(x, y, z);
    }
    uint numVertices;
    in >> numVertices >> end;

    // Check Vertice number
    if (numVertices != mesh.vertices.size()) {
        std::cerr << "Number of vertices does not match!" << std::endl;
        ok = 0;
        in.close();
        return ok;
    }

    float u,v,w;
    while ( in >> start && start != "#") {
        in >> u >> v >> w;
        mesh.textures.emplace_back(u, v, w);
    }
    uint numTextures;
    in >> numTextures >> end >> end;

    // Check Texture number
    if (numTextures != mesh.textures.size()) {
        std::cerr << "Number of textures does not match!" << std::endl;
        ok = 0;
        in.close();
        return ok;
    }

    float nx,ny,nz;
    while ( in >> start && start != "#") {
        in >> nx >> ny >> nz;
        mesh.normals.emplace_back(nx, ny, nz);
    }

    uint numNormals;
    in >> numNormals >> end >> end;

    // Check Normal number
    if (numNormals != mesh.normals.size()) {
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
            mesh.indicesv.push_back(vi);
            mesh.indicest.push_back(ti);
            mesh.indicesn.push_back(ni);
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
        std::cout << "mesh.indicesv.size(): " << mesh.indicesv.size() << std::endl;
        std::cout << "mesh.indicest.size(): " << mesh.indicest.size() << std::endl;
        std::cout << "mesh.indicesn.size(): " << mesh.indicesn.size() << std::endl;
#endif
        ok = 0;
        in.close();
        return ok;
    }

    in.close();

    return ok;
}

extern void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color);
static void DrawMesh(const drawobj::MeshData &mesh, TGAImage &zbuffer, TGAImage &framebuffer) {
    const int width  = framebuffer.width();
    const int height = framebuffer.height();
    for (size_t i = 0; i < mesh.indicesv.size(); i += 3) {
        int ax = (mesh.vertices[mesh.indicesv[i]-1].x + 1.) * width / 2;
        int ay = (mesh.vertices[mesh.indicesv[i]-1].y + 1.) * height / 2; 
        int az = (mesh.vertices[mesh.indicesv[i]-1].z + 1.) * 255. / 2; 

        int bx = (mesh.vertices[mesh.indicesv[i+1]-1].x + 1.) * width / 2;
        int by = (mesh.vertices[mesh.indicesv[i+1]-1].y + 1.) * height / 2;
        int bz = (mesh.vertices[mesh.indicesv[i+1]-1].z + 1.) * 255. / 2; 

        int cx = (mesh.vertices[mesh.indicesv[i+2]-1].x + 1.) * width / 2;
        int cy = (mesh.vertices[mesh.indicesv[i+2]-1].y + 1.) * height / 2;
        int cz = (mesh.vertices[mesh.indicesv[i+2]-1].z + 1.) * 255. / 2; 

        // Line(ax, ay, bx, by, framebuffer, red);
        // Line(bx, by, cx, cy, framebuffer, red);
        // Line(cx, cy, ax, ay, framebuffer, red);
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, {
            static_cast<uint8_t>(std::rand()%256), 
            static_cast<uint8_t>(std::rand()%256), 
            static_cast<uint8_t>(std::rand()%256), 
            255}
        );
    }
    return;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << "obj/model.obj" << std::endl;
        return 1;
    }
    
    // const std::string filename("./obj/diablo3_pose/diablo3_pose.obj");
    const std::string filename(argv[1]);
    
    drawobj::MeshData meshDiablo;
    if (!ConstructMeshFromFile(filename, meshDiablo) ) {
        std::cerr << "Failed to load mesh!" << std::endl;
        return -1;
    }

    constexpr int width  = 1024;
    constexpr int height = 1024;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);   // all zero

    DrawMesh(meshDiablo, zbuffer, framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");

    return 0;
}


// 