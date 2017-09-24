#!/bin/sh

# im too lazy to build a makefile right now
gcc -c convolution.c -O3  -o convolution.o
gcc -c image.c -O3 -o image.o
gcc -c bmp.c -O3 -o bmp.o
gcc -c canny.c -O3 -o canny.o
gcc -c hough.c -O3 -o hough.o
gcc main.c -O3 -o main hough.o canny.o image.o bmp.o convolution.o
