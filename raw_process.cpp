#include "raw_process.h"

using namespace std;
using namespace cv;


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
	matchTemplate(targetROI, baseImage, resultMat, TM_CCORR_NORMED); // 使用相关系数归一化方法进行模板匹配
	Point maxLoc;
	minMaxLoc(resultMat, nullptr, nullptr, nullptr, &maxLoc); // 获取匹配结果中响应值最大的点

	//LOG("%d, %d.", maxLoc.x, maxLoc.y);

	// 计算平移矩阵并对齐图像
	Mat translationMat = (Mat_<float>(2, 3) << 1, 0, maxLoc.x - 139, 0, 1, maxLoc.y - 112); // 平移矩阵
	//这里的139和112应该是长宽的1/8
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

	int t0 = clock();


	// 对图片进行对齐
	vector<Mat> alignedImages; // 存储对齐后的图像
	alignedImages.push_back(images[0]); // 第一张图像作为基准图像
	for (int i = 1; i < images.size(); i++) {
		Mat aligned = alignImages(images[0], images[i], images[i].cols / 10); // 对齐其他图像
		alignedImages.push_back(aligned); // 将对齐后的图像加入数组
	}

	// 堆栈中间值去噪
	Mat denoisedImage = medianStackDenoise(alignedImages); // 对齐后的图像进行去噪
	
	int t1 = clock();
	LOG("time_used = %.02f.", ((float)t1 - t0) / 1000);

	// 保存去噪后的结果
	imwrite("denoised_image.bmp", denoisedImage); // 将去噪结果保存为图像文件

	cout << "图像处理完成，去噪后的图像已保存为 denoised_image.bmp" << endl; // 输出完成信息

	return 0; // 返回成功码
}