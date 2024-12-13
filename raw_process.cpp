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
	cfg.img_num = 10;
	return;
}

// 用于堆栈中间值去噪的函数
Mat medianStackDenoise(const vector<Mat>& images) {
	Mat result(images[0].size(), images[0].type()); // 创建结果图像，与输入图像的尺寸和类型相同

	// 遍历每个像素的所有图像，取中位值
	for (int i = 0; i < images[0].rows; i++) { // 遍历每一行
		for (int j = 0; j < images[0].cols; j++) { // 遍历每一列
			vector<Vec3b> pixelValues; // 存储该像素点在所有图像中的 RGB 值
			for (const auto& img : images) {
				pixelValues.push_back(img.at<Vec3b>(i, j)); // 将每张图像中对应像素的 RGB 值加入数组
			}

			// 自定义比较函数，按 RGB 三个通道的值进行排序
			sort(pixelValues.begin(), pixelValues.end(), [](const Vec3b& a, const Vec3b& b) {
				for (int k = 0; k < 3; k++) { // 比较 RGB 三个通道的值
					if (a[k] != b[k]) {
						return a[k] < b[k]; // 若有不同，则按通道值大小排序
					}
				}
				return false; // 如果 RGB 三个通道值相同，保持顺序
				});

			result.at<Vec3b>(i, j) = pixelValues[pixelValues.size() / 2]; // 取排序后的中位值作为结果像素值
		}
	}
	return result; // 返回去噪后的图像
}

// 图像配准函数（基于模板匹配）
Mat alignImages(const Mat& baseImage, const Mat& targetImage, int searchRange) {
	Mat result;

	// 局部模板匹配（假设目标图像的位移较小，限制匹配范围在图像的中心区域）
	int width = baseImage.cols / 2; // 计算中心区域宽度
	int height = baseImage.rows / 2; // 计算中心区域高度

	Mat targetROI = targetImage(Rect(width / 4, height / 4, width, height)); // 选择目标图像的中心区域
	Mat resultMat; // 保存模板匹配结果的矩阵

	// 使用模板匹配
	matchTemplate(targetROI, baseImage, resultMat, TM_CCOEFF_NORMED); // 使用相关系数归一化方法进行模板匹配
	Point maxLoc;
	minMaxLoc(resultMat, nullptr, nullptr, nullptr, &maxLoc); // 获取匹配结果中响应值最大的点

	// 计算平移矩阵并对齐图像
	Mat translationMat = (Mat_<float>(2, 3) << 1, 0, maxLoc.x, 0, 1, maxLoc.y); // 平移矩阵
	warpAffine(targetImage, result, translationMat, baseImage.size()); // 对目标图像进行平移变换，使其对齐基准图像

	return result; // 返回对齐后的图像
}

int main() {
	vector<Mat> images; // 用于存储输入图像
	string folderPath = "data3/"; // 图像文件夹路径

	// 读取 5 张图像
	for (int i = 1; i <= 5; i++) {
		string imagePath = folderPath + to_string(i) + ".bmp"; // 构造图像路径
		Mat img = imread(imagePath); // 读取图像
		if (img.empty()) { // 检查图像是否成功读取
			cerr << "无法读取图像: " << imagePath << endl; // 输出错误信息
			return -1; // 返回错误码
		}
		images.push_back(img); // 将读取的图像加入图像数组
	}

	// 对图片进行对齐
	vector<Mat> alignedImages; // 存储对齐后的图像
	alignedImages.push_back(images[0]); // 第一张图像作为基准图像
	for (int i = 1; i < images.size(); i++) {
		Mat aligned = alignImages(images[0], images[i], images[i].cols / 10); // 对齐其他图像
		alignedImages.push_back(aligned); // 将对齐后的图像加入数组
	}

	// 堆栈中间值去噪
	Mat denoisedImage = medianStackDenoise(alignedImages); // 对齐后的图像进行去噪

	// 保存去噪后的结果
	imwrite("denoised_image.bmp", denoisedImage); // 将去噪结果保存为图像文件

	cout << "图像处理完成，去噪后的图像已保存为 denoised_image.bmp" << endl; // 输出完成信息

	return 0; // 返回成功码
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