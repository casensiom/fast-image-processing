#ifndef _IMAGE_H_
#define _IMAGE_H_
#include "types.h"

typedef enum EPixelFormat {
    PF_UNKNOWN = 0,
    PF_ARGB = 4,
    PF_RGB = 3,
    PF_GRAY = 1
}EPixelFormat;

typedef struct SImage {
    uint32 mWidth;
    uint32 mHeight;
    uint32 mLeap;
    EPixelFormat mBpp;
    uint8 *mpData;
}SImage;


SImage create_image(uint32 _width, uint32 _height, EPixelFormat _format);
void reset_image(SImage *_pImg);
void release_image(SImage *_pImg);
void copy_image(SImage *_pImg, SImage *_pCopy);
SImage convertToGray(const SImage _source);
SImage convertToARGB(const SImage _source);
void dump(const SImage _source);
void draw_line(SImage *_pImage, uint32 color, int x0, int y0, int x1, int y1);

#endif
