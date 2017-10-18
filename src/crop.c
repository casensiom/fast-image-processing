#include "perspective.h"
#include "image.h"
#include "bmp.h"

int main(int argc, char *argv[])
{

    uint32 side = 255;
    uint32 w = 255;
    uint32 h = 255;
    //STextureTriangle tri1 = set_triangle(0, 0, 0.05, 0.8,       0, h, 0.9, 0.1,      w, 0, 0.2, 0.05);
    //STextureTriangle tri2 = set_triangle(w, 0, 0.2, 0.05,       0, h, 0.9, 0.1,      w, h, 0.5, 0.0);
    STextureTriangle tri1 = set_triangle(0, 0, 0.5, 0.0,       0, h, 0.0, 0.5,      w, 0, 1.0, 0.5);
    STextureTriangle tri2 = set_triangle(w, 0, 1.0, 0.5,       0, h, 0.0, 0.5,      w, h, 0.5, 1.0);

    SImage imgOut, imgRGB;

    reset_image(&imgOut);
    reset_image(&imgRGB);

    // create XOR image
    imgRGB = create_image(side, side, PF_ARGB);
    for(uint32 y = 0; y < side; ++y)
    {
        for(uint32 x = 0; x < side; ++x)
        {
            //imgRGB.mpData[(x+y*side)] = (x % 255) ^ (y % 255);
            imgRGB.mpData[(x+y*side) * 4 + 0] = (x % 255) ^ (y % 255);
            imgRGB.mpData[(x+y*side) * 4 + 1] = (x % 255) ^ (y % 255);
            imgRGB.mpData[(x+y*side) * 4 + 2] = (x % 255) ^ (y % 255);
            imgRGB.mpData[(x+y*side) * 4 + 3] = (x % 255) ^ (y % 255);
        }
    }
    save_image("perspective_in.bmp", &imgRGB);

    // crop image with perspective
    imgOut = create_image(w, h, PF_ARGB);
    render_textured_triangle(&tri1, &imgRGB, &imgOut);
    render_textured_triangle(&tri2, &imgRGB, &imgOut);
    save_image("perspective.bmp", &imgOut);


    release_image(&imgRGB);
    release_image(&imgOut);
}
