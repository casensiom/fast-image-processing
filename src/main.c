#include "canny.h"
#include "hough.h"
#include "image.h"
#include "bmp.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

void draw2(SImage *_pImg, float _theta, float _rho);
void draw(SImage *_pImg, float _theta, float _rho);

#define IS_ZERO(x) (CHECK_EPSILON(x, 0.0001))
#define CHECK_EPSILON(x, e) (fabs(x) <= e)


char *input_filename = "in.bmp";
int hough_threshold = 150;
int canny_min_threshold = 50;
int canny_max_threshold = 125;
int apply_canny = 1;
int max_lines = 16;
int verbose = 0;

#define STR_CANNY_DISABLE "--disable_canny"
#define STR_VERBOSE "--verbose"

#define STR_HOUGH_THRESHOLD "-hough"
#define STR_CANNY_MIN_THRESHOLD "-canny_min"
#define STR_CANNY_MAX_THRESHOLD "-canny_max"
#define STR_MAX_LINES "-lines"
#define STR_INPUT_FILENAME "-input"

int
get_args(int argc, char *argv[])
{
    int ret = 0;
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], STR_CANNY_DISABLE) == 0)
        {
            apply_canny = 0;
        }
        else if(strcmp(argv[i], STR_HOUGH_THRESHOLD) == 0)
        {
            ++i;
            hough_threshold = atoi(argv[i]);
        }
        else if(strcmp(argv[i], STR_CANNY_MIN_THRESHOLD) == 0)
        {
            ++i;
            canny_min_threshold = atoi(argv[i]);
        }
        else if(strcmp(argv[i], STR_CANNY_MAX_THRESHOLD) == 0)
        {
            ++i;
            canny_max_threshold = atoi(argv[i]);
        }
        else if(strcmp(argv[i], STR_MAX_LINES) == 0)
        {
            ++i;
            max_lines = atoi(argv[i]);
        }
        else if(strcmp(argv[i], STR_INPUT_FILENAME) == 0)
        {
            ++i;
            input_filename = argv[i];
        }
        else if(strcmp(argv[i], STR_VERBOSE) == 0)
        {
            verbose = 1;
        }
        else if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "/h") == 0)
        {
            ret = 1;
        }
    }
    return ret;
}

void
print_help()
{
    printf("\n");
    printf("You can set any of these properties, set the property name followed by the parameter value:\n");
    printf("  '%s'\t\tnumber   (default is 150) [indicates the value for hough threshold, thats the number of points to define a line]\n", STR_HOUGH_THRESHOLD);
    printf("  '%s'\t\tnumber   (default is 50)  [indicates the min threshold for canny filter]\n", STR_CANNY_MIN_THRESHOLD);
    printf("  '%s'\t\tnumber   (default is 125) [indicates the max threshold for canny filter]\n", STR_CANNY_MAX_THRESHOLD);
    printf("  '%s'\t\tnumber   (default is 16)  [indicates the maximun number of lines to store]\n", STR_MAX_LINES);
    printf("  '%s'\t\tfilename (default is 'in.bmp') [indicates the filename of the input image]\n", STR_INPUT_FILENAME);
    printf("\n");
    printf("You can set any of these flags:\n");
    printf("  '%s'\t[indicates it MUST NOT apply canny to the input image]\n", STR_CANNY_DISABLE);
    printf("  '%s'\t\t[indicates it MUST dump info messages]\n", STR_VERBOSE);
    printf("\n");
}

int
main(int argc, char *argv[])
{
    if(get_args(argc, argv))
    {
        print_help();
        return 0;
    }

    //if(verbose == 1)
    {
        printf("Current properties:\n");
        printf("    %s\t %s\n", STR_INPUT_FILENAME, input_filename);
        printf("    %s\t %d\n", STR_HOUGH_THRESHOLD, hough_threshold);
        printf("    %s\t %d\n", STR_CANNY_MIN_THRESHOLD, canny_min_threshold);
        printf("    %s\t %d\n", STR_CANNY_MAX_THRESHOLD, canny_max_threshold);
        printf("    %s\t %d\n", STR_MAX_LINES, max_lines);
        if(apply_canny == 0)
            printf("    %s\n", STR_CANNY_DISABLE);
        if(verbose == 1)
            printf("    %s\n", STR_VERBOSE);
    }

    //const char *fileIn = input_filename;

    const uint32 maxLines = max_lines;
    SPolar lineBuffer[maxLines];

    SImage imgRGB, imgGray, img;

    reset_image(&imgRGB);
    reset_image(&imgGray);
    reset_image(&img);

    ELoadError error = load_image(input_filename, &imgRGB);
    if(error != LE_NO_ERROR)
    {
        uint32 side = 350;
        imgRGB = create_image(side, side, PF_GRAY);
        for(uint32 y = 0; y < side; ++y)
        {
            for(uint32 x = 0; x < side; ++x)
            {
                imgRGB.mpData[x+y*side] = (x % 255) ^ (y % 255);
            }
        }
        save_image(input_filename, &imgRGB);
        error = LE_NO_ERROR;
    }

    if(error == LE_NO_ERROR)
    {
        imgGray = convertToGray(imgRGB);
#if 1
        copy_image(&imgGray, &img);
        if(apply_canny == 1)
            canny(img.mpData, img.mWidth, img.mHeight, canny_min_threshold, canny_max_threshold);
        save_image("out.bmp", &img);
        uint32 foundLines = hough(img.mpData, img.mWidth, img.mHeight, hough_threshold, lineBuffer, maxLines);

        save_hough_workspace("out_hough_workspace.bmp");


        //if(verbose == 1)
        {
            printf("%d lines found.\n", foundLines);
            printf("print lines.\n");
        }
        release_image(&imgRGB);
        imgRGB = convertToARGB(img);
        uint32 i;
        for(i =0; i < foundLines; ++i)
        {
            draw(&imgRGB, lineBuffer[i].theta, lineBuffer[i].rho);
            //draw2(&imgRGB, lineBuffer[i].theta, lineBuffer[i].rho);
        }
        //draw_line(&imgRGB, 0xFF0000, 0, 0, imgRGB.mWidth, imgRGB.mHeight);
        //draw_line(&imgRGB, 0xFF00FF00, imgRGB.mWidth/2, 0, imgRGB.mWidth/2, imgRGB.mHeight);
        save_image("out_hough.bmp", &imgRGB);
#else
        uint64 currentTime;
        uint64 lastTime;
        uint64 startTime = current_time_ms();
        uint32 frames = 0;
        uint64 cannyTime = 0;
        uint64 houghTime = 0;

        lastTime = (current_time_ms() - startTime)/1000;
        while(1)
        {
            copy_image(&imgGray, &img);

            uint64 preTime = current_time_ms();
            canny(img.mpData, img.mWidth, img.mHeight, 50, 125);
            uint64 postTime = current_time_ms();
            uint32 foundLines = hough(img.mpData, img.mWidth, img.mHeight, 8, lineBuffer, maxLines);
            uint64 postTime2 = current_time_ms();
            ++frames;
            cannyTime += postTime - preTime;
            houghTime += postTime2 - postTime;

            //printf("canny: %llums, hough: %llums, total: %llums\n", postTime - preTime, postTime2 - postTime, postTime2 - preTime);
            currentTime = (current_time_ms() - startTime)/1000;
            if(currentTime != lastTime)
            {
                printf("fps: %d, canny: %fms, hough: %fms\n", frames, (double)cannyTime / (double)frames, (double)houghTime / (double)frames);
                lastTime = currentTime;
                cannyTime = 0;
                houghTime = 0;
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
        printf("Error '%s' while opening '%s'\n", pError, input_filename);
    }

    release_image(&imgRGB);
    release_image(&imgGray);
    release_image(&img);
    release_canny();
    release_hough();

    return 0;
}


void draw2(SImage *_pImg, float _theta, float _rho)
{
    int x1, y1, x2, y2;
    x1 = y1 = x2 = y2 = 0;
    uint32 w = _pImg->mWidth;
    uint32 h = _pImg->mHeight;

    if(_theta >= 45*DEG2RAD && _theta <= 135*DEG2RAD)
    {
        //y = (r - x cos(t)) / sin(t)
        x1 = 0;
        y1 = ((double)(_rho) - ((x1 - (w/2) ) * cos(_theta))) / sin(_theta) + (h / 2);
        x2 = w;
        y2 = ((double)(_rho) - ((x2 - (w/2) ) * cos(_theta))) / sin(_theta) + (h / 2);
    }
    else
    {
        //x = (r - y sin(t)) / cos(t);
        y1 = 0;
        x1 = ((double)(_rho) - ((y1 - (h/2) ) * sin(_theta))) / cos(_theta) + (w / 2);
        y2 = h;
        x2 = ((double)(_rho) - ((y2 - (h/2) ) * sin(_theta))) / cos(_theta) + (w / 2);
    }
    printf("draw line2: t:%f, r:%f => (%d, %d) - (%d, %d)\n", _theta, _rho, x1, y1, x2, y2);
}


void draw(SImage *_pImg, float _theta, float _rho)
{
    int x1, y1, x2, y2;
    uint32 w = _pImg->mWidth;
    uint32 h = _pImg->mHeight;


    double      a = cos(_theta);
    double      b = sin(_theta);
    double      x0 = a * _rho;
    double      y0 = b * _rho;
    double      x = x0 - b; // (cos(_theta) * _rho) - sin(_theta)
    double      y = y0 + a; // (sin(_theta) * _rho) + cos(_theta)


    double      m = (b != 0) ? (-a / b) : 0;
    double      q = (b != 0) ? (_rho / b) : 0;

    // double      r = sqrt( x*x + y*y );
    // double      t = inv_tan( y / x );



    // x1 = (int)(x0 + 1000*(-b));
    // y1 = (int)(y0 + 1000*(a));
    // x2 = (int)(x0 - 1000*(-b));
    // y2 = (int)(y0 - 1000*(a));


    // y - y0 = (y0 + a) - y0 => a
    // x - x0 = (x0 - b) - x0 => -b
    // coefA = (y - y0) / (x - x0) => (a / -b) => m
    // _r / b
    // coefB = y0 - (coefA * x0) => (b * _rho) - ((a / -b) *  a * _rho)
    //
    double      coefA = ((x != x0) ? ((y - y0) / (x - x0)) : 0.0);
    double      coefB = y0 - (coefA * x0);

    if (fabs(b) < (M_PI / 4))
    {//vertical line : from y = 0 to y = frameSize.height
        y1 = 0;
        x1 = (coefA != 0.0 ? (int)(-coefB / coefA) : (int)x0) ;
        y2 = h;
        x2 = (coefA != 0.0 ? (int)((y2 - coefB) / coefA) : (int)x0)  ;
    }
    else
    {//horizontal line : from x = 0 to x = frameSize.width
        x1 = 0;
        y1 = (int)coefB;
        x2 = w;
        y2 = (int)(coefA * x2 + coefB) ;
    }


    //--
    uint32 i = 0;
    SCartesian p[4];
    p[0].x = 0;
    p[0].y = 0;
    p[1].x = 0;
    p[1].y = 0;
    if(IS_ZERO(m))
    {
        if (fabs(b) < (M_PI / 4))
        {
            p[i].x = x0 + w/2;
            p[i].y = 0;
            ++i;
            p[i].x = x0 + w/2;
            p[i].y = h;
            ++i;
        }
        else
        {
            p[i].x = 0;
            p[i].y = y0 + h/2;
            ++i;
            p[i].x = w;
            p[i].y = y0 + h/2;
            ++i;
        }
    }
    else
    {
        //  http://www.keymolen.com/2013/05/hough-transformation-c-implementation.html
        i = 0;
        if(_theta >= (M_PI/4) && _theta <= (3*M_PI/4))
        {
            //y = (r - x cos(t)) / sin(t)
            p[i].x = 0;
            p[i].y = (_rho + ((w/2) * a)) / b + (h/2);
            ++i;
            p[i].x = w - 0;
            p[i].y = (_rho - ((w/2) * a)) / b + (h/2);
            ++i;
        }
        else
        {
            //x = (r - y sin(t)) / cos(t);
            p[i].y = 0;
            p[i].x = (_rho + ((h/2) * b)) / a + (w/2);
            ++i;
            p[i].y = h - 0;
            p[i].x = (_rho - ((h/2) * b)) / a + (w/2);
            ++i;
        }
    }

    if(verbose == 1)
    {
        printf("draw line: t:%f (%.02fº), r:%.02f  --> coef( %f, %f) (m: %f, q: %f) px: (%f, %f)\n", _theta, _theta * RAD2DEG, _rho, coefA, coefB, m, q, x0, y0);
        printf("     line: (%03d, %03d) - (%03d, %03d)\n", x1, y1, x2, y2);
        printf("     line: (%03d, %03d) - (%03d, %03d) \n", p[0].x, p[0].y, p[1].x, p[1].y);
    }
    //draw_line(_pImg, 0xFF0000, x1, y1, x2, y2);
    draw_line(_pImg, 0xFF0000, p[0].x, p[0].y, p[1].x, p[1].y);

    //draw_line(_pImg, 0xFF00FF, x1, y1, x2, y2);
}

