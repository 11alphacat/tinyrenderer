#include <cmath>
#include <tuple>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

constexpr int width  = 128;
constexpr int height = 128;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

static inline void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, const TGAColor& color) {
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

namespace drawtriangle {
    struct Vector {
        int x,y,z;
        Vector(int x, int y, int z) : x(x), y(y), z(z) {}
        Vector(const Vector& v) : x(v.x), y(v.y), z(v.z) {}
        Vector() : x(0), y(0), z(0) {}

        Vector& operator=(const Vector& v) {
            x = v.x;
            y = v.y;
            z = v.z;
            return *this;
        }
        Vector operator+(const Vector& v) const {
            return Vector(x+v.x, y+v.y, z+v.z);
        }
        Vector operator-(const Vector& v) const {
            return Vector(x-v.x, y-v.y, z-v.z);
        }
        int operator*(const Vector& v) const {
            return x*v.x + y*v.y + z*v.z;
        }
    };

    bool isLeftRightLH(const Vector& lhs, const Vector& rhs) {
        /*
            after cross product, if z-component is posstive, then lhs is left of rhs
            because we use left hand coordinate system
        */
        return ((lhs.x * rhs.y - lhs.y*rhs.x ) > 0);
    }
};

static float signed_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5* ( (bx-ax)*(cy-ay) - (by-ay)*(cx-ax) );
}

void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_area(ax, ay, bx, by, cx, cy);
    // if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x!=bbmaxx; ++x) {
        for (int y=bbminy; y!=bbmaxy; ++y) {
            double alpha = signed_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);

            // depth test
            if (z < zbuffer.get(x, y)[0]) { // it seems that .obj file is in right hand coordinate system
                continue; 
            }
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }

    return;
}

// int main(int argc, char** argv) {
//     TGAImage framebuffer(width, height, TGAImage::RGB);
//     triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
//     triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
//     triangle(115, 83, 80,  90, 85, 120, framebuffer, green);
//     framebuffer.write_tga_file("framebuffer.tga");
//     return 0;
// }

