#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define DEBUG_MODE 1//需要调试时置1，否则置0

#include <stdio.h>
#include <iostream>
#include <vector>
#include <windows.h> 
#include <time.h>
#include <omp.h>
#include <stdint.h>

#include <stdlib.h>
#include <math.h>


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


// 定义RGB结构体，用于存储每个像素的红绿蓝值
typedef struct {
    U8 r, g, b;  // 红色、绿色、蓝色分量
} RGB;

// 定义图像结构体，包含图像宽度、高度和图像数据
typedef struct {
    //U16 width, height;  // 图像的宽度和高度
    RGB* data;          // 图像的像素数据，数组中每个元素表示一个RGB像素
} Image;


Image* readBMP(const char* filename);

void writeBMP(const char* filename, const Image* img);   // 将Image结构体数据写入BMP文件
void freeImage(Image* img);                              // 释放图像的内存
void medianStackDenoise(Image** images, int numImages, Image* output);  // 中值堆叠去噪
void alignImages(const Image* baseImage, const Image* targetImage, int searchRange, Image* alignedImage);
int main();
double getClarityEvaluation(uchar* data, int width, int height);
// 对齐图像
void processImages(const char** fileNames, int searchNum, int searchRange);  // 图像处理流程
