#!/bin/sh

# im too lazy to build a makefile right now
mkdir obj
cd obj

gcc -c ../../src/convolution.c -O3  -o convolution.o
gcc -c ../../src/image.c -O3 -o image.o
gcc -c ../../src/bmp.c -O3 -o bmp.o
gcc -c ../../src/canny.c -O3 -o canny.o
gcc -c ../../src/hough.c -O3 -o hough.o
gcc -c ../../src/timer.c -O3 -o timer.o
gcc ../../src/main.c -O3 -o ../../bin/edges timer.o hough.o canny.o image.o bmp.o convolution.o
