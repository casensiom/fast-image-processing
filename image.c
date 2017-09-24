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
SImage copy_image(SImage *_pImg)
{
    SImage dst;
    dst = create_image(_pImg->mWidth, _pImg->mHeight, _pImg->mBpp);

    uint32 size = _pImg->mWidth * _pImg->mHeight * _pImg->mBpp;
    memcpy(dst.mpData, _pImg->mpData, size);
    return dst;
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

