#include "raw_process.h"


using namespace std;
using namespace cv;


U32 time_print_prog_start = clock();
U32 time_print_prog_end;
U32 g_time_start;
U32 g_time_end;

G_CONFIG cfg = { 0 };
double f(double x, double y) {
	return (x/sqrt(x/y));  // 这里使用sin(sqrt(x^2 + y^2))
}

int main()
{
	// 设置图像大小和分辨率
	int width = 600;
	int height = 600;

	U64 max = 0;

	// 创建一个空的图像矩阵，存储每个像素的高度值
	Mat image = Mat::zeros(height, width, CV_8UC3);  // 图像大小和类型

	// 定义 x 和 y 的范围，这里选择 [-3, 3] 作为 x 和 y 的范围
	double min_val = -30, max_val = 30;
	double step = 1;  // 根据图像的宽度来决定步长

	// 遍历 x 和 y 的所有值，计算对应的 z = f(x, y)
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			// 计算对应的 x 和 y
			double x = min_val + i * step;
			double y = min_val + j * step;

			// 计算 f(x, y)
			double z = f(x, y);
			max = calc_max(max, z);

}
	}

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			// 计算对应的 x 和 y
			double x = min_val + i * step;
			double y = min_val + j * step;

			// 计算 f(x, y)
			double z = f(x, y);
			int color = z / max * 255.0;
			image.at<Vec3b>(j, i) = Vec3b(color, color, color);  // 设置灰度图像
		}
	}

	// 显示图像
	imshow("2D Function Surface", image);
	waitKey(0);

	return 0;
}
