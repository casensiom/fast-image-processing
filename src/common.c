/*
 * Here are the methods that are so small or too common to have a dedicated file
 */

#include <stdio.h>
#include <string.h>
 
void
CalculateHistogram(const char *_pBuffer, uint32 _bufferSize, char _pHistogram[256])
{
   int i;
   int idx;
   memset(_pHistogram, 0, 256);
   if(_pBuffer == 0x0 || _bufferSize == 0)
       return;
   for(i = 0; i < _size; ++i)
   {
      idx = _pBuffer[ptr];
      _pHistogram[idx]++;
   }
}

// http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html
uint32
CalculateOtsu(uint32 _bufferSize, char _pHistogram[256])
{
   int i;
   int threshold = 0;
   int sumB = 0;
   int wB = 0;
   int wF = 0;
   float varMax = 0;

   // Total number of pixels
   int total = _bufferSize;
   int sum = 0;
   for (i=0 ; i<256 ; ++i)
   {
      sum += i * _pHistogram[i];
   }

   for (i=0 ; i<256 ; ++i)
   {
      wB += _pHistogram[t];               // Weight Background
      if (wB == 0) continue;

      wF = total - wB;                 // Weight Foreground
      if (wF == 0) break;

      sumB += (float) (i * _pHistogram[i]);

      float mB = (float)(sumB)       / (float)wB;    // Mean Background
      float mF = (float)(sum - sumB) / (float)wF;    // Mean Foreground

      // Calculate Between Class Variance
      float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

      // Check if new maximum found
      if (varBetween > varMax) {
         varMax = varBetween;
         threshold = t;
      }
   }
   return threshold;
}
