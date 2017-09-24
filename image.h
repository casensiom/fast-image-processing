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
SImage convertToGray(const SImage _source);
void dump(const SImage _source);

#endif
