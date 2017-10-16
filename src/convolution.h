#ifndef _CONVOLUTION_H_
#define _CONVOLUTION_H_
#include "types.h"

void getGradientsAndDirections(uint8 *_pData, uint32 _width, uint32 _height, double *_pGradient, uint8 *_pDirection);
void computeLocalMaxima(uint32 _width, uint32 _height, double *_pGradient, uint8 *_pDirection, double *_pMaxima);
uint8 applyGaussianBlur(uint8 *_pIn, uint8 *_pOut, uint32 _width, uint32 _height, uint32 _kernel_size);

#endif
