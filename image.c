#include "image.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//-------------------------------------
SImage create_image(uint32 _width, uint32 _height, EPixelFormat _format)
{
    SImage img;
    reset_image(&img);

    if(_width * _height > 0 && _format != PF_UNKNOWN)
    {
        img.mpData = (uint8 *)malloc(_width * _height * _format);
        if(img.mpData != 0x0)
        {
            img.mWidth = _width;
            img.mHeight = _height;
            img.mLeap = 0;
            img.mBpp = _format;
        }
    }
    return img;
}

//-------------------------------------
void reset_image(SImage *_pImg)
{
    _pImg->mWidth = 0;
    _pImg->mHeight = 0;
    _pImg->mLeap = 0;
    _pImg->mBpp = PF_UNKNOWN;
    _pImg->mpData = 0x0;
}

//-------------------------------------
void release_image(SImage *_pImg)
{
    if(_pImg->mpData != 0x0)
    {
        free(_pImg->mpData);
    }
    reset_image(_pImg);
}
//-------------------------------------
void copy_image(SImage *_pImg, SImage *_pCopy)
{
    if(_pCopy == 0x0)
        return;

    if(_pCopy->mWidth != _pImg->mWidth || _pCopy->mHeight != _pImg->mHeight || _pCopy->mBpp != _pImg->mBpp )
    {
        release_image(_pCopy);
        *_pCopy = create_image(_pImg->mWidth, _pImg->mHeight, _pImg->mBpp);
    }

    uint32 size = _pImg->mWidth * _pImg->mHeight * _pImg->mBpp;
    memcpy(_pCopy->mpData, _pImg->mpData, size);
}

//-------------------------------------
SImage convertToGray(const SImage _source)
{
    SImage pDst;
    uint8 *pData = (uint8 *)_source.mpData;

    uint32 i, size = _source.mWidth * _source.mHeight;
    uint8 r, g, b;
    reset_image(&pDst);
    pDst = create_image(_source.mWidth, _source.mHeight, PF_GRAY);
    if(_source.mBpp == PF_GRAY)
    {
        memcpy(pDst.mpData, pData, size);
    }
    else 
    {
        if(_source.mBpp == PF_ARGB)
        {
            for(i = 0; i < size; ++i)
            {
                uint8 b = pData[0];
                uint8 g = pData[1];
                uint8 r = pData[2];
                uint8 a = pData[3];
                pDst.mpData[i] = (0.298f * (float)r + 0.586f * (float)g + 0.114f * (float)b);
                pData += 4;
            }
        }
        else if(_source.mBpp == PF_RGB)
        {
            for(i = 0; i < size; ++i)
            {
                uint8 r = pData[0];
                uint8 g = pData[1];
                uint8 b = pData[2];
                pDst.mpData[i] = (0.298f * (float)r + 0.586f * (float)g + 0.114f * (float)b);
                pData += 3;
            }
        }
        else 
        {
            // ERROR !!!!
        }
    }
    return pDst;
}

//-------------------------------------
SImage convertToARGB(const SImage _source)
{
    SImage pDst;
    uint8 *pData = (uint8 *)_source.mpData;

    uint32 i, size = _source.mWidth * _source.mHeight;
    //uint8 r, g, b;
    reset_image(&pDst);
    pDst = create_image(_source.mWidth, _source.mHeight, PF_ARGB);
    if(_source.mBpp == PF_GRAY)
    {
        uint32 *pDataOut = (uint32 *)pDst.mpData;
        for(i = 0; i < size; ++i)
        {
            uint32 g = *pData;
            uint32 color = (g << 24) | (g << 16) | (g<<8);
            pDataOut[i] = color;
            ++pData;
        }
    }
    else 
    {
        if(_source.mBpp == PF_ARGB)
        {
            memcpy(pDst.mpData, pData, size*4);
        }
        else if(_source.mBpp == PF_RGB)
        {
            for(i = 0; i < size; ++i)
            {
                pDst.mpData[i*3 + 0] = *pData;
                ++pData;
                pDst.mpData[i*3 + 1] = *pData;
                ++pData;
                pDst.mpData[i*3 + 2] = *pData;
                ++pData;
            }
        }
        else 
        {
            // ERROR !!!!
        }
    }
    return pDst;
}


void dump(const SImage _source)
{
    dump_data(_source.mpData, _source.mWidth, _source.mHeight, _source.mBpp);
}

void dump_data(const uint8*_pData, const uint32 _width, const uint32 _height, const uint32 _bpp)
{
    printf("\n\nImage %d x %d (%d bpp)\n", _width, _height, _bpp);
    for (int y = 0; y < _height; ++y)
    {
        for (int x = 0; x < _width; ++x)
        {
            if(_bpp == 4)
            {
                printf(" %08X", ((uint32 *)_pData)[x + y * _width]);
            } else {
                printf(" %02X", _pData[x + y * _width]);
            }
        }
        printf("\n");
    }
}



void draw_line_4bpp(SImage *_pImage, uint32 color, int x0, int y0, int x1, int y1)
{
    uint32 pos;
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    int sx = x0<x1 ? 1 : -1;
    int sy = y0<y1 ? 1 : -1;
    int d = ((dx>dy) ? dx : -dy)/2;
    int p;

    while(x0!=x1 || y0!=y1)
    {
        if( x0 < 0 || x0 > _pImage->mWidth || y0 < 0 || y0 > _pImage->mHeight)
        {
            break;
        }
        pos = x0 + y0 * _pImage->mWidth;
        ((uint32 *)_pImage->mpData)[pos] = color;
        p = d;
        if (p >-dx) { d -= dy; x0 += sx; }
        if (p < dy) { d += dx; y0 += sy; }
    }
}


void draw_line_3bpp(SImage *_pImage, uint32 color, int x0, int y0, int x1, int y1)
{
    uint8 r = ((color>>16) & 0xFF);
    uint8 g = ((color>>8) & 0xFF);
    uint8 b = ((color) & 0xFF);
    uint32 pos;
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    int sx = x0<x1 ? 1 : -1;
    int sy = y0<y1 ? 1 : -1;
    int d = ((dx>dy) ? dx : -dy)/2;
    int p;

    while(x0!=x1 || y0!=y1)
    {
        if( x0 < 0 || x0 > _pImage->mWidth || y0 < 0 || y0 > _pImage->mHeight)
        {
            break;
        }
        pos = x0*3 + y0 * _pImage->mWidth*3;
        _pImage->mpData[pos + 0] = r;
        _pImage->mpData[pos + 1] = g;
        _pImage->mpData[pos + 2] = b;
        p = d;
        if (p >-dx) { d -= dy; x0 += sx; }
        if (p < dy) { d += dx; y0 += sy; }
    }
}


void draw_line_1bpp(SImage *_pImage, uint32 color, int x0, int y0, int x1, int y1)
{
    uint8 col = (0.298f * (float)((color>>16) & 0xFF) + 0.586f * (float)((color>>8) & 0xFF) + 0.114f * (float)((color) & 0xFF));
    uint32 pos;
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    int sx = x0<x1 ? 1 : -1;
    int sy = y0<y1 ? 1 : -1;
    int d = ((dx>dy) ? dx : -dy)/2;
    int p;

    while(x0!=x1 || y0!=y1)
    {
        if( x0 < 0 || x0 > _pImage->mWidth || y0 < 0 || y0 > _pImage->mHeight)
        {
            break;
        }
        pos = x0 + y0 * _pImage->mWidth;
        _pImage->mpData[pos] = col;
        p = d;
        if (p >-dx) { d -= dy; x0 += sx; }
        if (p < dy) { d += dx; y0 += sy; }
    }
}

void draw_line(SImage *_pImage, uint32 color, int x0, int y0, int x1, int y1)
{
    if(_pImage != 0x0 && _pImage->mpData != 0x0)
    {
        if(_pImage->mBpp == 4)
            draw_line_4bpp(_pImage, color, x0, y0, x1, y1);
        else if(_pImage->mBpp == 3)
            draw_line_3bpp(_pImage, color, x0, y0, x1, y1);
        else if(_pImage->mBpp == 1)
            draw_line_1bpp(_pImage, color, x0, y0, x1, y1);
    }
}
