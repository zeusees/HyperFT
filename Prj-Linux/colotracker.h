#ifndef COLOTRACKER_H
#define COLOTRACKER_H

//#include "cv.h"
//#include "highgui.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <iostream>

using namespace std;
using namespace cv;


class ColorTracker
{
private:

    double bbox_x;
    double bbox_y;
    double bbox_width;
    double bbox_height;

    double xmean = 0;
    double ymean = 0;
    float  median;

    int npoints = 64;				// Number of feature points
	int pcount;						// Number of passed points that finally track

    //IplImage *frame;
    //IplImage *frame1_1C = NULL,*frame2_1C = NULL;
    cv::Mat frame;
    cv::Mat frame1_1C,frame2_1C;


	int winsize = 5;
    CvPoint pta,ptb;		

	// Init methods
    double euclid_dist(const Point2f* point1, const Point2f* point2);
    void   pairwise_dist(const Point2f* features, double *edist, int npoin);
    void   ncc_filter(cv::Mat frame1, cv::Mat frame2, Point2f *prev_feat, Point2f *curr_feat, int npoin, int method, cv::Mat rec0, cv::Mat rec1, cv::Mat res, int *ncc_pass);
    void   fb_filter(const Point2f* prev_features, const Point2f* backward_features, const Point2f* curr_feat, int *fb_pass, const int npoin);
    void   bbox_move(const Point2f* prev_feat,     const Point2f* curr_feat, const int npoin,double &xmean, double &ymean);

public:
    cv::Rect FirstRect;
    cv::Mat  FirstMat;
    void   init(cv::Mat & img);
    bool lost =false;	
    //frame-to-frame object tracking
    cv::Rect track(cv::Mat & img,cv::Mat before,double x1, double y1, double x2, double y2);
    
    inline cv::Rect track(cv::Mat & img,cv::Mat before)
    {
        return track(img,before,bbox_x, bbox_y,bbox_x+bbox_width, bbox_y+bbox_height);
    }
};

#endif // COLOTRACKER_H
