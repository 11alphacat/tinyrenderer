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

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    // if (signed_area(ax, ay, bx, by, cx, cy) <1) {
    //     return; // backface culling + discarding triangles that cover less than a pixel
    // }

    using Vector = drawtriangle::Vector;
    using drawtriangle::isLeftRightLH;

    int bottomy = std::min(ay, std::min(by,cy)), upy = std::max(ay, std::max(by,cy));
    int leftx = std::min(ax, std::min(bx,cx)), rightx = std::max(ax, std::max(bx,cx));
    
#ifdef DEBUG
std::cout << "leftx: " << leftx << " rightx: " << rightx << std::endl;
std::cout << "bottomy: " << bottomy << " upy: " << upy << std::endl;
#endif
    
    Vector AB(bx-ax, by-ay, 0), BC(cx-bx, cy-by, 0), CA(ax-cx, ay-cy, 0);

    Vector AP,BP,CP;
    for ( int py = bottomy; py != upy; ++py) {
        for ( int px = leftx; px != rightx; ++px) {
            AP = {px-ax, py-ay, 0};
            BP = {px-bx, py-by, 0};
            CP = {px-cx, py-cy, 0};
            
            if ( isLeftRightLH(AB,AP) == isLeftRightLH(BC,BP) &&
                 isLeftRightLH(BC,BP) == isLeftRightLH(CA,CP)) 
            {
                framebuffer.set(px, py, color);
            }
        }
    }
    
    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(cx, cy, ax, ay, framebuffer, color);
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
    triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
    triangle(115, 83, 80,  90, 85, 120, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

