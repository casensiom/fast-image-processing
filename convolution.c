#include "convolution.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Take care, enable this define can create artifacts in the vertical borders on small images
#define IMPROVE_LOOPS

uint8 applyConvolution(uint8 *_pData, uint32 _width, uint32 _height, uint32 _x, uint32 _y, uint8 *_pKernel, uint8 _kernel_size, uint32 _kernel_sum);
uint8 decodeSegment(uint32 x, uint32 y);

//-------------------------------------
void getGradientsAndDirections(uint8 *_pData, uint32 _width, uint32 _height, double *_pGradient, uint8 *_pDirection) 
{

    const int32 Gx[] = {-1, 0, 1,
                        -2, 0, 2,
                        -1, 0, 1};
 
    const int32 Gy[] = { 1, 2, 1,
                         0, 0, 0,
                        -1,-2,-1};

    // apply sobel kernels and store gradients and directions
    int kernel_size = 3;
    int kernel_half = kernel_size >> 1;
    int leap = kernel_size - 1;
    int offset_xy = kernel_half;  // 3x3
    int src_pos = offset_xy + (offset_xy * _width);


#ifdef IMPROVE_LOOPS
    uint32 i, size = _height * _width - (_width * offset_xy * 2);
    for(i = 0; i < size; ++i)
    {
#else
    for (int y = offset_xy; y < _height - offset_xy; ++y) 
    {
        for (int x = offset_xy; x < _width - offset_xy; ++x) 
        {
#endif
            int32 convolve_X = 0.0;
            int32 convolve_Y = 0.0;

            int conv_pos = src_pos - offset_xy + (-offset_xy * _width);
            int k = 0;
            for (int ky = 0; ky < kernel_size; ky++) {
                for (int kx = 0; kx < kernel_size; kx++) {
                    convolve_X += _pData[conv_pos] * Gx[k];
                    convolve_Y += _pData[conv_pos] * Gy[k];
                    ++conv_pos;
                    ++k;
                }
                conv_pos += _width - kernel_size;
            }

            _pGradient[src_pos] = ((sqrt((convolve_X * convolve_X) + (convolve_Y * convolve_Y))));
            _pDirection[src_pos] = decodeSegment(convolve_X, convolve_Y);

            ++src_pos;
        }
#ifndef IMPROVE_LOOPS
        src_pos += leap;
    }
#endif
}

//-------------------------------------
void computeLocalMaxima(uint32 _width, uint32 _height, double *_pGradient, uint8 *_pDirection, double *_pMaxima)
{
    memcpy(_pMaxima, _pGradient, _width *_height * sizeof(double));

    // delete the points where de direcction and gradients does not fit
    int kernel_size = 3;
    int kernel_half = kernel_size >> 1;
    int leap = kernel_size - 1;
    int offset_xy = kernel_half;  // 3x3
    int pos = offset_xy + (offset_xy * _width);

    int32 lt = - (_width + 1);
    int32 tt = - _width;
    int32 rt = - (_width - 1);
    int32 ll = -1;
    int32 rr = 1;
    int32 lb = (_width - 1);
    int32 bb = _width;
    int32 rb = (_width + 1);
    double *pGrad = _pGradient + pos;
    double *pMax = _pMaxima + pos;
    uint8 *pDir = _pDirection + pos;

#ifdef IMPROVE_LOOPS
    uint32 i, size = _height * _width - (_width * offset_xy * 2);
    for (i = 0; i < size; ++i)
    {
#else

    for (int y = offset_xy; y < _height - offset_xy; ++y) 
    {
    for (int x = offset_xy; x < _width - offset_xy; ++x) 
    {
#endif
        if ((*pDir == 0 ) ||
            (*pDir == 1 && (*(pGrad + ll) >= *pGrad || *(pGrad + rr) >= *pGrad)) ||
            (*pDir == 2 && (*(pGrad + rt) >= *pGrad || *(pGrad + lb) >= *pGrad)) ||
            (*pDir == 3 && (*(pGrad + tt) >= *pGrad || *(pGrad + bb) >= *pGrad)) ||
            (*pDir == 4 && (*(pGrad + lt) >= *pGrad || *(pGrad + rb) >= *pGrad)))
        {
            *pMax = 0;
        }
        ++pGrad;
        ++pMax;
        ++pDir;
    }

#ifndef IMPROVE_LOOPS
        pos += leap;
        pGrad += leap;
        pMax += leap;
        pDir += leap;
    }
#endif
}

//-------------------------------------
uint8 applyGaussianBlur(uint8 *_pIn, uint8 *_pOut, uint32 _width, uint32 _height, uint32 _kernel_size)
{    

    const uint32 Gaus3x3Sum = 16;
    const uint8  Gaus3x3[] = { 1, 2, 1,    
                               2, 4, 2,   // * 1/16  
                               1, 2, 1}; 


    const uint32 Gaus5x5Sum = 159;
    const uint8  Gaus5x5[] = {  2,  4,  5,  4, 2,
                                4,  9, 12,  9, 4,
                                5, 12, 15, 12, 5, // * 1/159
                                4,  9, 12,  9, 4,
                                2,  4,  5,  4, 2 };

    uint8 *pSrc = _pIn;
    uint8 *pDst = _pOut;
    uint32 px;
    uint32 py;
    uint8 *pKernel;
    uint32 kernel_sum;
    if(_kernel_size == 5)
    {
        pKernel = (uint8 *)Gaus5x5;
        kernel_sum = Gaus5x5Sum;
    } else {
        _kernel_size = 3;
        pKernel = (uint8 *)Gaus3x3;
        kernel_sum = Gaus3x3Sum;
    }
    px = _kernel_size;
    py = _kernel_size;

    if(_kernel_size > _width || _kernel_size > _height)
    {
        return 0;
    }


    // Maybe is better to copy the full image in the destiantion, so we dont need to copy segements of memory
    // after apply the blur ¿?¿?¿?
    // memcpy(_pOut, _pIn, _width*_height*sizeof(uint8));

    uint32 maxWidth  = _width  - _kernel_size;
    uint32 maxHeight = _height - _kernel_size;
    pDst = _pOut + (_kernel_size + _kernel_size * _width);
    for(py = _kernel_size; py < maxHeight; ++py)
    {
        for(px = _kernel_size; px < maxWidth; ++px)
        {
            uint8 pixel = applyConvolution(_pIn, _width, _height, px, py, pKernel, _kernel_size, kernel_sum);
            *pDst = pixel;
            ++pDst;
        }
        pDst += _kernel_size-1;
    }
/*
    // Now we need to copy the unblurred pixels, all the kernel_size_half frame
    //
    //  ************
    //  ************
    //  **        **
    //  **        **
    //  ************
    //  ************
    //

    uint32 kernel_half = _kernel_size >> 1;
    pSrc = _pIn;
    pDst = _pOut;
    // full horizontal lines
    for(py = 0; py < kernel_half; ++py)
    {
        memcpy(pSrc, pDst, _width);
        pSrc += _width;
        pDst += _width;
    }

    //vertical parts
    memcpy(pSrc, pDst, kernel_half);
    pSrc += _width - kernel_half;
    pDst += _width - kernel_half;
    for(py = kernel_half; py < _height - kernel_half; ++py)
    {
        memcpy(pSrc, pDst, _kernel_size - 1);
        pSrc += _width;
        pDst += _width;
    }

    // full horizontal lines
    pSrc += kernel_half;
    pDst += kernel_half;
    for(py = _height - kernel_half; py < _height; ++py)
    {
        memcpy(pSrc, pDst, _width);
        pSrc += _width;
        pDst += _width;
    }
*/
    return 1;
}



//-------------------------------------
uint8 applyConvolution(uint8 *_pData, uint32 _width, uint32 _height, uint32 _x, uint32 _y, uint8 *_pKernel, uint8 _kernel_size, uint32 _kernel_sum) 
{

    // these security checks can be avoid if we only call this method from our GaussianBlur method
    // if(kernel_half > _width || kernel_half > _height)
    // {
    //     return 0;
    // }
    // if(_x < kernel_half || _y < kernel_half || _x > (_width - kernel_half) || _y > (_height - kernel_half))
    // {
    //     if(_x < _width || _y < _height)
    //         return _pData[_x + _y * _width];
    //     return 0;
    // }

    uint32 convolution = 0;
    const uint32 kernel_half = _kernel_size >> 1;
    const uint32 leap = _width - _kernel_size;
    _pData += (_x - kernel_half) + (_y - kernel_half) * _width; // initial offset
    for(int j = 0; j < _kernel_size; ++j)
    {
        for(int i = 0; i < _kernel_size; ++i)
        {
            convolution += *(_pData) * *(_pKernel);
            ++_pData;
            ++_pKernel;
        }
        _pData += leap;
    }
    return (uint8)((double)convolution / (double)_kernel_sum);
}

//-------------------------------------
uint8 decodeSegment(uint32 x, uint32 y)
{

    uint8 segment = 0;

    if (x != 0.0 && y != 0.0) 
    {
        double theta = atan2(x, y);  // radians. atan2 range: -PI,+PI,
                                      // theta : 0 - 2PI
//#define RANGE_NORMALIZE
#ifdef RANGE_NORMALIZE
        theta = (float)(fmod(theta + M_PI, M_PI) / M_PI) * 8;
        if (theta <= 1 || theta > 7) // 0 deg
            segment = 1;  // "-"
        else if (theta > 1 && theta <= 3) // 45 deg
            segment = 2;  // "/"
        else if (theta > 3 && theta <= 5) // 90 deg
            segment = 3;  // "|"
        else if (theta > 5 && theta <= 7) // 135 deg
            segment = 4;  // "\"
#else
        theta = theta * (180.0 / M_PI);  // degrees

        if ((theta <= 22.5 && theta >= -22.5) || (theta <= -157.5) || (theta >= 157.5))
            segment = 1;  // "-"
        else if ((theta > 22.5 && theta <= 67.5) || (theta > -157.5 && theta <= -112.5))
            segment = 2;  // "/"
        else if ((theta > 67.5 && theta <= 112.5) || (theta >= -112.5 && theta < -67.5))
            segment = 3;  // "|"
        else if ((theta >= -67.5 && theta < -22.5) || (theta > 112.5 && theta < 157.5))
            segment = 4;  // "\"
#endif
    }
    return segment;
}

