#include "canny.h"
#include "convolution.h"
#include <stdlib.h>
#include <string.h>

// https://github.com/brunokeymolen/canny/blob/master/canny.cpp
// https://rosettacode.org/wiki/Canny_edge_detector#C

#define MAX_BRIGHTNESS 255
#define TMP_BRIGHTNESS 100

//-------------------------------------
typedef struct SCannyWorkspace
{
    uint32 width;
    uint32 height;

    uint8  *pMap;
    double *pGradient;
    double *pMaxima;
    uint8  *pDirection;
} SCannyWorkspace;
//-------------------------------------

//-------------------------------------
static SCannyWorkspace *spCannyWorkspace = 0x0;
//-------------------------------------

// private methods
//-------------------------------------
void canny_priv(uint8 *_pData, const uint32 _width, const uint32 _height, const uint32 _tmin, const uint32 _tmax, SCannyWorkspace *_pWorkspace);
void traceEdgesWithHysteresis(uint8 *_pData, const uint32 _width, const uint32 _height, const uint32 _tmin, const uint32 _tmax, SCannyWorkspace *_pWorkspace);
SCannyWorkspace *create_workspace_canny(const uint32 _width, const uint32 _height);
void release_workspace_canny(SCannyWorkspace *_pWorkspace);
//-------------------------------------

//-------------------------------------
void
init_canny(const uint32 _width, const uint32 _height)
{
    if(spCannyWorkspace == 0x0)
    {
        spCannyWorkspace = create_workspace_canny(_width, _height);
    }
    else if(spCannyWorkspace->width != _width || spCannyWorkspace->height != _height)
    {
        // TODO: Alert the user that using canny in different image sizes can decrease the efficiency
        release_workspace_canny(spCannyWorkspace);
        spCannyWorkspace = create_workspace_canny(_width, _height);
    }
}

//-------------------------------------
void
realease_canny()
{
    release_workspace_canny(spCannyWorkspace);
}

//-------------------------------------
void
canny(uint8 *_pData, const uint32 _width, const uint32 _height, const uint32 _tmin, const uint32 _tmax)
{
    init_canny(_width, _height);
    canny_priv(_pData, _width, _height, _tmin, _tmax, spCannyWorkspace);
}

//-------------------------------------
void
canny_priv(uint8 *_pData, const uint32 _width, const uint32 _height, const uint32 _tmin, const uint32 _tmax, SCannyWorkspace *_pWorkspace)
{
    if(_pWorkspace == 0x0)
    {
        return;
    }

    // 1. Apply gaussian blur _img -> pMap
    applyGaussianBlur(_pData, _pWorkspace->pMap, _width, _height, 5);

    // 2. Compute gradients and directions
    getGradientsAndDirections(_pData, _width, _height, _pWorkspace->pGradient, _pWorkspace->pDirection);

    // 3. Compute local maxima
    computeLocalMaxima(_width, _height, _pWorkspace->pGradient, _pWorkspace->pDirection, _pWorkspace->pMaxima);

    // 4. Trace edges
    traceEdgesWithHysteresis(_pData, _width, _height, _tmin, _tmax, _pWorkspace);
}

//-------------------------------------
void
traceEdgesWithHysteresis(uint8 *_pData, const uint32 _width, const uint32 _height, const uint32 _tmin, const uint32 _tmax, SCannyWorkspace *_pWorkspace)
{
#if 0
    // Reuse array as a stack. width*height/2 elements should be enough.
    int32 *edges = (int32 *)_pWorkspace->pGradient;
    memset(edges, 0, sizeof(int32) * _width * _height);
    // Tracing edges with hysteresis . Non-recursive implementation.
    uint8 *pData = _pData;
    double *pMax = _pWorkspace->pMaxima;
    uint32 c = 1;
    for (int j = 1; j < _height - 1; ++j)
    {
        for (int i = 1; i < _width - 1; ++i)
        {
            if (pMax[c] >= _tmax && pData[c] == 0) 
            { // trace edges
                pData[c] = MAX_BRIGHTNESS;
                int nedges = 1;
                edges[0] = c;
 
                do
                {
                    nedges--;
                    const int t = edges[nedges];
 
                    int nbs[8]; // neighbours
                    nbs[0] = t - _width;     // nn
                    nbs[1] = t + _width;     // ss
                    nbs[2] = t + 1;      // ww
                    nbs[3] = t - 1;      // ee
                    nbs[4] = nbs[0] + 1; // nw
                    nbs[5] = nbs[0] - 1; // ne
                    nbs[6] = nbs[1] + 1; // sw
                    nbs[7] = nbs[1] - 1; // se
 
                    for (int k = 0; k < 8; k++)
                    {
                        if (pMax[nbs[k]] >= _tmin && pData[nbs[k]] == 0) 
                        {
                            pData[nbs[k]] = MAX_BRIGHTNESS;
                            edges[nedges] = nbs[k];
                            nedges++;
                        }
                    }
                } while (nedges > 0);
            }
            c++;
        }
        c += 2;
    }
#else
    uint32 x, y;

    // 4. Mark double threshold _tmin and _tmax
    uint8 *pData = _pData;
    double *pMax = _pWorkspace->pMaxima;
    for (y = 0; y < _height; ++y) {
        for (x = 0; x < _width; ++x) {
            //int src_pos = x + (y * _width);
            if (*pMax > _tmax)
            {
                *pData = MAX_BRIGHTNESS;
            }
            else if (*pMax > _tmin) 
            {
                *pData = TMP_BRIGHTNESS;
            }
            else 
            {
                *pData = 0;
            }
            ++pMax;
            ++pData;
        }
    }

    // 5. Refine edges with hysteresis
    pData = _pData;
    for (y = 1; y < _height - 1; y++) {
        for (x = 1; x < _width - 1; x++) {
            //int src_pos = x + (y * _width);
            if (*pData == TMP_BRIGHTNESS) {
                if (*(pData - _width - 1) == MAX_BRIGHTNESS ||
                    *(pData - _width) == MAX_BRIGHTNESS ||
                    *(pData - _width + 1) == MAX_BRIGHTNESS || 
                    *(pData - 1) == MAX_BRIGHTNESS || 
                    *(pData + 1) == MAX_BRIGHTNESS ||
                    *(pData + _width - 1) == MAX_BRIGHTNESS ||
                    *(pData + _width) == MAX_BRIGHTNESS ||
                    *(pData + _width + 1) == MAX_BRIGHTNESS) {
                    *pData = MAX_BRIGHTNESS;
                } else {
                    *pData = 0;
                }
            }
            ++pData;
        }
        pData += 2;
    }
#endif
}

//-------------------------------------
SCannyWorkspace *
create_workspace_canny(const uint32 _width, const uint32 _height)
{
    SCannyWorkspace * work = (SCannyWorkspace *)malloc(sizeof(SCannyWorkspace));
    work->width  = _width;
    work->height = _height;

    uint32 size      = _width * _height;
    work->pMap       = (uint8 *)malloc(size * sizeof(uint8));
    work->pGradient  = (double *)malloc(size * sizeof(double));
    work->pMaxima    = (double *)malloc(size * sizeof(double));
    work->pDirection = (uint8 *)malloc(size * sizeof(uint8));
    return work;
}

//-------------------------------------
void 
release_workspace_canny(SCannyWorkspace *_pWorkspace)
{
    if(_pWorkspace != 0x0) 
    {
        _pWorkspace->width  = 0;
        _pWorkspace->height = 0;
        if(_pWorkspace->pMap != 0x0)
        {
            free(_pWorkspace->pMap);
            _pWorkspace->pMap = 0x0;
        }

        if(_pWorkspace->pGradient != 0x0)
        {
            free(_pWorkspace->pGradient);
            _pWorkspace->pGradient = 0x0;
        }

        if(_pWorkspace->pMaxima != 0x0)
        {
            free(_pWorkspace->pMaxima);
            _pWorkspace->pMaxima = 0x0;
        }

        if(_pWorkspace->pDirection != 0x0)
        {
            free(_pWorkspace->pDirection);
            _pWorkspace->pDirection = 0x0;
        }

        free(_pWorkspace);
    }
}

