#include <iostream>
#include <string>
#include "tgaimage.h"
#include "object.h"

static constexpr int width  = 1024;
static constexpr int height = 1024;

static TGAImage framebuffer(width, height, TGAImage::RGB);
static TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);   // all zero

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << "obj/model.obj" << std::endl;
        return 1;
    }
    
    /*
        e.g. "./obj/diablo3_pose/diablo3_pose.obj"
    */ 
    std::vector<std::string> vsFilenameList {argv+1, argv+argc};
    Object gameObj;
    if (!gameObj.Initialize(vsFilenameList)) {
        std::cerr << "Failed to initialize object from files!" << std::endl;
        return -1;
    }

    gameObj.Render(framebuffer, zbuffer);

    framebuffer.write_tga_file("framebuffer.tga");

    return 0;
}