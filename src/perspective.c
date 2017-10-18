#include "perspective.h"


//--
int check_side(int _X, int _Y, int _dX, int _dY, int _x, int _y);
void get_barycentric(const STextureTriangle *_pTri, int _x, int _y, double *_pL1, double *_pL2, double *_pL3);
void get_cartesian(const STextureTriangle *_pTri, double _l1, double _l2, double _l3, float *_pU, float *_pV);
uint32 get_pixel(const SImage *_pImg, float _x, float _y);
void set_pixel(SImage *_pImg, int _x, int _y, uint32 _color);
void write_pixel(SImage *_pOutput, const SImage *_pTexture, int _x, int _y, float _u, float _v);


//--
STextureTriangle set_triangle(int _x1, int _y1, float _u1, float _v1, int _x2, int _y2, float _u2, float _v2, int _x3, int _y3, float _u3, float _v3)
{
    STextureTriangle ret;
    ret = init_triangle(ret, _x1, _y1, _x2, _y2, _x3, _y3);
    ret = init_texture (ret, _u1, _v1, _u2, _v2, _u3, _v3);
    return ret;
}

//--
STextureTriangle init_triangle(STextureTriangle tri, int _x1, int _y1, int _x2, int _y2, int _x3, int _y3)
{
    STextureTriangle ret = tri;

    ret.points[0].x = _x1;
    ret.points[0].y = _y1;
    ret.points[1].x = _x2;
    ret.points[1].y = _y2;
    ret.points[2].x = _x3;
    ret.points[2].y = _y3;

    //compute determinant
    ret.det = (ret.points[1].y - ret.points[2].y) * (ret.points[0].x - ret.points[2].x) + (ret.points[2].x - ret.points[1].x) * (ret.points[0].y - ret.points[2].y);

    return ret;
}

//--
STextureTriangle init_texture(STextureTriangle tri, float _u1, float _v1, float _u2, float _v2, float _u3, float _v3)
{
    STextureTriangle ret = tri;

    ret.points[0].u = _u1;
    ret.points[0].v = _v1;
    ret.points[1].u = _u2;
    ret.points[1].v = _v2;
    ret.points[2].u = _u3;
    ret.points[2].v = _v3;

    return ret;
}

//-------------------------------------
// Returns a value related to the position of the point (x,y) relative to the edge defined by the points (X,Y) and (X+dX, Y+dY)
//  > 0 if (x,y) is to the "right" side
// == 0 if (x,y) is exactly on the line
//  < 0 if (x,y) is to the "left " side
int check_side(int _X, int _Y, int _dX, int _dY, int _x, int _y)
{
    return (_x - _X) * _dY - (_y - _Y) * _dX;
}

//-------------------------------------
// gets the barycentric coordinates given a cartesian coordinates.
void get_barycentric(const STextureTriangle *_pTri, int _x, int _y, double *_pL1, double *_pL2, double *_pL3)
{
    *_pL1 = ((_pTri->points[1].y - _pTri->points[2].y ) * ((double)_x - _pTri->points[2].x) + (_pTri->points[2].x - _pTri->points[1].x) * ((double)_y - _pTri->points[2].y)) / (double)_pTri->det;
    *_pL2 = ((_pTri->points[2].y - _pTri->points[0].y ) * ((double)_x - _pTri->points[2].x) + (_pTri->points[0].x - _pTri->points[2].x) * ((double)_y - _pTri->points[2].y)) / (double)_pTri->det;
    *_pL3 = (1.0 - *_pL1 - *_pL2);
}

//-------------------------------------
// gets the cartesian coordinates given a barycentric coordinates.
void get_cartesian(const STextureTriangle *_pTri, double _l1, double _l2, double _l3, float *_pU, float *_pV)
{
    *_pU = _l1 * _pTri->points[0].u + _l2 * _pTri->points[1].u + _l3 * _pTri->points[2].u;
    *_pV = _l1 * _pTri->points[0].v + _l2 * _pTri->points[1].v + _l3 * _pTri->points[2].v;
}

//-------------------------------------
// Gets the color of a (u,v) texture coordinates.
// Advice the params are floats!!
uint32 get_pixel(const SImage *_pImg, float _x, float _y)
{
    int tx = _pImg->mWidth * _x;
    int ty = _pImg->mHeight * _y;
    int bpp = _pImg->mBpp;
    int pos = (ty * _pImg->mWidth + tx);
    uint32 color = 0;

    if(bpp == 1)
    {
        color = _pImg->mpData[pos];
    }
    else if(bpp == 3)
    {
        pos *= bpp;
        uint32 r = _pImg->mpData[pos + 0];
        uint32 g = _pImg->mpData[pos + 1];
        uint32 b = _pImg->mpData[pos + 2];
        color = ((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF);
    }
    else if(bpp == 4)
    {
        color = ((uint32 *)_pImg->mpData)[pos];
    }
    return color;
}

//-------------------------------------
// Sets the color of a pixel
void set_pixel(SImage *_pImg, int _x, int _y, uint32 _color)
{
    int bpp = _pImg->mBpp;
    int pos = (_y * _pImg->mWidth + _x);

    if(bpp == 1)
    {
        _pImg->mpData[pos] = (uint8)(_color & 0xFF);
    }
    else if(bpp == 3)
    {
        uint8 r = (uint8)((_color >> 16) & 0xFF);
        uint8 g = (uint8)((_color >>  8) & 0xFF);
        uint8 b = (uint8)((_color      ) & 0xFF);
        pos *= bpp;
        _pImg->mpData[pos + 0] = r;
        _pImg->mpData[pos + 1] = g;
        _pImg->mpData[pos + 2] = b;
    }
    else if(bpp == 4)
    {
        ((uint32 *)_pImg->mpData)[pos] = _color;
    }
}

//-------------------------------------
// Writes a texture value into a destination image, on the specified position.
void write_pixel(SImage *_pOutput, const SImage *_pTexture, int _x, int _y, float _u, float _v)
{
    uint32 color = get_pixel(_pTexture, _u, _v);
    set_pixel(_pOutput, _x, _y, color);
}

//-------------------------------------
// Rendes a triangle with texture
void render_textured_triangle(const STextureTriangle *_pTri, const SImage *_pTexture, SImage *_pOutput)
{
    int   x, y;
    float u, v;
    double l1, l2, l3;
    int sx, sy, ex, ey;

    //get triangle bounding box
    sx = MIN(_pTri->points[0].x, MIN(_pTri->points[1].x, _pTri->points[2].x));
    sy = MIN(_pTri->points[0].y, MIN(_pTri->points[1].y, _pTri->points[2].y));
    ex = MAX(_pTri->points[0].x, MAX(_pTri->points[1].x, _pTri->points[2].x));
    ey = MAX(_pTri->points[0].y, MAX(_pTri->points[1].y, _pTri->points[2].y));

#define CHECK_SIDES
#ifdef CHECK_SIDES
    //E(x+1,y) = E(x,y) + dY
    //E(x,y+1) = E(x,y) - dX
    int sd1, sd2, sd3;
    // store the direction vectors
    int diffX_A2B = _pTri->points[1].x - _pTri->points[0].x;
    int diffX_B2C = _pTri->points[2].x - _pTri->points[1].x;
    int diffX_C2A = _pTri->points[0].x - _pTri->points[2].x;
    int diffY_A2B = _pTri->points[1].y - _pTri->points[0].y;
    int diffY_B2C = _pTri->points[2].y - _pTri->points[1].y;
    int diffY_C2A = _pTri->points[0].y - _pTri->points[2].y;

    for (y = sy; y < ey; ++y) {
        x   = sx;
        sd1 = check_side(_pTri->points[0].x, _pTri->points[0].y, diffX_A2B, diffY_A2B, x, y);
        sd2 = check_side(_pTri->points[1].x, _pTri->points[1].y, diffX_B2C, diffY_B2C, x, y);
        sd3 = check_side(_pTri->points[2].x, _pTri->points[2].y, diffX_C2A, diffY_C2A, x, y);

        for (; x < ex; ++x) {
            sd1 += diffY_A2B;
            sd2 += diffY_B2C;
            sd3 += diffY_C2A;

            //set_pixel(_pOutput, x, y, 0x20);    // just testing
            // check if we are inside the triangle
            if (sd1 >= 0 && sd2 >= 0 && sd3 >= 0) {
                get_barycentric(_pTri, x, y, &l1, &l2, &l3);
                get_cartesian(_pTri, l1, l2, l3, &u, &v);
                write_pixel(_pOutput, _pTexture, x, y, u, v);
            }
        }
    }
#else
    for (y = sy; y < ey; ++y) {
        for (x = sx; x < ex; ++x) {
            get_barycentric(_pTri, x, y, &l1, &l2, &l3);
            //set_pixel(_pOutput, x, y, 0x20);    // just testing
            // check if we are inside the triangle
            if (l1 >= 0 && l2 >= 0 && l3 >= 0)
            {
                get_cartesian(_pTri, l1, l2, l3, &u, &v);
                write_pixel(_pOutput, _pTexture, x, y, u, v);
            }
        }
    }
#endif
}

