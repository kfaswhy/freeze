#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define DEBUG_MODE 1//需要调试时置1，否则置0

#include <stdio.h>
#include <iostream>
#include <vector>
//#include <windows.h> 
//#include <time.h>
//#include <omp.h>
//#include <stdint.h>
//
//#include <stdlib.h>
//#include <math.h>

#include <opencv2/opencv.hpp>


typedef unsigned long long U64;
typedef long long S64;
typedef unsigned int U32;
typedef int S32;
typedef unsigned short U16;
typedef short S16;
typedef unsigned char U8;

typedef unsigned char uchar;


#define U16MAX (0xFFFF)
#define U8MAX (255)
#define U8MIN (0)

#define calc_min(a,b) ((a)>(b)?(b):(a))
#define calc_max(a,b) ((a)<(b)?(b):(a))
#define calc_abs(a) ((a)>0?(a):(-a))
#define clp_range(min,x,max) calc_min(calc_max((x), (min)), (max))

#define safe_sub(a,b) ((a)>(b)?(a-b):0)

#define OK 0
#define ERROR -1

#define LOG(...) {printf("%s [%d]: ", __FUNCTION__, __LINE__); printf(__VA_ARGS__); printf("\n"); }

int main();

U8** select_images();

U8 is_ok(double base, double test);

U8* read_img(const char* file_name);

U8* save_img(const char* file_name, U8* img);

U8* medianStackDenoise(U8** images);

U8* alignImages(U8* baseImage, U8* targetImage);

double getClarityEvaluation(uchar* data);


