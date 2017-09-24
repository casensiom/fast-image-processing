
/*
 * Loading part taken from
 * http://www.vbforums.com/showthread.php?t=261522
 * BMP info:
 * http://en.wikipedia.org/wiki/BMP_file_format
 *
 * Note: the magic number has been removed from the bmpfile_header_t
 * structure since it causes alignment problems
 *     bmpfile_magic_t should be written/read first
 * followed by the
 *     bmpfile_header_t
 * [this avoids compiler-specific alignment pragmas etc.]
 */

#include "bmp.h"
#include <stdio.h>

typedef struct {
    uint8 magic[2];
} bmpfile_magic_t;
 
typedef struct {
    uint32 filesz;
    uint16 creator1;
    uint16 creator2;
    uint32 bmp_offset;
} bmpfile_header_t;
 
typedef struct {
    uint32 header_sz;
    int32  width;
    int32  height;
    uint16 nplanes;
    uint16 bitspp;
    uint32 compress_type;
    uint32 bmp_bytesz;
    int32  hres;
    int32  vres;
    uint32 ncolors;
    uint32 nimpcolors;
} bitmap_info_header_t;
 
typedef struct {
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 nothing;
} rgb_t;


uint32 load_image(const char *filename, SImage *pImage)
{
    release_image(pImage);

    bitmap_info_header_t myInfoHeader;
    bitmap_info_header_t *bitmapInfoHeader = &myInfoHeader;
    

    FILE *filePtr = fopen(filename, "rb");
    if (filePtr == 0x0) {
        return LE_CANT_OPEN;
    }
 
    bmpfile_magic_t mag;
    if (fread(&mag, sizeof(bmpfile_magic_t), 1, filePtr) != 1) {
        fclose(filePtr);
        return LE_CANT_READ;
    }
 
    // verify that this is a bmp file by check bitmap id
    // warning: dereferencing type-punned pointer will break
    // strict-aliasing rules [-Wstrict-aliasing]
    if (*((uint16*)mag.magic) != 0x4D42) {
        fclose(filePtr);
        return LE_INVALID_MAGIC;
    }
 
    bmpfile_header_t bitmapFileHeader; // our bitmap file header
    // read the bitmap file header
    if (fread(&bitmapFileHeader, sizeof(bmpfile_header_t),  1, filePtr) != 1) {
        fclose(filePtr);
        return LE_CANT_READ;
    }
 
    // read the bitmap info header
    if (fread(bitmapInfoHeader, sizeof(bitmap_info_header_t), 1, filePtr) != 1) {
        fclose(filePtr);
        return LE_CANT_READ;
    }
 
    if (bitmapInfoHeader->compress_type != 0)
        fprintf(stderr, "Warning, compression is not supported.\n");
 
    // move file point to the beginning of bitmap data
    if (fseek(filePtr, bitmapFileHeader.bmp_offset, SEEK_SET)) {
        fclose(filePtr);
        return LE_CANT_READ;
    }
 
    printf("Reading img: %dbpp, %d x %d\n", bitmapInfoHeader->bitspp, bitmapInfoHeader->width, bitmapInfoHeader->height);
    EPixelFormat pf = ((bitmapInfoHeader->bitspp == 32)?PF_ARGB:(bitmapInfoHeader->bitspp == 24)?PF_RGB:PF_GRAY);
    *pImage = create_image(bitmapInfoHeader->width, bitmapInfoHeader->height, pf);
    // allocate enough memory for the bitmap image data
    //pixel_t *bitmapImage = pImage->mpData;
 
    /*
    // read in the bitmap image data
    size_t pad, count=0;
    unsigned char c;
    pad = 4*ceil(bitmapInfoHeader->bitspp*bitmapInfoHeader->width/32.) - bitmapInfoHeader->width;
    for(size_t i=0; i<bitmapInfoHeader->height; i++){
        for(size_t j=0; j<bitmapInfoHeader->width; j++){
            if (fread(&c, sizeof(unsigned char), 1, filePtr) != 1) {
                fclose(filePtr);
                return LE_CANT_READ;
            }
            pImage->mpData[count++] = (uint8) c;
        }
        fseek(filePtr, pad, SEEK_CUR);
    }
    */
 
    // If we were using unsigned char as pixel_t, then:
    fread(pImage->mpData, 1, bitmapInfoHeader->bmp_bytesz, filePtr);
 
    // close file and return bitmap image data
    fclose(filePtr);
    return LE_NO_ERROR;
}


uint32 save_image(const char *filename, const SImage *_pImg)
{

    unsigned int headers[13];
    FILE * outfile;
    int extrabytes;
    int paddedsize;
    int x; int y; int n;
    int red, green, blue;
    int WIDTH = _pImg->mWidth;
    int HEIGHT = _pImg->mHeight;

    extrabytes = 4 - ((WIDTH * 3) % 4);                 // How many bytes of padding to add to each
                                                        // horizontal line - the size of which must
                                                        // be a multiple of 4 bytes.
    if (extrabytes == 4)
       extrabytes = 0;

    paddedsize = ((WIDTH * 3) + extrabytes) * HEIGHT;

    // Headers...
    // Note that the "BM" identifier in bytes 0 and 1 is NOT included in these "headers".

    headers[0]  = paddedsize + 54;      // bfSize (whole file size)
    headers[1]  = 0;                    // bfReserved (both)
    headers[2]  = 54;                   // bfOffbits
    headers[3]  = 40;                   // biSize
    headers[4]  = WIDTH;  // biWidth
    headers[5]  = HEIGHT; // biHeight

    // Would have biPlanes and biBitCount in position 6, but they're shorts.
    // It's easier to write them out separately (see below) than pretend
    // they're a single int, especially with endian issues...

    headers[7]  = 0;                    // biCompression
    headers[8]  = paddedsize;           // biSizeImage
    headers[9]  = 0;                    // biXPelsPerMeter
    headers[10] = 0;                    // biYPelsPerMeter
    headers[11] = 0;                    // biClrUsed
    headers[12] = 0;                    // biClrImportant

    outfile = fopen(filename, "wb");

    //
    // Headers begin...
    // When printing ints and shorts, we write out 1 character at a time to avoid endian issues.
    //

    fprintf(outfile, "BM");

    for (n = 0; n <= 5; n++)
    {
       fprintf(outfile, "%c", headers[n] & 0x000000FF);
       fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
       fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
       fprintf(outfile, "%c", (headers[n] & (unsigned int) 0xFF000000) >> 24);
    }

    // These next 4 characters are for the biPlanes and biBitCount fields.

    fprintf(outfile, "%c", 1);
    fprintf(outfile, "%c", 0);
    fprintf(outfile, "%c", 24);
    fprintf(outfile, "%c", 0);

    for (n = 7; n <= 12; n++)
    {
       fprintf(outfile, "%c", headers[n] & 0x000000FF);
       fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
       fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
       fprintf(outfile, "%c", (headers[n] & (unsigned int) 0xFF000000) >> 24);
    }

    //
    // Headers done, now write the data...
    //

    //for (y = HEIGHT - 1; y >= 0; y--)     // BMP image format is written from bottom to top...
    for (y = 0; y < HEIGHT; ++y) 
    {
       for (x = 0; x < WIDTH; x++)
       {
            if(_pImg->mBpp == 1)
            {
                red = _pImg->mpData[x + y * WIDTH];
                green = _pImg->mpData[x + y * WIDTH];
                blue = _pImg->mpData[x + y * WIDTH];
            }
            else 
            {
                int extra = 0;
                int bpp = 3;
                if(_pImg->mBpp == 4)
                {
                    bpp = 4;
                    extra = 1;
                }
                red = _pImg->mpData[x*bpp + y * WIDTH + extra + 0];
                green = _pImg->mpData[x*bpp + y * WIDTH + extra + 1];
                blue = _pImg->mpData[x*bpp + y * WIDTH + extra + 2];
            }

          // Also, it's written in (b,g,r) format...

          fprintf(outfile, "%c", blue);
          fprintf(outfile, "%c", green);
          fprintf(outfile, "%c", red);
       }
       if (extrabytes)      // See above - BMP lines must be of lengths divisible by 4.
       {
          for (n = 1; n <= extrabytes; n++)
          {
             fprintf(outfile, "%c", 0);
          }
       }
    }

    fclose(outfile);
    return LE_NO_ERROR;

}