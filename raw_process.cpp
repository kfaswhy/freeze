#include "raw_process.h"

using namespace std;

const int height = 900;
const int width = 1112;

const int total_num = 5; //图像总数量
const int img_num = 5; //筛选出的图像数量
const int search_range = 15; // 模板匹配的最大范围

const int search_step = 1;  //模板匹配时移动的步长
const float search_area = 0.5; //计算匹配代价函数时的取样范围比例
const int search_sample = 3; //计算匹配代价函数时的下采样强度，1表示不做下采样

// 要处理的图像文件路径数组
const char* fileNames[] = {
    "data/1.bmp",
    "data/2.bmp",
    "data/3.bmp",
    "data/4.bmp",
    "data/5.bmp",
    "data/6.bmp",
    "data/7.bmp",
    "data/8.bmp",
    "data/9.bmp",
    "data/10.bmp",
    "data/11.bmp",
    "data/12.bmp",
    "data/13.bmp",
    "data/14.bmp",
    "data/15.bmp",
    "data/16.bmp",
    "data/17.bmp",
    "data/18.bmp",
    "data/19.bmp",
    "data/20.bmp"
};

int main() 
{
    int j = 0;
    int t0 = clock();
    select_images();
    int t1 = clock();


    U8** img = (U8**)malloc(img_num * sizeof(U8*));

    for (int i = 0; i < img_num; ++i)
    {
        img[i] = read_img(fileNames[i]);
    }

    

    // 对齐图像
    U8** alignedImages = (U8**)malloc(img_num * sizeof(U8*));
    alignedImages[0] = img[0];
    for (int i = 1; i < img_num; ++i) 
    {
        alignedImages[i] = alignImages(img[0], img[i]);
    }

    //时域中值滤波
    U8 *denoisedImage= medianStackDenoise(alignedImages);


    
    LOG("time_used = %.02f. s", ((float)t1 - t0) / 1000);

    save_img("denoisedImage.jpg", denoisedImage);

    return 0;
}

void select_images()
{
    U8** all_img = (U8**)malloc(total_num * sizeof(U8*));
    double *clarity = (double*)malloc(total_num * sizeof(double));
    double max_clarity = 0.0;
    U8 max_index = 0;

    for (int i = 0; i < total_num; ++i)
    {
        all_img[i] = read_img(fileNames[i]);
        clarity[i] = getClarityEvaluation(all_img[i]);
        if(clarity[i] > max_clarity)
        {
            max_clarity = clarity[i];
            max_index = i;
        }

        LOG("%s: %.03f.", fileNames[i], clarity[i]);
    }

    LOG("max: %s, clarify = %.3f ", fileNames[max_index], max_clarity);

    U8* index_select = (U8*)malloc(img_num * sizeof(U8));
    index_select[0] = max_index;
	U8 num_selected = 1; //已选择的图像数量

    //向后查找
    U8 is_stop = 0; //查找停止标记
    U8 is_skip = 0; //跳过标记
    U8 back = max_index + 1; //序列终点
    while (is_stop == 0)
    {
        if (num_selected >= img_num || back >= total_num)
        {
            is_stop = 1;
            //LOG("num_selected = %d, back = %d. break.", num_selected, back);
            break;
        }

        if (is_skip == 1)
        {
            back++;
        }
        
        //LOG("----------------------------");
        //LOG("back = %d. ", back);
        if (is_ok(max_clarity, clarity[back]))
        {
            //LOG("max = %.3f, clarity[%d] = %.3f. OK!", max_clarity, back, clarity[back]);
            index_select[num_selected] = back;
            num_selected++;
            back++;
        }
        else
        {
            //LOG("max = %.3f, clarity[%d] = %.3f. FAILED!!", max_clarity, back, clarity[back]);
            //LOG("skip = %d.", is_skip);
            if (!is_skip) //如果此前未跳过
            {
                is_skip = 1;
                back++;
            }
            else //如果跳过一次
            {
                is_stop = 1;
                break;
            }
        }
    }

    //向后查找
    is_stop = 0; //查找停止标记
    is_skip = 0; //跳过标记
    U8 forward = max_index - 1; //前序起点
    while (is_stop == 0)
    {
        if (num_selected >= img_num || forward >= total_num)
        {
            is_stop = 1;
            //LOG("num_selected = %d, forward = %d. break.", num_selected, forward);
            break;
        }

        if (is_skip == 1)
        {
            forward--;
        }

        //LOG("----------------------------");
        //LOG("forward = %d. ", forward);
        if (is_ok(max_clarity, clarity[forward]))
        {
            //LOG("max = %.3f, clarity[%d] = %.3f. OK!", max_clarity, forward, clarity[forward]);
            index_select[num_selected] = forward;
            num_selected++;
            forward--;
        }
        else
        {
            //LOG("max = %.3f, clarity[%d] = %.3f. FAILED!!", max_clarity, forward, clarity[forward]);
            //LOG("skip = %d.", is_skip);
            if (!is_skip) //如果此前未跳过
            {
                is_skip = 1;
                forward--;
            }
            else //如果跳过一次
            {
                is_stop = 1;
                break;
            }
        }
    }

   


    for (int i = 0; i < num_selected; i++)
    {
        LOG("%s: %.03f.", fileNames[index_select[i]], clarity[index_select[i]]);
    }

}

U8 is_ok(double base, double test)
{
    
    return (test >= (base - 1));
}

U8* read_img(const char* file_name) {
    // 使用OpenCV加载图像
    cv::Mat img = cv::imread(file_name, cv::IMREAD_COLOR);  // 读取为彩色图像

    if (img.empty()) {
        printf("Error loading image: %s\n", file_name);
        return NULL;
    }

    // 检查图像大小是否匹配（根据需要调整）
    if (img.cols != width || img.rows != height) {
        printf("Image size mismatch: expected %dx%d, got %dx%d\n", width, height, img.cols, img.rows);
        return NULL;
    }

    // 将OpenCV Mat图像转换为RGB格式的U8数组
    U8* img_data = (U8*)malloc(width * height * 3 * sizeof(U8));  // 3通道RGB图像

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // 获取OpenCV图像的像素
            cv::Vec3b pixel = img.at<cv::Vec3b>(i, j);
            img_data[(i * width + j) * 3 + 0] = pixel[2];  // 红色
            img_data[(i * width + j) * 3 + 1] = pixel[1];  // 绿色
            img_data[(i * width + j) * 3 + 2] = pixel[0];  // 蓝色
        }
    }

    return img_data;  // 返回指向图像数据的指针
}

U8* save_img(const char* file_name, U8* img) 
{
    // 创建一个空的 Mat 对象，表示要保存的图像（BGR顺序）
    cv::Mat output_img(height, width, CV_8UC3);  // CV_8UC3 表示 3 通道的 8 位图像

    // 将 RGB 数据转换为 BGR 数据，OpenCV 默认是 BGR 顺序
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // 获取图像数据
            int idx = (i * width + j) * 3;
            // 赋值到 Mat 图像
            output_img.at<cv::Vec3b>(i, j)[0] = img[idx + 2];  // B
            output_img.at<cv::Vec3b>(i, j)[1] = img[idx + 1];  // G
            output_img.at<cv::Vec3b>(i, j)[2] = img[idx + 0];  // R
        }
    }

    // 使用 OpenCV 保存图像
    bool success = cv::imwrite(file_name, output_img);  // 保存为文件

    if (!success) {
        printf("Error saving image: %s\n", file_name);
        return NULL;
    }

    printf("Image saved successfully to %s\n", file_name);
    return img;  // 返回输入的图像数据
}

// 中值堆叠去噪：对多张图像进行去噪处理，取每个像素点在所有图像中对应通道的中值
U8* medianStackDenoise(U8** images)
{

    U8* output = (U8*)malloc(width * height * 3 * sizeof(U8));
    int index = 0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            //U8 red[5], green[5], blue[5];

            U8* red = (U8*)malloc(img_num * sizeof(U8));
            U8* green = (U8*)malloc(img_num * sizeof(U8));
            U8* blue = (U8*)malloc(img_num * sizeof(U8));

            // 获取每张图像对应像素点的RGB值
            for (int k = 0; k < img_num; ++k) {
                red[k] = images[k][index * 3];
                green[k] = images[k][index * 3 + 1];
                blue[k] = images[k][index * 3 + 2];
            }

            // 对RGB通道分别进行排序
            for (int m = 0; m < img_num; ++m) {
                for (int n = m + 1; n < img_num; ++n) {
                    if (red[m] > red[n]) { U8 temp = red[m]; red[m] = red[n]; red[n] = temp; }
                    if (green[m] > green[n]) { U8 temp = green[m]; green[m] = green[n]; green[n] = temp; }
                    if (blue[m] > blue[n]) { U8 temp = blue[m]; blue[m] = blue[n]; blue[n] = temp; }
                }
            }
            output[index * 3] = red[img_num / 2];  // 取中值
            output[index * 3 + 1] = green[img_num / 2];
            output[index * 3 + 2] = blue[img_num / 2];

            index++;
        }
    }
    return output;
}

// 对齐图像：通过搜索最佳偏移量使目标图像与基准图像对齐
U8* alignImages(U8* baseImage, U8* targetImage) {
    int bestDx = 0, bestDy = 0;
    int minError = INT_MAX;

    //int centerX = width / 2;
    //int centerY = height / 2;

    const int y0 = (height >> 1) * (1 - search_area);
    const int y1 = (height >> 1) * (1 + search_area);
    const int x0 = (width >> 1) * (1 - search_area);
    const int x1 = (width >> 1) * (1 + search_area);
    




    // 遍历搜索范围内的所有偏移量，计算误差
    for (int dy = -search_range; dy <= search_range; ++dy) {
        for (int dx = -search_range; dx <= search_range; ++dx) {
            int error = 0;

            for (int y = y0; y < y1; y += search_sample) {
                for (int x = x0; x < x1; x += search_sample) {
                    int offsetX = x + dx;
                    int offsetY = y + dy;

                    if (offsetX >= 0 && offsetX < width && offsetY >= 0 && offsetY < height) {

                        int baseIndex = y * width + x;
                        int targetIndex = offsetY * width + offsetX;
                        int diffR = baseImage[3 * baseIndex] - targetImage[3 * targetIndex];
                        int diffG = baseImage[3 * baseIndex + 1] - targetImage[3 * targetIndex + 1];
                        int diffB = baseImage[3 * baseIndex + 2] - targetImage[3 * targetIndex + 2];

                        error += diffG * diffG + diffR * diffR + diffB * diffB;
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

    LOG("%d,%d", bestDx, bestDy);

    U8* alignedImage = (U8*)calloc(width * height * 3, sizeof(U8));


    int baseIndex = 0;
    // 应用最佳偏移量来对齐目标图像
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int offsetX = x + bestDx;
            int offsetY = y + bestDy;

            if (offsetX >= 0 && offsetX < width && offsetY >= 0 && offsetY < height)
            {
                int targetIndex = offsetY * width + offsetX;
                memcpy(&alignedImage[baseIndex * 3], &targetImage[targetIndex * 3], 3 * sizeof(U8));
            }
            baseIndex++;
            
        }
    }

    //save_img("xxxxx.jpg", alignedImage); 

    return alignedImage;
}



double getClarityEvaluation(uchar* data) 
{
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