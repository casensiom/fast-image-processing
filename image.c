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
    uint8 r, g, b;
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
    printf("\n\nImage %d x %d (%d bpp)\n", _source.mWidth, _source.mHeight, _source.mBpp);
    for (int y = 0; y < _source.mHeight; ++y)
    {
        for (int x = 0; x < _source.mWidth; ++x)
        {
            if(_source.mBpp == 4)
            {
                printf(" %08X", ((uint32 *)_source.mpData)[x + y * _source.mWidth]);
            } else {
                printf(" %02X", _source.mpData[x + y * _source.mWidth]);
            }
        }
        printf("\n");
    }
}



void draw_line_4bpp(SImage *_pImage, uint32 color, int x0, int y0, int x1, int y1)
{
    uint8 a = ((color>>24) & 0xFF);
    uint8 r = ((color>>16) & 0xFF);
    uint8 g = ((color>>8) & 0xFF);
    uint8 b = ((color) & 0xFF);
    uint32 pos;
    int dx, dy, p, x, y;
    dx=x1-x0;
    dy=y1-y0;
    x=x0;
    y=y0;
    p=2*dy-dx;
 
    while(x<x1)
    {
        if( x >= 0 && x < _pImage->mWidth && y >= 0 && y < _pImage->mHeight)
        {
            pos = x + y * _pImage->mWidth;
            ((uint32 *)_pImage->mpData)[pos] = color;
        }
        p += 2*dy;
        if(p>=0)
        {
            ++y;
            p -= 2*dx;
        }
        ++x;
    }
}


void draw_line_3bpp(SImage *_pImage, uint32 color, int x0, int y0, int x1, int y1)
{
    uint8 r = ((color>>16) & 0xFF);
    uint8 g = ((color>>8) & 0xFF);
    uint8 b = ((color) & 0xFF);
    uint32 pos;
    int dx, dy, p, x, y;
    dx=x1-x0;
    dy=y1-y0;
    x=x0;
    y=y0;
    p=2*dy-dx;
 
    while(x<x1)
    {
        if( x >= 0 && x < _pImage->mWidth && y >= 0 && y < _pImage->mHeight)
        {
            pos = x*3 + y * _pImage->mWidth*3;
            _pImage->mpData[pos + 0] = r;
            _pImage->mpData[pos + 1] = g;
            _pImage->mpData[pos + 2] = b;
        }
        p += 2*dy;
        if(p>=0)
        {
            ++y;
            p -= 2*dx;
        }
        ++x;
    }
}


void draw_line_1bpp(SImage *_pImage, uint32 color, int x0, int y0, int x1, int y1)
{
    uint8 col = (0.298f * (float)((color>>16) & 0xFF) + 0.586f * (float)((color>>8) & 0xFF) + 0.114f * (float)((color) & 0xFF));
    int dx, dy, p, x, y;
    dx=x1-x0;
    dy=y1-y0;
    x=x0;
    y=y0;
    p=2*dy-dx;
 
    while(x<x1)
    {
        if( x >= 0 && x < _pImage->mWidth && y >= 0 && y < _pImage->mHeight)
            _pImage->mpData[x + y * _pImage->mWidth] = col;
        p += 2*dy;
        if(p>=0)
        {
            ++y;
            p -= 2*dx;
        }
        ++x;
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
