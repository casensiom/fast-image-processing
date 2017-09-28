#include "hough.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//-------------------------------------
typedef struct SCollisionPoint
{
    uint32 index;
    uint32 acc;
}SCollisionPoint;
//-------------------------------------

//-------------------------------------
typedef struct SHoughWorkspace
{
    //--
    uint32 width;
    uint32 height;
    double accMaxR;
    uint32 accHeight;
    uint32 accWidth;
    uint32 *pAccum;
    uint32 accCenterX;
    uint32 accCenterY;

    //--
    SCollisionPoint *pCandidates;
    uint32 numCandidates;

    //--
    float *pSin;
    float *pCos;

} SHoughWorkspace;
//-------------------------------------


//-------------------------------------
static SHoughWorkspace *spHoughWorkspace = 0x0;
//-------------------------------------

uint32 HoughLinesStandard(uint8 *_pData, uint32 _width, uint32 _height, uint32 _threshold, SPolar *_lineBuffer, uint32 _size, SHoughWorkspace *_pWorkspace);
uint8 CheckMaximaLocal(uint32 *_pAccum, uint32 _index, uint32 _width, uint32 _height);
uint8 insert_sort(uint32 _idx, uint32 _acc, SHoughWorkspace *_pWorkspace);
SHoughWorkspace *create_workspace_hough(const uint32 _width, const uint32 _height);
void reset_workspace_hough(SHoughWorkspace *_pWorkspace);
void release_workspace_hough(SHoughWorkspace *_pWorkspace);
//-------------------------------------

//-------------------------------------
void
init_hough(const uint32 _width, const uint32 _height)
{
    if(spHoughWorkspace == 0x0)
    {
        spHoughWorkspace = create_workspace_hough(_width, _height);
    }
    else if(spHoughWorkspace->width != _width || spHoughWorkspace->height != _height)
    {
        // TODO: Alert the user that using hough in different image sizes can decrease the efficiency
        release_workspace_hough(spHoughWorkspace);
        spHoughWorkspace = create_workspace_hough(_width, _height);
    }
}

//-------------------------------------
void
release_hough()
{
    release_workspace_hough(spHoughWorkspace);
    spHoughWorkspace = 0x0;
}

//-------------------------------------
uint32
hough(uint8 *_pData, uint32 _width, uint32 _height, uint32 _threshold, SPolar *_lineBuffer, uint32 _size)
{
    init_hough(_width, _height);
    return HoughLinesStandard(_pData, _width, _height, _threshold, _lineBuffer, _size, spHoughWorkspace);
}

//
// source from openCV library, adapted as needed
//
//-------------------------------------
uint32
HoughLinesStandard(uint8 *_pData, uint32 _width, uint32 _height, uint32 _threshold, 
                    SPolar *_lineBuffer, uint32 _size, SHoughWorkspace *_pWorkspace)
{
    int32 x, y, r, t;

    uint32 *pAccum = _pWorkspace->pAccum;
    float *pSin = _pWorkspace->pSin;
    float *pCos = _pWorkspace->pCos;
    int32 centerX = _width / 2;
    int32 centerY = _height / 2;
    double accMaxR = _pWorkspace->accMaxR;
    uint8* pData = _pData;

    reset_workspace_hough(_pWorkspace);

    // stage 1. fill accumulator
    for( y = 0; y < _height; ++y )
    {
        for( x = 0; x < _width; ++x )
        {
            if( *_pData > 0 )
            {
                //printf(" - pixel white\n");
                for(t = 0; t < _pWorkspace->accWidth; ++t )
                {
                    double r = ((double)(x - centerX) * pCos[t]) + ((double)(y - centerY) * pSin[t]) + accMaxR;  
                    pAccum[(int)((r * (double)_pWorkspace->accWidth) + 0.5) + t]++; 
                }
            }
            ++_pData;
        }
    }


    // stage 2. find local maximums and store the candidates in a sorted array
    for(r = 0; r < _pWorkspace->accHeight; ++r )
    {
        for(t = 0; t < _pWorkspace->accWidth; ++t )
        {
            uint32 base = (r * _pWorkspace->accWidth) + t;
            if( pAccum[base] > _threshold && CheckMaximaLocal(pAccum, base, _pWorkspace->accWidth, _pWorkspace->accHeight) == 0)
            {
                //printf(" - Sort line %d (r: %d, t: %d)\n", base, r, t);
                insert_sort(base, pAccum[base], _pWorkspace);
            }
        }
    }

    // stage 3. store the first lines to the output buffer
    uint32 i, linesMax = ((_size <= _pWorkspace->numCandidates) ? _size : _pWorkspace->numCandidates);
    for( i = 0; i < linesMax; i++ )
    {
        SPolar line;
        int32 idx  = _pWorkspace->pCandidates[i].index;
        line.rho   = (float)(((float)idx / (float)_pWorkspace->accWidth) - accMaxR);
        line.theta = (float)((idx % _pWorkspace->accWidth) * DEG2RAD);
        _lineBuffer[i] = line;
        printf(" * Save line %d (r: %f, t: %f) = %d\n", idx, line.rho, line.theta, _pWorkspace->pCandidates[i].acc);
    }

    return linesMax;
}

//-------------------------------------
uint8
CheckMaximaLocal(uint32 *_pAccum, uint32 _index, uint32 _width, uint32 _height)
{
    uint8 ret = 0;
#define CHECK_MAXIMA_SIDES
#ifdef CHECK_MAXIMA_SIDES
        ret = (_index > _width && _index < (_height -1)*_width && 
                _pAccum[_index] >  _pAccum[_index - 1] && 
                _pAccum[_index] >= _pAccum[_index + 1] &&
                _pAccum[_index] >  _pAccum[_index - _width] && 
                _pAccum[_index] >= _pAccum[_index + _width] ) ? 0 : 1;

#else
#endif
    return ret;
}

//-------------------------------------
uint8
insert_sort(uint32 _idx, uint32 _acc, SHoughWorkspace *_pWorkspace)
{
    uint8 inserted = 0;
    SCollisionPoint *pSorted = _pWorkspace->pCandidates;
    uint32 numCandidates = _pWorkspace->numCandidates;

    for(uint32 i = 0; i < numCandidates; ++i)
    {
        if(_acc > pSorted[i].acc || (_acc == pSorted[i].acc && _idx < pSorted[i].index))
        {
            inserted = 1;
            for(uint32 j = numCandidates; j > i; --j)
            {
                pSorted[j] = pSorted[j-1];
            }

            _pWorkspace->numCandidates++;
            pSorted[i].acc = _acc;
            pSorted[i].index = _idx;
            break;
        }
    }

    if(inserted == 0)
    {
        pSorted[numCandidates].acc = _acc;
        pSorted[numCandidates].index = _idx;
        _pWorkspace->numCandidates++;
    }
    return inserted;
}

//-------------------------------------
SHoughWorkspace *
create_workspace_hough(const uint32 _width, const uint32 _height)
{
    SHoughWorkspace * work = (SHoughWorkspace *)malloc(sizeof(SHoughWorkspace));

    
    work->width    = _width;
    work->height   = _height;
    work->accMaxR    = ((sqrt(2.0) * (double)((_height>_width)?_height:_width)) / 2.0);  
    work->accHeight  = (uint32)(work->accMaxR * 2.0 + 0.5); // -r -> +r  
    work->accWidth   = 180;
    work->pAccum     = (uint32 *)malloc(work->accWidth * work->accHeight * sizeof(uint32));
    work->accCenterX = _width >> 1;
    work->accCenterY = _height >> 1;

    work->pCandidates = (SCollisionPoint *)malloc(work->accWidth * work->accHeight * sizeof(SCollisionPoint));
    work->numCandidates = 0;

    printf("Creating hough workpace with: width: %d, height: %d, accMaxR: %f, maxR: %d, maxT: %d\n", _width, _height, work->accMaxR, work->accHeight, work->accWidth);

    work->pSin = (float *)malloc(180 * sizeof(float));
    work->pCos = (float *)malloc(180 * sizeof(float));
    for(int n = 0; n < 180; ++n)
    {
        work->pSin[n] = (float)(sin((double)n * DEG2RAD));
        work->pCos[n] = (float)(cos((double)n * DEG2RAD));
    }

    return work;
}

//-------------------------------------
void
reset_workspace_hough(SHoughWorkspace *_pWorkspace)
{
    memset(_pWorkspace->pAccum, 0, sizeof(_pWorkspace->pAccum[0]) * _pWorkspace->accWidth * _pWorkspace->accHeight);
    memset(_pWorkspace->pCandidates, 0, sizeof(_pWorkspace->pCandidates[0]) * _pWorkspace->accWidth * _pWorkspace->accHeight);
    _pWorkspace->numCandidates = 0;
}
//-------------------------------------
void 
release_workspace_hough(SHoughWorkspace *_pWorkspace)
{
    if(_pWorkspace != 0x0) 
    {
        _pWorkspace->numCandidates = 0;
        if(_pWorkspace->pAccum != 0x0)
        {
            free(_pWorkspace->pAccum);
            _pWorkspace->pAccum = 0x0;
        }

        if(_pWorkspace->pCandidates != 0x0)
        {
            free(_pWorkspace->pCandidates);
            _pWorkspace->pCandidates = 0x0;
        }

        if(_pWorkspace->pSin != 0x0)
        {
            free(_pWorkspace->pSin);
            _pWorkspace->pSin = 0x0;
        }

        if(_pWorkspace->pCos != 0x0)
        {
            free(_pWorkspace->pCos);
            _pWorkspace->pCos = 0x0;
        }

        free(_pWorkspace);
    }
}
