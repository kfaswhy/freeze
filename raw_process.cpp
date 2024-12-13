#include "raw_process.h"


using namespace std;
using namespace cv;


U32 time_print_prog_start = clock();
U32 time_print_prog_end;
U32 g_time_start;
U32 g_time_end;

G_CONFIG cfg = { 0 };
IMG_CONTEXT ctx = { 0 };

void load_cfg()
{
	cfg.img_num = 2;
	cfg.search_range = 10;
	cfg.search_step = 1;
	return;
}

int main()
{

	load_cfg();

	const char* file_name[] = {
		"data3//1.bmp" ,
		"data3//7.bmp" ,
		"data3//3.bmp" ,
		"data3//4.bmp" ,
		"data3//5.bmp" 
	};

	/* 创建并读取图像队列 */
	RGB** img_q = (RGB**)malloc(cfg.img_num* sizeof(RGB*));
	for (int i = 0; i < cfg.img_num; i++)
	{
		img_q[i] = load_bmp(file_name[i], &ctx);
	}


	for (int i = 1; i < cfg.img_num; i++)
	{
		img_match(img_q[0], img_q[i], ctx, cfg);
	}



	save_bmp("tttttt.bmp", img_q[1], &ctx);

	return 0;
}
// 计算两个图像的 SAD 值，返回匹配度
long long calculate_sad(RGB* plate, RGB* img, IMG_CONTEXT context, int dx, int dy) {
	int width = context.width;
	int height = context.height;
	U64 sad = 0;
	int count = 0;

	for (int y = height / 2 - 200; y < height / 2 + 200; y++) {
		for (int x = width / 2 - 200; x < width / 2 + 200; x++) {
			int img_x = x + dx;
			int img_y = y + dy;

			// 检查是否越界
			if (img_x < 0 || img_x >= width || img_y < 0 || img_y >= height) {
				continue;
			}

			// 获取plate和img对应像素值
			RGB* p_pixel = &plate[y * width + x];
			RGB* i_pixel = &img[img_y * width + img_x];

			// 转换为灰度值
			int plate_gray = (p_pixel->r + p_pixel->g + p_pixel->b) / 3;
			int img_gray = (i_pixel->r + i_pixel->g + i_pixel->b) / 3;

			// 计算灰度值绝对差
			sad += plate_gray * img_gray;
			count++;
		}
	}

	return sad; // 如果没有有效像素，返回极大值
}


// 图像匹配主函数
void img_match(RGB* plate, RGB* img, IMG_CONTEXT context, G_CONFIG cfg) {
	int width = context.width;
	int height = context.height;

	int max_dx = height * cfg.search_range / 100;
	int max_dy = width * cfg.search_range / 100;
	int step = cfg.search_step;

	U64 min_sad = U64MAX;
	int best_dx = 0;
	int best_dy = 0;

	U64 max_sad = 0;

	RGB* sad_img = (RGB*)calloc(context.full_size, sizeof(RGB));



	// 遍历所有偏移范围
	for (int dy = -max_dy; dy <= max_dy; dy += step) {
		for (int dx = -max_dx; dx <= max_dx; dx += step) {
			U64 sad = calculate_sad(plate, img, context, dx, dy);

			if (sad > max_sad) {
				max_sad = sad;
				best_dx = dx;
				best_dy = dy;
			}

			if (sad < min_sad)
			{
				min_sad = sad;
			}

			sad_img[(dy + max_dy) * width + dx + max_dx].g = clp_range(0, (sad- 1642854614) * 255 / (1834004309 - 1642854614), 255);
		}

	}


	save_bmp("sad_img.bmp", sad_img, &context);



	// 打印最佳匹配位置
	printf("(%d, %d),[%u,%u]\n", best_dx, best_dy, min_sad,max_sad);

	// 偏移图像并保存（同原实现）
	RGB* shifted_img = (RGB*)malloc(context.full_size * sizeof(RGB));
	memset(shifted_img, 0, context.full_size * sizeof(RGB));

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int img_x = x + best_dx;
			int img_y = y + best_dy;

			if (img_x >= 0 && img_x < width && img_y >= 0 && img_y < height) {
				shifted_img[y * width + x].r = img[img_y * width + img_x].r;
				shifted_img[y * width + x].g = img[img_y * width + img_x].g;
				shifted_img[y * width + x].b = img[img_y * width + img_x].b;
			}


		}
	}

	save_bmp("shifted_img.bmp", shifted_img, &context);


	free(shifted_img);
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

float fast_sqrt(float number) {
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long*)&y;                       // 将 float 解释为 long 类型
	i = 0x5f3759df - (i >> 1);            // 魔术数字
	y = *(float*)&i;
	y = y * (threehalfs - (x2 * y * y));  // 近似值调整

	return 1.0f / y;
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
	context->full_size = context->height * context->width;
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