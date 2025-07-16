#include <iostream>
#include "mesh.h"
#include "common.h"

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

    double x,y,z;
    while (in >> start && start != "#" ) {
        in >> x >> y >> z;
        vertices.emplace_back(x, y, z);
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

    double u,v,w;
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

    double nx,ny,nz;
    while ( in >> start && start != "#") {
        in >> nx >> ny >> nz;
        normals.emplace_back(nx, ny, nz);
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


void Submesh::DrawSubmesh(TGAImage &framebuffer, TGAImage &zbuffer) const
{
// std::cout << "Drawing submesh: " << sFilename << std::endl;
// std::cout << "Number of vertices: " << vertices.size() << std::endl;
// std::cout << "Number of indices: " << indicesv.size() << std::endl;
    for (size_t i = 0; i < indicesv.size(); i += 3) {

#ifdef WIREFRAME 
        // warning: use indicesv[i] rather than i as index!
        comGL::line(vertices[indicesv[i]].xy(),vertices[indicesv[i+1]].xy(), framebuffer, red);
        comGL::line(vertices[indicesv[i+1]].xy(),vertices[indicesv[i+2]].xy(), framebuffer, red);
        comGL::line(vertices[indicesv[i+2]].xy(), vertices[indicesv[i]].xy(), framebuffer, red);

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