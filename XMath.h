#pragma once

// #define _XM_NO_INTRINSICS_

#include "DirectXMath/Inc/DirectXMath.h" 

#include "tgaimage.h"

namespace X = DirectX;

namespace GL 
{
    inline void line(const X::XMFLOAT4& from, const X::XMFLOAT4& to, TGAImage &framebuffer, const TGAColor& color) {
        int ax { static_cast<int>(from.x) },    ay { static_cast<int>(from.y) };
        int bx { static_cast<int>(to.x) },      by { static_cast<int>(to.y) };
        bool steep { false };  
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
        int y { ay };
        int desicion { 2 * std::abs(by-ay) - (bx-ax) };        // D := 2*dy - dx
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
    
    inline float signed_area(int ax, int ay, int bx, int by, int cx, int cy) {
        // return .5* ( (bx-ax)*(cy-ay) - (by-ay)*(cx-ax) );
        int bx_minus_ax { bx - ax };
        int cy_minus_ay { cy - ay };
        int by_minus_ay { by - ay };
        int cx_minux_ax { cx - ax };
        int _1 { bx_minus_ax * cy_minus_ay }, _2 { by_minus_ay * cx_minux_ax };
        return ( ( _1 - _2 ) >> 1 );
    }
    
    inline void triangle(const X::XMFLOAT4& a, const X::XMFLOAT4& b, const X::XMFLOAT4& c, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color) {
        int ax { static_cast<int>(a.x) }, ay { static_cast<int>(a.y) }, az { static_cast<int>(a.z) };
        int bx { static_cast<int>(b.x) }, by { static_cast<int>(b.y) }, bz { static_cast<int>(b.z) };
        int cx { static_cast<int>(c.x) }, cy { static_cast<int>(c.y) }, cz { static_cast<int>(c.z) };
    
        int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle   
        int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
        int bbmaxx = std::max(std::max(ax, bx), cx);
        int bbmaxy = std::max(std::max(ay, by), cy);
        double total_area = signed_area(ax, ay, bx, by, cx, cy);
        if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel
    
        // #pragma omp parallel for 
        // can't use the above instruction !!! very slow!!
        // zbuffer.set(x, y, {z}); framebuffer.set(x, y, color) ==> multi-thread race !!
        for (int x=bbminx; x!=bbmaxx; ++x) {
            for (int y=bbminy; y!=bbmaxy; ++y) {
                double alpha = signed_area(x, y, bx, by, cx, cy) / total_area;
                double beta  = signed_area(x, y, cx, cy, ax, ay) / total_area;
                double gamma = signed_area(x, y, ax, ay, bx, by) / total_area;
                if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
                unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
    
                // depth test
                if (z < zbuffer.get(x, y)[0]) { // it seems that .obj file is in RH coordinate system
                    continue; 
                }
                zbuffer.set(x, y, {z});
                framebuffer.set(x, y, color);
            }
        }
    
        return;
    }
    
}; // namespace GL