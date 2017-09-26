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
    uint32 numAngle;
    uint32 numRho;
    float rho;
    float theta;
    double minTheta;
    double maxTheta;

    //--
    uint32 *pAcc;
    float *pSin;
    float *pCos;
    SCollisionPoint *pCandidates;
    uint32 numCandidates;

} SHoughWorkspace;
//-------------------------------------


//-------------------------------------
static SHoughWorkspace *spHoughWorkspace = 0x0;
//-------------------------------------

uint32 HoughLinesStandard(uint8 *_pData, uint32 _width, uint32 _height, uint32 _threshold, SPolar *_lineBuffer, uint32 _size, SHoughWorkspace *_pWorkspace);
uint8 insert_sort(uint32 _idx, uint32 _acc, SHoughWorkspace *_pWorkspace);
SHoughWorkspace *create_workspace_hough(const uint32 _width, const uint32 _height, const float _rho, const float _theta, const double _minTheta, const double _maxTheta);
void reset_workspace_hough(SHoughWorkspace *_pWorkspace);
void release_workspace_hough(SHoughWorkspace *_pWorkspace);
//-------------------------------------


//-------------------------------------
void
init_hough(const uint32 _width, const uint32 _height)
{
    float rho = 1;
    float theta = M_PI/180.0f;
    double minTheta = 0;
    double maxTheta = M_PI;
    init_hough_ex(_width, _height, rho, theta, minTheta, maxTheta);
}

//-------------------------------------
void
init_hough_ex(const uint32 _width, const uint32 _height, const float _rho, const float _theta, 
    const double _minTheta, const double _maxTheta)
{
    if(spHoughWorkspace == 0x0)
    {
        spHoughWorkspace = create_workspace_hough(_width, _height, _rho, _theta, _minTheta, _maxTheta);
    }
    else if(spHoughWorkspace->width != _width || spHoughWorkspace->height != _height ||
            spHoughWorkspace->rho != _rho ||  spHoughWorkspace->theta != _theta ||
            spHoughWorkspace->minTheta != _minTheta || spHoughWorkspace->maxTheta != _maxTheta)
    {
        // TODO: Alert the user that using canny in different image sizes can decrease the efficiency
        release_workspace_hough(spHoughWorkspace);
        spHoughWorkspace = create_workspace_hough(_width, _height, _rho, _theta, _minTheta, _maxTheta);
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
    uint32 i, j;

    float rho = _pWorkspace->rho;
    float theta = _pWorkspace->theta;
    double minTheta = _pWorkspace->minTheta;
    double maxTheta = _pWorkspace->maxTheta;
    uint32 numAngle = _pWorkspace->numAngle;
    uint32 numRho = _pWorkspace->numRho;
    uint32 *pAccum = _pWorkspace->pAcc;
    float *pSin = _pWorkspace->pSin;
    float *pCos = _pWorkspace->pCos;
    uint8* pData = _pData;

    //float irho = 1.0f / rho;

    reset_workspace_hough(_pWorkspace);

    // stage 1. fill accumulator
    for( i = 0; i < _height; ++i )
    {
        for( j = 0; j < _width; ++j )
        {
            if( *pData != 0 )
            {
                for(uint32 n = 0; n < numAngle; ++n )
                {
                    uint32 r = (uint32)(( j * pCos[n] + i * pSin[n] ) + 0.5f);
                    r += (numRho - 1) / 2;
                    pAccum[(n+1) * (numRho+2) + r+1]++;
                }
            }
            ++pData;
        }
    }

    // stage 2. find local maximums and store the candidates in a sorted array
    for(uint32 r = 0; r < numRho; ++r )
    {
        for(uint32 n = 0; n < numAngle; ++n )
        {
            uint32 base = (n+1) * (numRho+2) + r+1;
            if( pAccum[base] > _threshold &&
                pAccum[base] > pAccum[base - 1] && pAccum[base] >= pAccum[base + 1] &&
                pAccum[base] > pAccum[base - numRho - 2] && pAccum[base] >= pAccum[base + numRho + 2] )
            {
                insert_sort(base, pAccum[base], _pWorkspace);
                if(_pWorkspace->numCandidates >= _size)
                {
                    r = numRho;
                    n = numAngle;
                    break;
                }
            }
        }
    }

    // stage 4. store the first lines to the output buffer
    uint32 linesMax = ((_size <= _pWorkspace->numCandidates) ? _size : _pWorkspace->numCandidates);
    double scale = 1./(numRho+2);
    for( i = 0; i < linesMax; i++ )
    {
        SPolar line;
        int32 idx = _pWorkspace->pCandidates[i].index;
        int32 n = (int32)((idx*scale) - 1);
        int32 r = idx - (n+1)*(numRho+2) - 1;
        line.rho = (r - (numRho - 1)*0.5f) * rho;
        line.theta = ((float)minTheta) + (float)n * theta;
        _lineBuffer[i] = line;
    }

    return linesMax;
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
create_workspace_hough(const uint32 _width, const uint32 _height, const float _rho, const float _theta, const double _minTheta, const double _maxTheta)
{


    SHoughWorkspace * work = (SHoughWorkspace *)malloc(sizeof(SHoughWorkspace));

    double minTheta = _minTheta;
    double maxTheta = _maxTheta;
    if(minTheta > maxTheta) 
    {
        double temp = maxTheta;
        maxTheta = minTheta;
        minTheta = temp;
    }

    uint32 numAngle = (uint32)(((maxTheta - minTheta) / _theta) + 0.5f);
    uint32 numRho = (uint32)((((_width + _height) * 2 + 1) / _rho) + 0.5f);
    //--
    work->width    = _width;
    work->height   = _height;
    work->rho      = _rho;
    work->theta    = _theta;
    work->minTheta = minTheta;
    work->maxTheta = maxTheta;
    work->numAngle = numAngle;
    work->numRho   = numRho;

    work->pSin = (float *)malloc(numAngle * sizeof(float));
    work->pCos = (float *)malloc(numAngle * sizeof(float));
    work->pAcc = (uint32 *)malloc((numAngle+2) * (numRho+2) * sizeof(uint32));
    work->pCandidates = (SCollisionPoint *)malloc((numAngle+2) * (numRho+2) * sizeof(SCollisionPoint));
    work->numCandidates = 0;

    //printf("Creating hough workpace with: width: %d, height: %d, numAngle: %d, numRho: %d, rho: %f, theta: %f, minTheta: %f, maxTheta: %f\n", _width, _height, numAngle, numRho, _rho, _theta, minTheta, maxTheta);

    float irho = 1 / _rho;
    float ang = minTheta;
    for(int n = 0; n < numAngle; ang += _theta, ++n)
    {
        work->pSin[n] = (float)(sin((double)ang) * irho);
        work->pCos[n] = (float)(cos((double)ang) * irho);
    }

    return work;
}

//-------------------------------------
void
reset_workspace_hough(SHoughWorkspace *_pWorkspace)
{
    memset(_pWorkspace->pAcc, 0, sizeof(_pWorkspace->pAcc[0]) * (_pWorkspace->numAngle+2) * (_pWorkspace->numRho+2));
    memset(_pWorkspace->pCandidates, 0, sizeof(_pWorkspace->pCandidates[0]) * (_pWorkspace->numAngle+2) * (_pWorkspace->numRho+2));
    _pWorkspace->numCandidates = 0;
}
//-------------------------------------
void 
release_workspace_hough(SHoughWorkspace *_pWorkspace)
{
    if(_pWorkspace != 0x0) 
    {
        _pWorkspace->numAngle  = 0;
        _pWorkspace->numRho = 0;
        _pWorkspace->numCandidates = 0;
        if(_pWorkspace->pAcc != 0x0)
        {
            free(_pWorkspace->pAcc);
            _pWorkspace->pAcc = 0x0;
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

        if(_pWorkspace->pCandidates != 0x0)
        {
            free(_pWorkspace->pCandidates);
            _pWorkspace->pCandidates = 0x0;
        }

        free(_pWorkspace);
    }
}
