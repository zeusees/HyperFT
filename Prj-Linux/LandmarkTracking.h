#ifndef ZEUSEESFACETRACKING_H
#define ZEUSEESFACETRACKING_H
//#include <opencv2/opencv.hpp>
//#include <thread>
#include "mtcnn.h"
#include "time.h"
#include "colotracker.h"


cv::Rect boundingRect(const std::vector<cv::Point>& pts) {
	if (pts.size() > 1)
	{
		int xmin = pts[0].x;
		int ymin = pts[0].y;
		int xmax = pts[0].x;
		int ymax = pts[0].y;
		for (int i = 1; i < pts.size(); i++)
		{
			if (pts[i].x < xmin)
				xmin = pts[i].x;
			if (pts[i].y < ymin)
				ymin = pts[i].y;
			if (pts[i].x > xmax)
				xmax = pts[i].x;
			if (pts[i].y > ymax)
				ymax = pts[i].y;
		}
		return cv::Rect(xmin, ymin, xmax - xmin, ymax - ymin);
	}
}


//typedef int T;
//T i = 1;


class Face {
public:

	Face(int instance_id, Shape::Rect<float> rect) {
		face_id = instance_id;

		face_location = rect;
		isCanShow = false; //追踪一次后待框稳定后即可显示
		
	}

	Face() {

		isCanShow = false; //追踪一次后待框稳定后即可显示
	}

	Bbox faceBbox;

	int face_id = -1;
	long frameId = 0;
	int ptr_num = 0;

	Shape::Rect<float> face_location;
	bool isCanShow;
	cv::Mat frame_face_prev;

	static cv::Rect SquarePadding(cv::Rect facebox, int margin_rows, int margin_cols, bool max)
	{
		int c_x = facebox.x + facebox.width / 2;
		int c_y = facebox.y + facebox.height / 2;
		int large = 0;
		if (max)
			large = std::max(facebox.height, facebox.width) / 2;
		else
			large = min(facebox.height, facebox.width) / 2;
		cv::Rect rectNot(c_x - large, c_y - large, c_x + large, c_y + large);
		rectNot.x = std::max(0, rectNot.x);
		rectNot.y = std::max(0, rectNot.y);
		rectNot.height = min(rectNot.height, margin_rows - 1);
		rectNot.width = min(rectNot.width, margin_cols - 1);
		if (rectNot.height - rectNot.y != rectNot.width - rectNot.x)
			return SquarePadding(cv::Rect(rectNot.x, rectNot.y, rectNot.width - rectNot.x, rectNot.height - rectNot.y), margin_rows, margin_cols, false);

		return cv::Rect(rectNot.x, rectNot.y, rectNot.width - rectNot.x, rectNot.height - rectNot.y);
	}

	static cv::Rect SquarePadding(cv::Rect facebox, int padding)
	{

		int c_x = facebox.x - padding;
		int c_y = facebox.y - padding;
		return cv::Rect(facebox.x - padding, facebox.y - padding, facebox.width + padding * 2, facebox.height + padding * 2);;
	}

	static double getDistance(cv::Point x, cv::Point y)
	{
		return sqrt((x.x - y.x) * (x.x - y.x) + (x.y - y.y) * (x.y - y.y));
	}


	vector<vector<cv::Point> > faceSequence;
	vector<vector<float>> attitudeSequence;


};



class FaceTracking {
public:
	FaceTracking(string modelPath)
	{
		this->detector = new MTCNN(modelPath);
		downSimpilingFactor = 1;
		faceMinSize = 70;
		this->detector->SetMinFace(faceMinSize);
		detection_Time = -1;

	}

	~FaceTracking() {
		delete this->detector;

	}

	void detecting(cv::Mat* image) {
		ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(image->data, ncnn::Mat::PIXEL_BGR2RGB, image->cols, image->rows);
		std::vector<Bbox> finalBbox;
		if(isMaxFace)
			detector->detectMaxFace(ncnn_img, finalBbox);
		else
			detector->detect(ncnn_img, finalBbox);
		const int num_box = finalBbox.size();
		std::vector<cv::Rect> bbox;
		bbox.resize(num_box);
		candidateFaces_lock = 1;
		for (int i = 0; i < num_box; i++) {
			bbox[i] = cv::Rect(finalBbox[i].x1, finalBbox[i].y1, finalBbox[i].x2 - finalBbox[i].x1 + 1,
				finalBbox[i].y2 - finalBbox[i].y1 + 1);
			bbox[i] = Face::SquarePadding(bbox[i], image->rows, image->cols, true);
			Shape::Rect<float> f_rect(bbox[i].x / static_cast<float>(image->cols),
				bbox[i].y / static_cast<float>(image->rows),
				bbox[i].width / static_cast<float>(image->cols),
				bbox[i].height / static_cast<float>(image->rows)
			);
			std::shared_ptr<Face> face(new Face(trackingID, f_rect));
			(*image)(bbox[i]).copyTo(face->frame_face_prev);

			trackingID = trackingID + 1;
			candidateFaces.push_back(*face);
		}
		candidateFaces_lock = 0;
	}

	void Init(cv::Mat& image) {
		ImageHighDP = image;
		cv::Size lowDpSize(ImageHighDP.cols / downSimpilingFactor, ImageHighDP.rows / downSimpilingFactor);
		cv::resize(image, ImageLowDP, lowDpSize);
		trackingID = 0;
		detection_Interval = 200; //detect faces every 200 ms
		detecting(&image);
		stabilization = false;
		UI_height = image.rows;
		UI_width  = image.cols;
		lastImage =image.clone();
		MF_Tracker.init(image);
	}

	void doingLandmark_onet(cv::Mat& face, Bbox& faceBbox, int zeroadd_x, int zeroadd_y, int stable_state = 0) {
		ncnn::Mat in = ncnn::Mat::from_pixels_resize(face.data, ncnn::Mat::PIXEL_BGR, face.cols, face.rows, 48, 48);
		faceBbox = detector->onet(in, zeroadd_x, zeroadd_y, face.cols, face.rows);

	}


	void tracking_corrfilter(const cv::Mat& frame, const cv::Mat& model, cv::Rect& trackBox, float scale)
	{
		trackBox.x /= scale;
		trackBox.y /= scale;
		trackBox.height /= scale;
		trackBox.width /= scale;
		int zeroadd_x = 0;
		int zeroadd_y = 0;
		cv::Mat frame_;
		cv::Mat model_;
		cv::resize(frame, frame_, cv::Size(), 1 / scale, 1 / scale);
		cv::resize(model, model_, cv::Size(), 1 / scale, 1 / scale);
		cv::Mat gray;
		cvtColor(frame_, gray, cv::COLOR_RGB2GRAY);
		cv::Mat gray_model;
		cvtColor(model_, gray_model, cv::COLOR_RGB2GRAY);
		cv::Rect searchWindow;
		searchWindow.width  = trackBox.width  * 3;
		searchWindow.height = trackBox.height * 3;
		searchWindow.x = trackBox.x + trackBox.width * 0.5 - searchWindow.width * 0.5;
		searchWindow.y = trackBox.y + trackBox.height * 0.5 - searchWindow.height * 0.5;
		searchWindow &= cv::Rect(0, 0, frame_.cols, frame_.rows);
		cv::Mat similarity;
		matchTemplate(gray(searchWindow), gray_model, similarity, cv::TM_CCOEFF_NORMED);
		double mag_r;
		cv::Point point;
		minMaxLoc(similarity, 0, &mag_r, 0, &point);
		trackBox.x = point.x + searchWindow.x;
		trackBox.y = point.y + searchWindow.y;
		trackBox.x *= scale;
		trackBox.y *= scale;
		trackBox.height *= scale;
		trackBox.width *= scale;
	}

	bool tracking(cv::Mat& image, Face& face)
	{
		cv::Rect faceROI = face.face_location.convert_cv_rect(image.rows, image.cols);
		cv::Mat faceROI_Image;

		double t = (double)getTickCount();//开始时间
		//tracking_corrfilter(image, face.frame_face_prev, faceROI, tpm_scale);
		faceROI =MF_Tracker.track(image,lastImage,faceROI.x,faceROI.y,faceROI.x+faceROI.width,faceROI.y+faceROI.height);
		t = (double)getTickCount() - t;//代码运行时间=结束时间-开始时间

		//printf("互相关匹配时间= %gms\n", t*1000. / getTickFrequency());//转换时间单位并输出代码运行时间
        printf("光流时间= %gms\n", t*1000. / getTickFrequency());//转换时间单位并输出代码运行时间
		//lastImage = image;
		
		image(faceROI).copyTo(faceROI_Image);

		cv::Rect bdbox;

		doingLandmark_onet(faceROI_Image, face.faceBbox, faceROI.x, faceROI.y, face.frameId > 1);

		bdbox.x = face.faceBbox.x1;
		bdbox.y = face.faceBbox.y1;
		bdbox.width = face.faceBbox.x2 - face.faceBbox.x1;
		bdbox.height = face.faceBbox.y2 - face.faceBbox.y1;

		bdbox = Face::SquarePadding(bdbox, static_cast<int>(bdbox.height * -0.05));
		bdbox = Face::SquarePadding(bdbox, image.rows, image.cols, 1);

		Shape::Rect<float> boxfloat(bdbox.x / static_cast<float>(image.cols),
			bdbox.y / static_cast<float>(image.rows),
			bdbox.width / static_cast<float>(image.cols),
			bdbox.height / static_cast<float>(image.rows));

		face.faceBbox.x1 = bdbox.x;
		face.faceBbox.y1 = bdbox.y;
		face.faceBbox.x2 = bdbox.x + bdbox.width;
		face.faceBbox.y2 = bdbox.y + bdbox.height;


		face.face_location = boxfloat;
		faceROI = face.face_location.convert_cv_rect(image.rows, image.cols);

		image(faceROI).copyTo(face.frame_face_prev);
		face.frameId += 1;
		ncnn::Mat rnet_data = ncnn::Mat::from_pixels_resize(faceROI_Image.data, ncnn::Mat::PIXEL_BGR2RGB, faceROI_Image.cols, faceROI_Image.rows, 24, 24);
		
		float sim = detector->rnet(rnet_data);

		face.isCanShow = true;
		if (sim > 0.9) {
			//stablize
			float diff_x = 0;
			float diff_y = 0;
			return true;
		}
		return false;

	}
	void setMask(cv::Mat& image, cv::Rect& rect_mask)
	{

		int height = image.rows;
		int width = image.cols;
		cv::Mat subImage = image(rect_mask);
		subImage.setTo(0);
	}

	void update(cv::Mat& image)
	{
		ImageHighDP = image;
		//std::cout << trackingFace.size() << std::endl;
		if (candidateFaces.size() > 0 && !candidateFaces_lock)
		{
			for (int i = 0; i < candidateFaces.size(); i++)
			{
				trackingFace.push_back(candidateFaces[i]);
			}
			candidateFaces.clear();
		}
		for (vector<Face>::iterator iter = trackingFace.begin(); iter != trackingFace.end();)
		{
			if (!tracking(image, *iter))
			{
				iter = trackingFace.erase(iter); //追踪失败 则删除此人脸
			}
			else {
				iter++;
			}
		}

		if (trackingFace.size() <= 0)
		{
			detection_Interval = 200;
		}
		else
		{
			detection_Interval = 1000;
		}

		if (detection_Time < 0)
		{
			detection_Time = (double)cv::getTickCount();
		}
		else {
			double diff = (double)(cv::getTickCount() - detection_Time) * 1000 / cv::getTickFrequency();
			if (diff > detection_Interval)
			{
				cv::Size lowDpSize(ImageHighDP.cols / downSimpilingFactor, ImageHighDP.rows / downSimpilingFactor);
				cv::resize(image, ImageLowDP, lowDpSize);
				//set Mask to protect the tracking face not to be detected.
				for (auto& face : trackingFace)
				{
					Shape::Rect<float> rect = face.face_location;
					cv::Rect rect1 = rect.convert_cv_rect(ImageLowDP.rows, ImageLowDP.cols);
					setMask(ImageLowDP, rect1);
				}
				detection_Time = (double)cv::getTickCount();
				// do detection in thread
				detecting(&ImageLowDP);
			}

		}
	}



	vector<Face> trackingFace; //跟踪中的人脸
	int UI_width;
	int UI_height;


private:
    cv::Mat lastImage;
	int isLostDetection;
	int isTracking;
	int isDetection;
	cv::Mat ImageHighDP;
	cv::Mat ImageLowDP;
	int downSimpilingFactor;
	int faceMinSize;
	MTCNN* detector;
	ColorTracker MF_Tracker;
	vector<Face> candidateFaces; // 将检测到的人脸放入此列队 待跟踪的人脸
	bool candidateFaces_lock;
	double detection_Time;
	double detection_Interval;
	int trackingID;
	bool stabilization;
	int tpm_scale = 2;
	bool isMaxFace = true;
};
#endif //ZEUSEESFACETRACKING_H
