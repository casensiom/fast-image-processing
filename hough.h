#ifndef _HOUGH_H_
#define _HOUGH_H_

#include "types.h"

// https://github.com/opencv/opencv/blob/master/modules/imgproc/src/hough.cpp

typedef struct SPolar
{
    float rho;
    float theta;
} SPolar;

typedef struct SCartesian
{
    int32 x;
    int32 y;
} SCartesian;

void init_hough(const uint32 _width, const uint32 _height);
void release_hough();
uint32 hough(uint8 *_pData, uint32 _width, uint32 _height, uint32 _threshold, SPolar *_lineBuffer, uint32 _size);

void save_hough_workspace(const char *filename);

#endif
