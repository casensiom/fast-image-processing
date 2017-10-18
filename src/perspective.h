#ifndef _PERSPECTIVE_H_
#define _PERSPECTIVE_H_
#include "types.h"
#include "image.h"

// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.157.4621&rep=rep1&type=pdf
// https://github.com/4DA/simple-rasterizer/
// https://github.com/ssloy/tinyrenderer/

typedef struct STextureVector
{
    int x;
    int y;
    float u;
    float v;
}STextureVector;

typedef struct STextureTriangle
{
    STextureVector points[3];
    int det;
} STextureTriangle;

//--
STextureTriangle set_triangle(int _x1, int _y1, float _u1, float _v1, int _x2, int _y2, float _u2, float _v2, int _x3, int _y3, float _u3, float _v3);
STextureTriangle init_triangle(STextureTriangle tri, int _x1, int _y1, int _x2, int _y2, int _x3, int _y3);
STextureTriangle init_texture(STextureTriangle tri, float _u1, float _v1, float _u2, float _v2, float _u3, float _v3);

//--
void render_textured_triangle(const STextureTriangle *_pTri, const SImage *_pTexture, SImage *_pOutput);

#endif
