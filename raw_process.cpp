#include "raw_process.h"


using namespace std;
using namespace cv;


U32 time_print_prog_start = clock();
U32 time_print_prog_end;
U32 g_time_start;
U32 g_time_end;

G_CONFIG cfg = { 0 };

void load_cfg()
{
	//cfg.width = 2592;
	//cfg.height = 1536;
	return;
}

// 定义冻结函数，接收多帧图片并输出冻结效果图
void freeze(vector<Mat>& frames, Mat& result)
{
	// 将每帧图片转换为灰度图像，便于计算光流场
	vector<Mat> grayframe;
	for (const auto& frame : frames)
	{
		Mat gray;
		cvtColor(frame, gray, COLOR_BGR2GRAY); // 转为灰度
		grayframe.push_back(gray); // 保存灰度图
	}

	// 以第一帧灰度图作为基准帧，后续帧需对齐到该帧
	Mat baseframe = grayframe[0];
	vector<Mat> alignedframe; // 保存对齐后的帧
	alignedframe.push_back(frames[0]); // 第一帧直接加入对齐帧列表

	// 对其余帧进行光流计算和配准
	for (size_t i = 1; i < grayframe.size(); ++i)
	{
		Mat flow, warped;
		// 使用Farneback方法计算光流场，获取当前帧到基准帧的位移信息
		calcOpticalFlowFarneback(baseframe, grayframe[i], flow,
			0.5, 3, 15, 3, 5, 1.2, 0);

		// 根据光流场生成像素映射表
		Mat mapX(baseframe.size(), CV_32FC1); // X坐标映射表
		Mat mapY(baseframe.size(), CV_32FC1); // Y坐标映射表
		for (int y = 0; y < baseframe.rows; ++y)
		{
			for (int x = 0; x < baseframe.cols; ++x)
			{
				Point2f f = flow.at<Point2f>(y, x); // 光流位移
				mapX.at<float>(y, x) = x + f.x;    // 映射到X坐标
				mapY.at<float>(y, x) = y + f.y;    // 映射到Y坐标
			}
		}
		 
		// 根据映射表对当前帧进行重映射（即配准）
		remap(frames[i], warped, mapX, mapY, INTER_LINEAR); // 使用双线性插值
		alignedframe.push_back(warped); // 保存配准结果
	}

	// 对齐后的所有帧进行像素融合，计算平均值生成冻结效果
	Mat fused = Mat::zeros(frames[0].size(), CV_32FC3); // 初始化为全零图像（32位浮点）

	for (const auto& img : alignedframe)
	{
		Mat temp;
		img.convertTo(temp, CV_32FC3); // 转为32位浮点以便参与累加
		fused += temp; // 累加像素值
	}

	// 计算平均值，完成像素融合
	fused /= static_cast<float>(alignedframe.size());
	fused.convertTo(result, CV_8UC3); // 转回8位无符号整型，作为最终结果
}

int main()
{
	vector<Mat> images;

	// 读取一系列图片，保存到矢量容器中
#if 0
	images.push_back(imread("data\\2.bmp"));
	images.push_back(imread("data\\3.bmp"));
	images.push_back(imread("data\\4.bmp"));
	images.push_back(imread("data\\5.bmp"));
	images.push_back(imread("data\\6.bmp"));
	images.push_back(imread("data\\7.bmp"));
	images.push_back(imread("data\\8.bmp"));
	images.push_back(imread("data\\9.bmp"));
	images.push_back(imread("data\\10.bmp"));
	images.push_back(imread("data\\11.bmp"));
	images.push_back(imread("data\\12.bmp"));
	images.push_back(imread("data\\13.bmp"));
	images.push_back(imread("data\\14.bmp"));
	images.push_back(imread("data\\15.bmp"));
	images.push_back(imread("data\\16.bmp"));
	images.push_back(imread("data\\17.bmp"));
	images.push_back(imread("data\\18.bmp"));
#endif

#if 1
	images.push_back(imread("data2\\1.bmp"));
	images.push_back(imread("data2\\2.bmp"));
	images.push_back(imread("data2\\3.bmp"));
	images.push_back(imread("data2\\4.bmp"));
	images.push_back(imread("data2\\5.bmp"));
	images.push_back(imread("data2\\6.bmp"));
	images.push_back(imread("data2\\7.bmp"));
	images.push_back(imread("data2\\8.bmp"));
	images.push_back(imread("data2\\9.bmp"));
	images.push_back(imread("data2\\10.bmp"));
	images.push_back(imread("data2\\11.bmp"));

#endif


	Mat result; // 用于存储冻结效果图
	clock_t start = clock(); // 开始计时
	freeze(images, result); // 调用冻结函数
	clock_t end = clock(); // 结束计时
	cout << "Time: " << (double)(end - start) << "ms" << endl; // 输出耗时

	// 显示并保存结果
	imshow("result", result); // 显示冻结效果图
	imwrite("result.bmp", result); // 保存冻结效果图
	waitKey(0); // 等待按键退出

	return 0;
}



RGB* yyy2rgb_process(YUV* yuv, IMG_CONTEXT context, G_CONFIG cfg)
{
	if (!yuv) {
		std::cerr << "Invalid input parameters!" << std::endl;
		return NULL; // 错误代码
	}

	RGB* rgb = (RGB*)calloc(context.height * context.width, sizeof(RGB));

	RGB* tmp = rgb;
	for (U32 i = 0; i < context.full_size; ++i) 
	{
		yuv[i].y = clp_range(0, yuv[i].y, 255);
		tmp->r = yuv[i].y;
		tmp->g = yuv[i].y;
		tmp->b = yuv[i].y;
		tmp++;
	}
	LOG("done.");

	return rgb;
}


U8 save_rgb(const char* filename, RGB* rgb_data, IMG_CONTEXT context, G_CONFIG cfg) {
	// 创建 OpenCV Mat 对象
	int width = context.width;
	int height = context.height;
	cv::Mat img(height, width, CV_8UC3);

	// 填充 Mat 数据
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGB pixel = rgb_data[y * width + x];
			img.at<cv::Vec3b>(y, x) = cv::Vec3b(pixel.b, pixel.g, pixel.r);
		}
	}

	// 保存到文件
	if (cv::imwrite(filename, img)) {
		return 1; // 保存成功
	}
	else {
		return 0; // 保存失败
	}
}

void safe_free(void* p)
{
	if (NULL != p)
	{
		free(p);
		p = NULL;
	}
	return;
}

void print_prog(U32 cur_pos, U32 tgt)
{
	time_print_prog_end = clock();
	if ((time_print_prog_end - time_print_prog_start) >= 1000)
	{
		LOG("Processing: %d%%.", cur_pos * 100 / tgt);
		time_print_prog_start = clock();
	}
	return;
}

RGB* load_bmp(const char* filename, IMG_CONTEXT* context)
{
	FILE* f_in = fopen(filename, "rb");
	if (f_in == NULL)
	{
		LOG("Cannot find %s", filename);
		return NULL;
	}

	fread(&context->fileHeader, sizeof(BITMAPFILEHEADER), 1, f_in);
	fread(&context->infoHeader, sizeof(BITMAPINFOHEADER), 1, f_in);

	context->height = context->infoHeader.biHeight;
	context->width = context->infoHeader.biWidth;

	//context->w_samp = context->width / cfg.sample_ratio;
	//context->h_samp = context->height / cfg.sample_ratio;


	int LineByteCnt = (((context->width * context->infoHeader.biBitCount) + 31) >> 5) << 2;
	//int ImageDataSize = LineByteCnt * height;
	context->PaddingSize = 4 - ((context->width * context->infoHeader.biBitCount) >> 3) & 3;
	context->pad = (BYTE*)malloc(sizeof(BYTE) * context->PaddingSize);
	RGB* img = (RGB*)malloc(sizeof(RGB) * context->height * context->width);

	if (context->infoHeader.biBitCount == 24) {
		for (int i = 0; i < context->height; i++) {
			for (int j = 0; j < context->width; j++) {
				int index = i * context->width + j;
				fread(&img[index], sizeof(RGB), 1, f_in);
			}
			if (context->PaddingSize != 0)
			{
				fread(context->pad, 1, context->PaddingSize, f_in);
			}
		}
	}
	else
	{
		LOG("Only support BMP in 24-bit.");
		return NULL;
	}

	fclose(f_in);
	return img;
}

void save_bmp(const char* filename, RGB* img, IMG_CONTEXT* context)
{
	FILE* f_out = fopen(filename, "wb");

	context->fileHeader.bfType = 0x4D42; // 'BM'
	context->fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	context->fileHeader.bfSize = context->fileHeader.bfOffBits + context->width * context->height * sizeof(RGB);
	context->fileHeader.bfReserved1 = 0;
	context->fileHeader.bfReserved2 = 0;

	context->infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	context->infoHeader.biWidth = context->width;
	context->infoHeader.biHeight = context->height;
	context->infoHeader.biPlanes = 1;
	context->infoHeader.biBitCount = 24;
	context->infoHeader.biCompression = 0;
	context->infoHeader.biSizeImage = context->width * context->height * sizeof(RGB);
	context->infoHeader.biXPelsPerMeter = 0;
	context->infoHeader.biYPelsPerMeter = 0;
	context->infoHeader.biClrUsed = 0;
	context->infoHeader.biClrImportant = 0;

	fwrite(&context->fileHeader, sizeof(context->fileHeader), 1, f_out);
	fwrite(&context->infoHeader, sizeof(context->infoHeader), 1, f_out);

	for (int i = 0; i < context->height; i++)
	{
		for (int j = 0; j < context->width; j++)
		{
			fwrite(&img[i * context->width + j], sizeof(RGB), 1, f_out);
		}
		if (context->PaddingSize != 0)
		{
			fwrite(context->pad, 1, context->PaddingSize, f_out);
		}
	}
	fclose(f_out);
	return;
}