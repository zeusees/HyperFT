#include "colotracker.h"
#include <iostream>
#include <fstream> 
#include <vector>
#include "opencv/cv.h"
#include "opencv/ml.h"
#include "opencv/highgui.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

#define Scale 6


/* -------------------------CODE-----------------------------*/
double ColorTracker::euclid_dist(const Point2f* point1, const Point2f* point2) 
{
	/*
	 * This function calculates the euclidean distance between 2 points
	*/
	double distance, xvec, yvec;
	xvec = point2->x - point1->x;
	yvec = point2->y - point1->y;
	distance = sqrt((xvec * xvec) + (yvec * yvec));
	return distance;
}



void ColorTracker::pairwise_dist(const Point2f* features, double *edist, int npoin) 
{
	/*
	 * calculate m x n euclidean pairwise distance matrix.
	*/
	for (int i = 0; i < npoin; i++) 
	{
		for (int j = 0; j < npoin; j++) 
		{
			int ind = npoin*i + j;
			edist[ind] = euclid_dist(&features[i],&features[j]);
		}
	}
}


void ColorTracker::ncc_filter(cv::Mat frame1, cv::Mat frame2, Point2f *prev_feat, Point2f *curr_feat, 
				int npoin, int method, cv::Mat rec0, cv::Mat  rec1, cv::Mat res, int *ncc_pass) 
{
	/*
	 * Normalized Cross Correlation Filter 	 
	*/
	
	int filt = npoin/2;
	vector<float> ncc_err (npoin,0.0);

	for (int i = 0; i < npoin; i++) 
	{
		//从原图像中的特征点周围提取一个感兴趣的矩形区域图像
		getRectSubPix( frame1, Size(11,11),prev_feat[i], rec0);
		getRectSubPix( frame2, Size(11,11),curr_feat[i], rec1 );
		//模板匹配函数
		//HI_MPI_IVE_NCC
		//对获取得到的两个矩形框区域互相关匹配
		matchTemplate( rec0,rec1, res, method );
		ncc_err[i] = res.at<float>(0,0); 
	}
	vector<float>err_copy (ncc_err);
	sort(ncc_err.begin(), ncc_err.end());
	//使用ncnn计算的互相关进行验证，过滤掉bounding box中质量最差的点的50%
	median = (ncc_err[filt]+ncc_err[filt-1])/2.;
	cout<<"median"<<median<<endl;
	for(int i = 0; i < npoin; i++) 
	{
		
		if (err_copy[i] >= median) 
		{
			ncc_pass[i] = 1;		
		}
		else 
		{
			ncc_pass[i] = 0;
		}
	}	
}

// tracking box mouse callback

int cmp( const void *a , const void *b ){ 
	return *(double *)a > *(double *)b ? 1 : -1; 
}


void ColorTracker::fb_filter(const Point2f* prev_features, const Point2f* backward_features, 
const Point2f* curr_feat, int *fb_pass, const int npoin) 
{
	/*
	 * This function implements forward-backward error filtering
	*/
	//vector<double> euclidean_dist (npoin,0.0);
	double euclidean_dist[npoin];
	double err_copy[npoin];

	memcpy(err_copy,euclidean_dist,sizeof(euclidean_dist));
	
	int filt = npoin/2;
	for(int i = 0; i < npoin; i++) 
	{
		//比较原始点和反向点之间的欧式距离
		euclidean_dist[i] = euclid_dist(&prev_features[i], &backward_features[i]);
        //cout<<"euclidean_dist[i]==="<<i<<"===="<<euclidean_dist[i]<<endl;
	}
	
	//vector<double> err_copy (euclidean_dist);
	//qsort(in,100,sizeof(in[0]),cmp)；
	qsort(euclidean_dist,npoin,sizeof(euclidean_dist[0]),cmp);
	//use the STL sort algorithm to filter results
	//对该欧式距离进行排序，因为理论上如果所有的点没有误差，那么点坐标的欧式距离应该为0
	//sort(euclidean_dist.begin(), euclidean_dist.end());
	double median = (euclidean_dist[filt]+euclidean_dist[filt-1])/2.;
	//对点进行排序后，认为大于中值距离的点是坏点，相当于一半的点偏离原始点，说明这些点此时光流跟踪失败
	for(int i = 0; i < npoin; i++) 
	{
		if (err_copy[i] <= median) 
		{
			fb_pass[i] = 1;		
		}
		else 
		{
			fb_pass[i] = 0;
		}
	}
}


void ColorTracker::bbox_move(const Point2f* prev_feat, const Point2f* curr_feat, const int npoin,
				double &xmean, double &ymean)
 {
	/*
	 * Calculate bounding box motion. 
	 */
	//计算的是x方向和y方向上的平均偏移量
	//vector<double> xvec (npoin,0.0);
	//vector<double> yvec (npoin,0.0);
	double xvec[npoin];
	double yvec[npoin];
	for (int i = 0; i < npoin; i++) 
	{
		xvec[i] = curr_feat[i].x - prev_feat[i].x;
		yvec[i] = curr_feat[i].y - prev_feat[i].y;
	}	
	qsort(xvec,npoin,sizeof(xvec[0]),cmp);
	qsort(yvec,npoin,sizeof(yvec[0]),cmp);
	//sort(xvec.begin(), xvec.end());
	//sort(yvec.begin(), yvec.end());
	
	xmean = xvec[npoin/2];
	ymean = yvec[npoin/2];		//The final mostion is that of the mean of all the points. 
}


void ColorTracker::init(cv::Mat & img)
{
	//保存第一帧图像
	resize(img,frame1_1C,Size(img.cols/Scale,img.rows/Scale),0,0,1);
	cvtColor(frame1_1C,frame1_1C,CV_BGRA2GRAY);
}


cv::Rect ColorTracker::track(cv::Mat & img,cv::Mat before,double x1, double y1, double x2, double y2)
{
	    bbox_x      =x1/Scale;
    	bbox_y      =y1/Scale;
    	bbox_width  =(x2-x1)/Scale;
    	bbox_height =(y2-y1)/Scale;

		double t = (double)getTickCount();//开始时间
		resize(img,frame2_1C,Size(img.cols/Scale,img.rows/Scale),0,0,1);
		cvtColor(frame2_1C,frame2_1C,CV_BGRA2GRAY);
		t = (double)getTickCount() - t;//代码运行时间=结束时间-开始时间
		printf("预处理时间= %gms\n", t*1000. / getTickFrequency());
		cv::Mat  rec0(winsize,winsize,CV_8UC1);
		cv::Mat  rec1(winsize,winsize,CV_8UC1);
		cv::Mat  res(1,1,CV_32FC1);
	
		/* This array will contain the locations of the points from frame 1 in frame 2. */
		vector<Point2f> frame1_features(npoints);
		vector<Point2f> frame2_features(npoints);
		vector<Point2f> FB_features(npoints);
	
		// The i-th element of this array will be non-zero if and only if the i-th feature of
	 	// frame 1 was found in frame 2.
	 
		char optical_flow_found_feature[npoints];    		//features in first frame
		char optical_flow_found_feature2[npoints];			//features in second frame
		float optical_flow_feature_error[npoints];			//error in Optical Flow 
		vector<int> fb_pass(npoints);						//Points that passed fb
		vector<int> ncc_pass(npoints);						//Points that passed ncc

        for(int i = 0;i<8;i++)
		{
			for(int j = 0;j<8;j++)
			{
				int l = i*8 + j;
				
				frame1_features[l].x = bbox_x + (bbox_width/8)*j  + (bbox_width/16);
				frame1_features[l].y = bbox_y + (bbox_height/8)*i + (bbox_height/16);
			}
		}
        //金字塔图像
        // Pyr Lucas kanade Optical Flow
		TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
        //计算金字塔光流点，HI_MPI_IVE_LKOpticalFlow这是海思对应的函数，计算前向和反向光流点
		vector<uchar> status;
        vector<float> err;

		calcOpticalFlowPyrLK(frame1_1C, frame2_1C, frame1_features, frame2_features, status,err,Size(7,7),3, termcrit, 0, 0.001);
		//calcOpticalFlowPyrLK(frame2_1C, frame1_1C, frame2_features, FB_features,     status,err,Size(7,7),3, termcrit, 0, 0.001);
		

        double xmean = 0;
		double ymean = 0;
        //fb_filter(&frame1_features[0], &FB_features[0], &frame2_features[0], &fb_pass[0], npoints);

        //归一化的相关性系数匹配方法，正值表示匹配的结果较好，负值则表示匹配的效果较差，也是值越大，匹配效果也好。
		//ncc_filter(frame1_1C,frame2_1C,&frame1_features[0],&frame2_features[0],npoints,CV_TM_CCOEFF_NORMED, rec0, rec1, res, &ncc_pass[0]);

        int pcount_prev = 1;
		pcount = 0;

        for(int i = 0; i<npoints;i++)
		{
			if( /*fb_pass[i] &&ncc_pass[i]  &&*/  (frame2_features[i].x>bbox_x) && (frame2_features[i].y>bbox_y) && (frame2_features[i].x < bbox_x + bbox_width ) && (frame2_features[i].y < bbox_y +bbox_height) )
			{
				//总的好点数
				pcount++;
			}
		}
#if 0
        if((median)<0.10 )
		{
            lost =true;
            cv::Rect track_box;
            bbox_x 		=0;
    		bbox_y 		=0;
    		bbox_width	=0;
    		bbox_height =0;
            return  Rect(0,0,0,0);
        }

#endif
        if(pcount == 0) 
		{  
            lost =true;
            cv::Rect track_box;
            bbox_x 		=0;
    		bbox_y 		=0;
    		bbox_width	=0;
    		bbox_height =0;
            return  Rect(0,0,0,0);
		} 
        vector<Point2f> curr_features2(pcount),prev_features2(pcount);
		int j = 0;
		
		for( int i = 0; i< npoints; i++)
		{
			if( /*fb_pass[i] && ncc_pass[i] &&*/  (frame2_features[i].x>bbox_x) && (frame2_features[i].y>bbox_y) && (frame2_features[i].x < bbox_x + bbox_width ) && (frame2_features[i].y < bbox_y +bbox_height) )
			{
				curr_features2[j] = frame2_features[i];
				prev_features2[j] = frame1_features[i];
				j++;
			}
		}
		
		int n2 = pcount*pcount;
        
        vector<double> pdist_prev(n2),pdist_curr(n2),pdiv(n2);
		//计算的是原始的点对之间的距离矩阵，相当于这些点之间的拓扑结构一样
		//第二个则是计算的第二个图距离矩阵，相当于第二帧图像上特征点的拓扑结构
		pairwise_dist(&prev_features2[0],&pdist_prev[0],pcount); // Find distance btw all points
		pairwise_dist(&curr_features2[0],&pdist_curr[0],pcount);

        //Divide corresponding distances to find the amount of scaling 
		//计算尺寸
		//（1）对于每一对点，都有一个比例，是当前点和前一阵点的距离比例。
		//（2）边界框的尺寸变化被定义为这些比例的中值。
		for (int i = 0; i < n2; i++) 
		{
			if (pdist_prev[i] > 0.0) 
			{
				pdiv[i] = pdist_curr[i]/pdist_prev[i];
			}
		}
		sort(pdiv.begin(),pdiv.end());
	
		double box_scale;
		box_scale = pdiv[n2/2]; // Scaling set to the median of all values
		
		/*
		* Bounding Box is moved using the points that were able to pass FB and NCC 
		*/	
		bbox_move(&prev_features2[0],&curr_features2[0],pcount,xmean,ymean);
		//跟踪后的矩形框的坐标
		bbox_x          = bbox_x + (xmean) - bbox_width*(box_scale - 1.)/2.;
		bbox_y          = bbox_y + (ymean) - bbox_height*(box_scale - 1.)/2.;
		bbox_width      = bbox_width * (box_scale);
		bbox_height     = bbox_height * (box_scale);

        cv::Rect track_box;
        track_box.x         =bbox_x <=0 ?0:bbox_x;
        track_box.y         =bbox_y <=0 ?0:bbox_y;
        track_box.width     =bbox_width;
        track_box.height    =bbox_height;

		if(  track_box.x+track_box.width >=frame1_1C.cols){
			track_box.width =frame1_1C.cols-2-track_box.x;
		}
		if(  track_box.y+track_box.height >=frame1_1C.rows){
			track_box.height =frame1_1C.rows-2-track_box.y;
		}
        //将当前帧赋值给第一帧图像
		//cvtColor(img,frame1_1C,CV_BGRA2GRAY);
		frame1_1C =frame2_1C;

		track_box.x 		=track_box.x*Scale;
		track_box.y 		=track_box.y*Scale;
		track_box.width 	=track_box.width*Scale;
		track_box.height 	=track_box.height*Scale;

		cout<<"track_box="<<track_box<<endl;

        return track_box;
		
}
