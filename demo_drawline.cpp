#include <cmath>
#include <random>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
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

#ifdef OP_2
// 2.28s 2.28s 2.33s
// ENTIRELY optimize float to int
// because framebuffer.set() accecpts int
// It turns out we have arrived at Bresenham’s line-drawing algorithm
// Today, integer operations are not always more efficient than floating-point calculations — it depends on the context
    const float k = (by-ay) / static_cast<float>(bx-ax);
    int y(ay);
    int ierror(0);
    for ( int x = ax; x != bx; ++x ) {
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
        ierror += 2* std::abs(by-ay);
        if (ierror > (bx-ax)) {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx-ax);
        }
        /*
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx-ax)   * (ierror > bx - ax);
        */
    }
#undef OP_2
#endif

// #ifdef VERSION_4
// 2.64s 2.50s 2.48s
// optimize float to int
// because framebuffer.set() accecpts int
    const float k = (by-ay) / static_cast<float>(bx-ax);
    int y(ay);
    float error(0.);
    for ( int x = ax; x != bx; ++x ) {
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
        error += k;
        if (error > 0.5) {
            y += by > ay ? 1 : -1;
            error -= 1.;
        }
    }
// #undef VERSION_4
// #endif


#ifdef OP_1
// 2.34s 2.49s 2.64s
    float y = ay;
    for (int x=ax; x!=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        y += (by-ay) / static_cast<float>(bx-ax);   // actually complier is smart enough to see it's a constant
    }
#undef OP_1
#endif


#ifdef VERSION_3
// 2.35s 2.33s 2.32s
// y = ay + k* (x - ax)
// thus y0 = ay, y1 = y0 + k, y2 = y1 + k
    const float k = (by-ay) / static_cast<float>(bx-ax);
    float y(ay);
    for ( int x = ax; x != bx; ++x ) {
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
        y += k;
    }
#undef VERSION_3
#endif

#ifdef VERSION_2
// 3.05s 2.93s 2.94s
// y = ay + k* (x - ax)
    const float k = (by-ay) / static_cast<float>(bx-ax);
    for ( int x = ax, d = 0; x != bx; ++x,++d ) {
        int y = std::round( ay + k*d );
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
    }
#undef VERSION_2
#endif

#ifdef VERSION_1
// 3.52s 3.62s 3.67s
    for (int x = ax; x <= bx; ++x) {
        float t = (x-ax) / static_cast<float>(bx-ax);
        int y = std::round( ay + (by-ay)*t );
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
    }   
#undef VERSION_1
#endif

#ifdef VERSION_0
    for (float t=0.; t<1.; t+=.002) {
        int x = std::round( ax + (bx-ax)*t );
        int y = std::round( ay + (by-ay)*t );
        framebuffer.set(x, y, color);
    }
#undef VERSION_0
#endif

    return;
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    line(ax, ay, bx, by, framebuffer, blue);
    line(cx, cy, bx, by, framebuffer, green);
    line(cx, cy, ax, ay, framebuffer, yellow);
    line(ax, ay, cx, cy, framebuffer, red);

#ifdef PROFILE
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
{
    int ax,ay,bx,by;
    for (int i = 0; i != (1 << 24); ++i) {
        ax = rand() % width;
        ay = rand() % height;
        bx = rand() % width;
        by = rand() % height;
        line(ax, ay, bx, by, framebuffer, {
            static_cast<uint8_t>(dis(gen)),
            static_cast<uint8_t>(dis(gen)),
            static_cast<uint8_t>(dis(gen)),
            static_cast<uint8_t>(dis(gen))
        });
    }
}
#undef PROFILE
#endif

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}