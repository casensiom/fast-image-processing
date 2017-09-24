#include "canny.h"
#include "hough.h"
#include "image.h"
#include "bmp.h"
#include <stdio.h>

int
main(int argc, char *argv[])
{
    const char *fileIn = "in.bmp";

    const uint32 maxLines = 256;
    SPolar lineBuffer[maxLines];

    SImage imgRGB;
    ELoadError error = load_image(fileIn, &imgRGB);
    if(error == LE_NO_ERROR)
    {
        SImage img = convertToGray(imgRGB);
        printf("Canny\n");
        canny(img.mpData, img.mWidth, img.mHeight, 50, 125);
        save_image("out.bmp", &img);

        printf("Hough\n");
        uint32 foundLines = hough(img.mpData, img.mWidth, img.mHeight, 8, lineBuffer, maxLines);

        printf("found %d lines. (max %d)\n", foundLines, maxLines);
        for(uint32 i = 0; i < foundLines; ++i)
        {
            printf("line %d) r: %f, t: %f\n", i, lineBuffer[i].rho, lineBuffer[i].theta);
        }

        realease_canny();
        realease_hough();
    }
    else
    {
        char *pError = "Unknonwn error";
        switch(error)
        {
            case LE_NO_ERROR:
            pError = "NO_ERROR";
            break;
            case LE_CANT_OPEN:
            pError = "CANT_OPEN";
            break;
            case LE_CANT_READ:
            pError = "CANT_READ";
            break;
            case LE_CANT_WRITE:
            pError = "CANT_WRITE";
            break;
            case LE_INVALID_MAGIC:
            pError = "INVALID_MAGIC";
            break;
        }
        printf("Error '%s' while opening '%s'\n", pError, fileIn);
    }
    return 0;
}