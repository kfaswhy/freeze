#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

void freeze(vector<Mat>& frames, Mat& result)
{
	//转灰度
	vector<Mat> grayframe;
	for(const auto& frame : frames)
	{
		Mat gray;
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		grayframe.push_back(gray);
	}

	//计算光流场
	Mat baseframe = grayframe[0];//选取第一帧为基准帧
	vector<Mat> alignedframe;
	alignedframe.push_back(frames[0]);

	for (size_t i = 1; i < grayframe.size(); ++i)
	{	
		Mat flow, warped;
		calcOpticalFlowFarneback(baseframe, grayframe[i], flow,
		0.5, 3, 15, 3, 5, 1.2, 0);

		//像素映射
		Mat mapX(baseframe.size(), CV_32FC1);
		Mat mapY(baseframe.size(), CV_32FC1);
		for (int y = 0; y < baseframe.rows; ++y)
		{
			for (int x = 0; x < baseframe.cols; ++x)
			{
				Point2f f = flow.at<Point2f>(y, x);//光流位移
				mapX.at<float>(y, x) = x + f.x;
				mapY.at<float>(y, x) = y + f.y;
			}
		}

		//配准
		remap(frames[i], warped, mapX, mapY, INTER_LINEAR);
		alignedframe.push_back(warped);

		imshow("warped", warped);
		waitKey(0);

	}

	//像素融合
	Mat fused = Mat::zeros(frames[0].size(), CV_32FC3);

	for (const auto& img : alignedframe)
	{
		Mat temp;
		img.convertTo(temp, CV_32FC3);
		fused += temp;
	}

	fused /= static_cast<float>(alignedframe.size());
	fused.convertTo(result, CV_8UC3); 
}

int main()
{
	vector<Mat> images;

	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\2.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\3.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\4.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\5.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\6.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\7.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\8.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\9.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\10.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\11.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\12.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\13.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\14.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\15.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\16.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\17.bmp"));
	images.push_back(imread("C:\\Users\\应凯\\Desktop\\Camera Roll2\\18.bmp"));

	Mat result;
	clock_t start = clock();
	freeze(images, result);
	clock_t end = clock();
	cout << "Time: " << (double)(end - start) << "ms" << endl;
	imshow("result", result);
	imwrite("C:\\Users\\应凯\\Desktop\\Camera Roll2\\result.bmp", result);
	waitKey(0);

	return 0;
}