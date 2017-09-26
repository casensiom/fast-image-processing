#ifndef _CANNY_H_
#define _CANNY_H_


// https://github.com/brunokeymolen/canny/blob/master/canny.cpp
// https://rosettacode.org/wiki/Canny_edge_detector#C

#include "types.h"

void init_canny(const uint32 _width, const uint32 _height);
void release_canny();
void canny(uint8 *_pData, uint32 _width, uint32 _height, const uint32 _tmin, const uint32 _tmax);

#endif
