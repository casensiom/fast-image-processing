#ifndef _BMP_H_
#define _BMP_H_
#include "types.h"
#include "image.h"

typedef enum ELoadError
{
    LE_NO_ERROR,
    LE_CANT_OPEN,
    LE_CANT_READ,
    LE_CANT_WRITE,
    LE_INVALID_MAGIC,
} ELoadError;

uint32 load_image(const char *filename, SImage *pImage);
uint32 save_image(const char *filename, const SImage *_pImg);

#endif