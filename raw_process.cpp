#include "raw_process.h"

using namespace std;

int img_num = 14; //图像数量
int search_range = 20; // 模板匹配的最大范围

int search_step = 1;  //模板匹配时移动的步长
float search_area = 0.4; //计算匹配代价函数时的取样范围比例
int search_sample = 2; //计算匹配代价函数时的下采样强度，1表示不做下采样

int main() {

    // 要处理的图像文件路径数组
    const char* fileNames[] = {
        "data3/1.bmp",
        "data3/2.bmp",
        "data3/3.bmp",
        "data3/4.bmp",
        "data3/5.bmp",
        "data3/6.bmp",
        "data3/7.bmp",
        "data3/8.bmp",
        "data3/9.bmp",
        "data3/10.bmp",
        "data3/11.bmp",
        "data3/12.bmp",
        "data3/13.bmp",
        "data3/14.bmp",
        "data3/15.bmp",
        "data3/16.bmp",
        "data3/17.bmp",
        "data3/18.bmp",
        "data3/19.bmp",
        "data3/20.bmp"
    };
    int fileCount = sizeof(fileNames) / sizeof(fileNames[0]);  // 计算文件数量

    if (fileCount < img_num)
    {
        LOG("image num error.");
        return ERROR;
    }


    // 调用processImages函数处理图像

    Image** images = (Image**)malloc(img_num * sizeof(Image*));
    //int count = 0;

    U8* data = (U8*)malloc(1112 * 900 * 3 * sizeof(U8));

    // 读取指定数量的图像文件
    for (int i = 0; i < img_num; ++i) {
        images[i] = readBMP(fileNames[i]);
        memcpy(data, images[i]->data, 1112 * 900 * 3 * sizeof(U8));
        float ret = getClarityEvaluation(data, images[i]->width, images[i]->height);
        cout <<i+1<<". " << ret << endl;
    }

    int t0 = clock();

    // 对齐图像
    Image** alignedImages = (Image**)malloc(img_num * sizeof(Image*));
    alignedImages[0] = images[0];


    for (int i = 1; i < img_num; ++i) {
        alignedImages[i] = (Image*)malloc(sizeof(Image));
        alignImages(images[0], images[i], search_range, alignedImages[i]);
    }

    // 对齐后的图像进行中值堆叠去噪
    Image denoisedImage;
    medianStackDenoise(alignedImages, img_num, &denoisedImage);

    int t1 = clock();
    LOG("time_used = %.02f.", ((float)t1 - t0) / 1000);

    // 保存去噪后的图像
    writeBMP("denoised_image.bmp", &denoisedImage);
    printf("图像处理完成，去噪后的图像已保存为 denoised_image.bmp\n");

    return 0;
}

double getClarityEvaluation(uchar* data, int width, int height) {
    // 将BGR数据转换为灰度
    std::vector<double> grayscaleData(width * height);
    for (int i = 0; i < width * height; ++i) {
        grayscaleData[i] = 0.299 * data[3 * i] + 0.587 * data[3 * i + 1] + 0.114 * data[3 * i + 2];
    }

    // 应用Laplacian算子
    std::vector<double> laplacianData(width * height, 0.0);
    int laplacianKernel[3][3] = {
        { 0,  1,  0 },
        { 1, -4,  1 },
        { 0,  1,  0 }
    };

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            double sum = 0.0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixelPos = (y + ky) * width + (x + kx);
                    sum += grayscaleData[pixelPos] * laplacianKernel[ky + 1][kx + 1];
                }
            }
            laplacianData[y * width + x] = sum;
        }
    }

    // 计算平均值
    double meanValue = 0.0;
    int count = 0;
    for (double val : laplacianData) {
        meanValue += std::abs(val); // 使用绝对值处理负数
        ++count;
    }
    meanValue /= count;

    return meanValue;
}



// 读取BMP文件，返回包含图像数据的Image结构体指针
Image* readBMP(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("无法打开文件 %s\n", filename);
        return NULL;
    }

    // 读取BMP文件头
    U8 header[54];
    fread(header, sizeof(U8), 54, file);

    // 获取图像的宽度、高度和位深度
    U32 width = *(U32*)&header[18];
    U32 height = *(U32*)&header[22];
    U16 bitsPerPixel = *(U16*)&header[28];

    // 检查位深度是否为24位
    if (bitsPerPixel != 24) {
        printf("不支持的位深度: %d (仅支持24位)\n", bitsPerPixel);
        fclose(file);
        return NULL;
    }

    // 计算图像数据的大小
    size_t dataSize = width * height * 3;
    U8* data = (U8*)malloc(dataSize);
    fread(data, sizeof(U8), dataSize, file);
    fclose(file);

    // 为Image结构体分配内存
    Image* img = (Image*)malloc(sizeof(Image));
    img->width = width;
    img->height = height;
    img->data = (RGB*)malloc(width * height * sizeof(RGB));

    // 将图像数据从BMP格式转换为RGB格式
    for (size_t i = 0; i < dataSize; i += 3) {
        size_t index = i / 3;
        img->data[index].b = data[i];
        img->data[index].g = data[i + 1];
        img->data[index].r = data[i + 2];
    }
    free(data);  // 释放临时存储的图像数据
    return img;
}

// 将图像数据保存为BMP文件
void writeBMP(const char* filename, const Image* img) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("无法保存文件 %s\n", filename);
        return;
    }

    // 写入BMP文件头
    U8 header[54] = { 0 };
    header[0] = 'B';
    header[1] = 'M';
    U32 fileSize = 54 + img->width * img->height * 3;  // 文件总大小
    *(U32*)&header[2] = fileSize;
    *(U32*)&header[10] = 54;
    *(U32*)&header[14] = 40;
    *(U32*)&header[18] = img->width;
    *(U32*)&header[22] = img->height;
    *(U16*)&header[26] = 1;   // 颜色平面数
    *(U16*)&header[28] = 24;  // 每像素位深度

    fwrite(header, sizeof(U8), 54, file);  // 写入文件头

    // 写入像素数据
    for (size_t i = 0; i < img->width * img->height; ++i) {
        fwrite(&img->data[i].b, sizeof(U8), 1, file);
        fwrite(&img->data[i].g, sizeof(U8), 1, file);
        fwrite(&img->data[i].r, sizeof(U8), 1, file);
    }

    fclose(file);  // 关闭文件
}

// 释放图像数据占用的内存
void freeImage(Image* img) {
    if (img) {
        free(img->data);  // 释放图像数据内存
        free(img);         // 释放Image结构体内存
    }
}

// 中值堆叠去噪：对多张图像进行去噪处理，取每个像素点在所有图像中对应通道的中值
void medianStackDenoise(Image** images, int numImages, Image* output) {
    int width = images[0]->width;
    int height = images[0]->height;

    output->width = width;
    output->height = height;
    output->data = (RGB*)malloc(width * height * sizeof(RGB));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            U8 red[5], green[5], blue[5];

            // 获取每张图像对应像素点的RGB值
            for (int k = 0; k < numImages; ++k) {
                int index = i * width + j;
                red[k] = images[k]->data[index].r;
                green[k] = images[k]->data[index].g;
                blue[k] = images[k]->data[index].b;
            }

            // 对RGB通道分别进行排序
            for (int m = 0; m < numImages; ++m) {
                for (int n = m + 1; n < numImages; ++n) {
                    if (red[m] > red[n]) { U8 temp = red[m]; red[m] = red[n]; red[n] = temp; }
                    if (green[m] > green[n]) { U8 temp = green[m]; green[m] = green[n]; green[n] = temp; }
                    if (blue[m] > blue[n]) { U8 temp = blue[m]; blue[m] = blue[n]; blue[n] = temp; }
                }
            }

            int index = i * width + j;
            output->data[index].r = red[numImages / 2];  // 取中值
            output->data[index].g = green[numImages / 2];
            output->data[index].b = blue[numImages / 2];
        }
    }
}

// 对齐图像：通过搜索最佳偏移量使目标图像与基准图像对齐
void alignImages(const Image* baseImage, const Image* targetImage, int search_range, Image* alignedImage) {
    int bestDx = 0, bestDy = 0;
    int minError = INT_MAX;

    int centerX = baseImage->width / 2;
    int centerY = baseImage->height / 2;

    float start = 0.5 - search_area / 2;
    float end = 1 - start;

    // 遍历搜索范围内的所有偏移量，计算误差
    for (int dy = -search_range; dy <= search_range; ++dy) {
        for (int dx = -search_range; dx <= search_range; ++dx) {
            int error = 0;

            for (int y = baseImage->height * start; y < baseImage->height * end; y += search_sample) {
                for (int x = baseImage->width * start; x < baseImage->width * end; x += search_sample) {
                    int offsetX = x + dx;
                    int offsetY = y + dy;

                    if (offsetX >= 0 && offsetX < baseImage->width && offsetY >= 0 && offsetY < baseImage->height) {
                        
                        int baseIndex = y * baseImage->width + x;
                        int targetIndex = offsetY * baseImage->width + offsetX;

                        RGB basePixel = baseImage->data[baseIndex];
                        RGB targetPixel = targetImage->data[targetIndex];
        
                        int diffG = basePixel.g - targetPixel.g;
                        int diffR = basePixel.r - targetPixel.r;
                        error += diffG * diffG+ diffR* diffR;
                        
                    }
                }
            }

            if (error < minError) {  // 更新最小误差和最佳偏移量
                minError = error;
                bestDx = dx;
                bestDy = dy;
            }
        }
    }

    LOG("%d,%d",bestDx,bestDy);

    alignedImage->width = baseImage->width;
    alignedImage->height = baseImage->height;
    alignedImage->data = (RGB*)malloc(baseImage->width * baseImage->height * sizeof(RGB));

    // 应用最佳偏移量来对齐目标图像
    for (int y = 0; y < baseImage->height; ++y) {
        for (int x = 0; x < baseImage->width; ++x) {
            int offsetX = x + bestDx;
            int offsetY = y + bestDy;

            if (offsetX >= 0 && offsetX < baseImage->width && offsetY >= 0 && offsetY < baseImage->height) {
                int baseIndex = y * baseImage->width + x;
                int targetIndex = offsetY * baseImage->width + offsetX;
                alignedImage->data[baseIndex] = targetImage->data[targetIndex];
            }
            else {
                int baseIndex = y * baseImage->width + x;
                alignedImage->data[baseIndex].r = 0;
                alignedImage->data[baseIndex].g = 0;
                alignedImage->data[baseIndex].b = 0;
            }
        }
    }
}
