#include "canny.h"
#include "hough.h"
#include "image.h"
#include "bmp.h"
#include "timer.h"
#include <stdio.h>
#include <time.h>

int
main(int argc, char *argv[])
{
    const char *fileIn = "in.bmp";

    const uint32 maxLines = 256;
    SPolar lineBuffer[maxLines];

    SImage imgRGB, imgGray, img;

    reset_image(&imgRGB);
    reset_image(&imgGray);
    reset_image(&img);

    ELoadError error = load_image(fileIn, &imgRGB);
    if(error == LE_NO_ERROR)
    {
        imgGray = convertToGray(imgRGB);
#if 1
        img = copy_image(&imgGray);
        canny(img.mpData, img.mWidth, img.mHeight, 50, 125);
        save_image("out.bmp", &img);
        uint32 foundLines = hough(img.mpData, img.mWidth, img.mHeight, 8, lineBuffer, maxLines);
#else
        uint64 currentTime;
        uint64 lastTime;
        uint64 startTime = current_time_ms();
        uint32 frames = 0;

        lastTime = (current_time_ms() - startTime)/1000;
        while(1)
        {
            img = copy_image(&imgGray);

            uint64 preTime = current_time_ms();
            canny(img.mpData, img.mWidth, img.mHeight, 50, 125);
            uint64 postTime = current_time_ms();
            uint32 foundLines = hough(img.mpData, img.mWidth, img.mHeight, 8, lineBuffer, maxLines);
            uint64 postTime2 = current_time_ms();
            ++frames;

            printf("canny: %llums, hough: %llums, total: %llums\n", postTime - preTime, postTime2 - postTime, postTime2 - preTime);
            currentTime = (current_time_ms() - startTime)/1000;
            if(currentTime != lastTime)
            {
                printf("fps: %d\n", frames);
                lastTime = currentTime;
                frames = 0;
            }
        }
#endif
        // printf("found %d lines. (max %d)\n", foundLines, maxLines);
        // for(uint32 i = 0; i < foundLines; ++i)
        // {
        //     printf("line %d) r: %f, t: %f\n", i, lineBuffer[i].rho, lineBuffer[i].theta);
        // }

        //save_image("out.bmp", &img);

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

    release_image(&imgRGB);
    release_image(&imgGray);
    release_image(&img);
    release_canny();
    release_hough();

    return 0;
}

