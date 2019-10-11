#ifndef __MTCNN_NCNN_H__
#define __MTCNN_NCNN_H__
#include "net.h"

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <time.h>
#include <algorithm>
#include <map>
#include <iostream>
using namespace std;

struct Bbox
{
	float score;
	int x1;
	int y1;
	int x2;
	int y2;
	float area;
	float ppoint[10];
	float regreCoord[4];
};

namespace Shape {

	template <typename T> class Rect {
	public:
		Rect() {}
		Rect(T x, T y, T w, T h) {
			this->x = x;
			this->y = y;
			this->width = w;
			height = h;

		}
		T x;
		T y;
		T width;
		T height;

		cv::Rect convert_cv_rect(int _height, int _width)
		{
			cv::Rect Rect_(static_cast<int>(x*_width), static_cast<int>(y*_height),
				static_cast<int>(width*_width), static_cast<int>(height*_height));
			return Rect_;
		}
	};
}

class MTCNN {

public:
	MTCNN(const string &model_path);
	MTCNN(const std::vector<std::string> param_files, const std::vector<std::string> bin_files);
	~MTCNN();

	void SetMinFace(int minSize);
	void detect(ncnn::Mat& img_, std::vector<Bbox>& finalBbox);
	void detectMaxFace(ncnn::Mat& img_, std::vector<Bbox>& finalBbox);
	float rnet(ncnn::Mat& img);
	Bbox onet(ncnn::Mat& img,int x, int y,int w,int h);
	ncnn::Net Pnet, Rnet, Onet;


	//void detection(const cv::Mat& img, std::vector<cv::Rect>& rectangles);
private:
	void generateBbox(ncnn::Mat score, ncnn::Mat location, vector<Bbox>& boundingBox_, float scale);
	void nmsTwoBoxs(vector<Bbox> &boundingBox_, vector<Bbox> &previousBox_, const float overlap_threshold, string modelname = "Union");
	void nms(vector<Bbox> &boundingBox_, const float overlap_threshold, string modelname = "Union");
	void refine(vector<Bbox> &vecBbox, const int &height, const int &width, bool square);
	void extractMaxFace(vector<Bbox> &boundingBox_);
	float iou(Bbox & b1, Bbox & b2, string modelname = "Union");
	void SmoothBbox(std::vector<Bbox>& winList);
	void PNet(float scale);
	void PNet();
	void RNet();
	void ONet();

	ncnn::Mat img;

	const float nms_threshold[3] = { 0.5f, 0.7f, 0.7f };
	const float mean_vals[3] = { 127.5, 127.5, 127.5 };
	const float norm_vals[3] = { 0.0078125, 0.0078125, 0.0078125 };
	const int MIN_DET_SIZE = 12;
	std::vector<Bbox> firstPreviousBbox_, secondPreviousBbox_, thirdPrevioussBbox_;
	std::vector<Bbox> firstBbox_, secondBbox_, thirdBbox_;
	int img_w, img_h;

private:
	const float threshold[3] = { 0.8f, 0.8f, 0.9f };
	int minsize = 30;
	const float pre_facetor = 0.7090f;
	bool smooth = true;//Œ»∂®–‘
};


#endif //__MTCNN_NCNN_H__
