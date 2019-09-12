#ifndef ZEUSEESFACETRACKING_H
#define ZEUSEESFACETRACKING_H
#include <opencv2/opencv.hpp>
#include <thread>
#include "mtcnn.h"
#include "time.h"
#include <android/log.h>

namespace Shape{

    template <typename T> class Rect{
    public:
        Rect(){}
        Rect(T x,T y,T w,T h) {
            this->x = x;
            this->y = y;
            this->width = w;
            height = h;

        }
        T x;
        T y;
        T width;
        T height;

        cv::Rect convert_cv_rect(int _height,int _width)
        {
            cv::Rect Rect_(static_cast<int>(x*_width),static_cast<int>(y*_height),
                           static_cast<int>(width*_width),static_cast<int>(height*_height));
            return Rect_;
        }
    };
}


#define WITH_ATT 0
static int HeadPosePointIndexs[] = {94,59,27,20,69,45,50};
int *estimateHeadPosePointIndexs = HeadPosePointIndexs;


static float estimateHeadPose2dArray[] = {
        -0.208764,-0.140359,0.458815,0.106082,0.00859783,-0.0866249,-0.443304,-0.00551231,-0.0697294,
        -0.157724,-0.173532,0.16253,0.0935172,-0.0280447,0.016427,-0.162489,-0.0468956,-0.102772,
        0.126487,-0.164141,0.184245,0.101047,0.0104349,-0.0243688,-0.183127,0.0267416,0.117526,
        0.201744,-0.051405,0.498323,0.0341851,-0.0126043,0.0578142,-0.490372,0.0244975,0.0670094,
        0.0244522,-0.211899,-1.73645,0.0873952,0.00189387,0.0850161,1.72599,0.00521321,0.0315345,
        -0.122839,0.405878,0.28964,-0.23045,0.0212364,-0.0533548,-0.290354,0.0718529,-0.176586,
        0.136662,0.335455,0.142905,-0.191773,-0.00149495,0.00509046,-0.156346,-0.0759126,0.133053,
        -0.0393198,0.307292,0.185202,-0.446933,-0.0789959,0.29604,-0.190589,-0.407886,0.0269739,
        -0.00319206,0.141906,0.143748,-0.194121,-0.0809829,0.0443648,-0.157001,-0.0928255,0.0334674,
        -0.0155408,-0.145267,-0.146458,0.205672,-0.111508,0.0481617,0.142516,-0.0820573,0.0329081,
        -0.0520549,-0.329935,-0.231104,0.451872,-0.140248,0.294419,0.223746,-0.381816,0.0223632,
        0.176198,-0.00558382,0.0509544,0.0258391,0.050704,-1.10825,-0.0198969,1.1124,0.189531,
        -0.0352285,0.163014,0.0842186,-0.24742,0.199899,0.228204,-0.0721214,-0.0561584,-0.157876,
        -0.0308544,-0.131422,-0.0865534,0.205083,0.161144,0.197055,0.0733392,-0.0916629,-0.147355,
        0.527424,-0.0592165,0.0150818,0.0603236,0.640014,-0.0714241,-0.0199933,-0.261328,0.891053};


cv::Mat estimateHeadPoseMat = cv::Mat(15,9,CV_32FC1,estimateHeadPose2dArray);
static float estimateHeadPose2dArray2[] = {
        0.139791,27.4028,7.02636,
        -2.48207,9.59384,6.03758,
        1.27402,10.4795,6.20801,
        1.17406,29.1886,1.67768,
        0.306761,-103.832,5.66238,
        4.78663,17.8726,-15.3623,
        -5.20016,9.29488,-11.2495,
        -25.1704,10.8649,-29.4877,
        -5.62572,9.0871,-12.0982,
        -5.19707,-8.25251,13.3965,
        -23.6643,-13.1348,29.4322,
        67.239,0.666896,1.84304,
        -2.83223,4.56333,-15.885,
        -4.74948,-3.79454,12.7986,
        -16.1,1.47175,4.03941 };
cv::Mat estimateHeadPoseMat2 = cv::Mat(15,3,CV_32FC1,estimateHeadPose2dArray2);




void EstimateHeadPose(std::vector<cv::Point> &current_shape, cv::Vec3d &eav){
    if(current_shape.empty())
        return;
    static const int samplePdim = 7;
    float miny = 10000000000.0f;
    float maxy = 0.0f;
    float sumx = 0.0f;
    float sumy = 0.0f;
    for(int i=0; i<samplePdim; i++){
        sumx += current_shape[i].x;
        float y = current_shape[i].y;
        sumy += y;
        if(miny > y)
            miny = y;
        if(maxy < y)
            maxy = y;
    }
    float dist = maxy - miny;
    sumx = sumx/samplePdim;
    sumy = sumy/samplePdim;
    static cv::Mat tmp(1, 2*samplePdim+1, CV_32FC1);
    for(int i=0; i<samplePdim; i++){
        tmp.at<float>(i) = (current_shape[estimateHeadPosePointIndexs[i]].x - sumx)/dist;
        tmp.at<float>(i+samplePdim) = (current_shape[estimateHeadPosePointIndexs[i]].y-sumy)/dist;

    }
    tmp.at<float>(2*samplePdim) = 1.0f;
    cv::Mat predict = tmp*estimateHeadPoseMat2;
    eav[0] = predict.at<float>(0);
    eav[1] = predict.at<float>(1);
    eav[2] = predict.at<float>(2);
    return;
}

cv::Rect boundingRect(const std::vector<cv::Point> &pts){
    if(pts.size()>1)
    {
        int xmin = pts[0].x;
        int ymin = pts[0].y;
        int xmax = pts[0].x;
        int ymax = pts[0].y;
        for(int i = 1 ; i < pts.size(); i ++)
        {
            if(pts[i].x<xmin)
                xmin = pts[i].x;
            if(pts[i].y<ymin)
                ymin = pts[i].y;
            if(pts[i].x>xmax)
                xmax= pts[i].x;
            if(pts[i].y>ymax)
                ymax= pts[i].y;
        }
        return cv::Rect(xmin,ymin,xmax-xmin,ymax-ymin);
    }
}


//typedef int T;
//T i = 1;


class Face{
public:

    Face(int instance_id,Shape::Rect<float> rect){
        face_id = instance_id;
        landmark = std::shared_ptr<vector<cv::Point> >(new vector<cv::Point>(106));
//        landmark_prev = std::shared_ptr<vector<cv::Point> >(new vector<cv::Point>(106));
        face_location = rect;
        isCanShow = false; //追踪一次后待框稳定后即可显示
        for(int i = 0 ; i < 106;i++)
        {
            (*landmark)[i].x = -1;
            (*landmark)[i].y = -1;
        }
    }

    Face(){

        landmark = std::shared_ptr<vector<cv::Point> >(new vector<cv::Point>(106));
        isCanShow = false; //追踪一次后待框稳定后即可显示
    }

    std::shared_ptr<vector<cv::Point> > landmark;
    int landmark_prev[212];
    int face_id = -1;
    long frameId = 0 ;
    int ptr_num = 0;
    int ptr_eye_state  = 0;

    Shape::Rect<float> face_location;
    bool isCanShow;
    bool stateMonth;
    bool stateEye;
    bool stateRise;
    bool stateShake;
    double eulerAngle[3];
    bool state_left[20] = {false};
    bool state_right[20] = {false};
    cv::Mat frame_face_prev;

    static cv::Rect SquarePadding(cv::Rect facebox,int margin_rows,int margin_cols,bool max)
    {
        int c_x = facebox.x + facebox.width/2;
        int c_y = facebox.y + facebox.height/2;
        int large = 0 ;
        if(max)
            large  = std::max(facebox.height,facebox.width)/2;
        else
            large  = std::min(facebox.height,facebox.width)/2;
        cv::Rect rectNot(c_x-large,c_y-large,c_x+large,c_y+large);
        rectNot.x = std::max(0,rectNot.x);
        rectNot.y = std::max(0,rectNot.y);
        rectNot.height= std::min(rectNot.height,margin_rows-1);
        rectNot.width = std::min(rectNot.width,margin_cols-1);
        if(rectNot.height-rectNot.y!=rectNot.width-rectNot.x)
            return SquarePadding(cv::Rect(rectNot.x,rectNot.y,rectNot.width-rectNot.x,rectNot.height - rectNot.y),margin_rows,margin_cols,false);

        return cv::Rect(rectNot.x,rectNot.y,rectNot.width-rectNot.x,rectNot.height - rectNot.y);
    }

    static cv::Rect SquarePadding(cv::Rect facebox,int padding)
    {

        int c_x = facebox.x  -  padding;
        int c_y = facebox.y - padding;
        return cv::Rect(facebox.x-padding,facebox.y-padding,facebox.width+padding*2,facebox.height+ padding*2);;
    }

    static double getDistance(cv::Point x,cv::Point y)
    {
        return sqrt((x.x - y.x)*(x.x - y.x)+(x.y - y.y)*(x.y - y.y));
    }


    vector<vector<cv::Point> > faceSequence;
    vector<vector<float>> attitudeSequence;


};
class FaceTracking{
public:
    FaceTracking(string modelPath)
    {
        this->detector =new MTCNN (modelPath);
        downSimpilingFactor = 1;
        faceMinSize = 70;
        this->detector->SetMinFace(faceMinSize);
        detection_Time = -1;

    }

    ~FaceTracking(){
        delete this->detector;

    }

    void detecting(cv::Mat *image) {
        ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(image->data, ncnn::Mat::PIXEL_BGR2RGB, image->cols, image->rows);
        std::vector<Bbox> finalBbox;
        detector->detect(ncnn_img, finalBbox);
        const int num_box = finalBbox.size();
        std::vector<cv::Rect> bbox;
        bbox.resize(num_box);
        candidateFaces_lock = 1;
        for (int i = 0; i < num_box; i++) {
            bbox[i] = cv::Rect(finalBbox[i].x1, finalBbox[i].y1, finalBbox[i].x2 - finalBbox[i].x1 + 1,
                               finalBbox[i].y2 - finalBbox[i].y1 + 1);
            bbox[i] = Face::SquarePadding(bbox[i],image->rows,image->cols,true);
            Shape::Rect<float> f_rect(bbox[i].x/ static_cast<float>(image->cols),
                                      bbox[i].y/ static_cast<float>(image->rows),
                                      bbox[i].width/ static_cast<float>(image->cols),
                                      bbox[i].height/ static_cast<float>(image->rows)
            );
            std::shared_ptr<Face> face(new Face(trackingID,f_rect));
            (*image)(bbox[i]).copyTo(face->frame_face_prev );

            trackingID = trackingID+1;
            candidateFaces.push_back(*face);
        }
        candidateFaces_lock = 0 ;
    }

    void Init(cv::Mat &image){
        ImageHighDP = image;
        cv::Size lowDpSize(ImageHighDP.cols/downSimpilingFactor,ImageHighDP.rows/downSimpilingFactor);
        cv::resize(image,ImageLowDP,lowDpSize);
        trackingID = 0 ;
        detection_Interval = 350; //detect faces every 200 ms
        detecting(&image);
        stabilization = 0;
        UI_height = image.rows;
        UI_width= image.cols;
    }


    void doingLandmark_onet(cv::Mat &face,std::vector<cv::Point> &pts,int zeroadd_x ,int zeroadd_y,int stable_state =0 ) {
        ncnn::Mat in = ncnn::Mat::from_pixels_resize(face.data, ncnn::Mat::PIXEL_BGR, face.cols, face.rows, 48, 48);
        const float mean_vals[3] = {127.5f, 127.5f, 127.5f};
        const float norm_vals[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
        in.substract_mean_normalize(mean_vals, norm_vals);
        ncnn::Extractor Onet= detector->Onet.create_extractor();
        Onet.set_num_threads(2);
        Onet.input("data", in);
        ncnn::Mat out;
        Onet.extract("conv6-2", out);
        __android_log_print(ANDROID_LOG_ERROR,"landmark","conv6-2 output %f %f %f %f",out[0],out[1],out[2],out[3]);


        for (int j = 0; j < 2; j++) {
            int x = 0 ;
            int y=  0 ;
            if (j == 0) {
                x = static_cast<int>(out[j * 2 + 0] * face.cols) + zeroadd_x;
                y = static_cast<int>(out[j * 2 + 1] * face.rows) + zeroadd_y;
            }
            else{
                x = static_cast<int>(out[j * 2 + 0] * face.cols) +face.cols+ zeroadd_x;
                y = static_cast<int>(out[j * 2 + 1] * face.rows) +face.rows+ zeroadd_y;
            }
            __android_log_print(ANDROID_LOG_ERROR,"landmark","landmark %d %d",x,y);
            cv::Point p(x, y);
            pts[j] = p;
        }

        //for (int j = 0; j < 5; j++) {
        //    int x = 0 ;
        //    int y=  0 ;
        //     x = static_cast<int>(out[j  + 0] * face.cols) + zeroadd_x;
        //     y = static_cast<int>(out[j + 5] * face.rows) + zeroadd_y;
        //   // __android_log_print(ANDROID_LOG_ERROR,"landmark","landmark %d %d",x,y);
        //    cv::Point p(x, y);
        //    pts[j] = p;
        //}

    }
    void doingLandmark_112(cv::Mat &face,std::vector<cv::Point> &pts,int zeroadd_x ,int zeroadd_y,int stable_state =0 ) {
        //ncnn::Mat in = ncnn::Mat::from_pixels_resize(face.data, ncnn::Mat::PIXEL_BGR, face.cols, face.rows, 80, 80);
        ncnn::Mat in = ncnn::Mat::from_pixels_resize(face.data, ncnn::Mat::PIXEL_BGR, face.cols, face.rows, 112, 112);
        const float mean_vals[3] = {127.5f, 127.5f, 127.5f};
        const float norm_vals[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
        in.substract_mean_normalize(mean_vals, norm_vals);
        ncnn::Extractor frNet = net_landmark_.create_extractor();
        frNet.set_num_threads(2);
        // frNet.set_light_mode(true);
        frNet.input("data", in);
        ncnn::Mat out;
        frNet.extract("prelu1", out);
        __android_log_print(ANDROID_LOG_ERROR,"ncnn","h:%d w:%d c:%d",out.h,out.w,out.c);
        for (int j = 0; j < 106; j++) {
            int x = static_cast<int>(out[j * 2 + 0] * face.cols)+zeroadd_x;
            int y = static_cast<int>(out[j * 2 + 1] * face.rows)+zeroadd_y;
            cv::Point p(x, y);
            pts[j] = p;
        }

    }

    void tracking_corrfilter(const cv::Mat &frame, const cv::Mat &model, cv::Rect &trackBox,float scale )
    {
        trackBox.x/=scale;
        trackBox.y/=scale;
        trackBox.height/=scale;
        trackBox.width/=scale;
        int zeroadd_x = 0;
        int zeroadd_y = 0;
        cv::Mat frame_;
        cv::Mat model_;
        cv::resize(frame,frame_,cv::Size(),1/scale,1/scale);
        cv::resize(model,model_,cv::Size(),1/scale,1/scale);
        cv::Mat gray;
        cvtColor(frame_, gray, CV_RGB2GRAY);
        cv::Mat gray_model;
        cvtColor(model_, gray_model, CV_RGB2GRAY);
        cv::Rect searchWindow;
        searchWindow.width = trackBox.width * 3;
        searchWindow.height = trackBox.height * 3;
        searchWindow.x = trackBox.x + trackBox.width * 0.5 - searchWindow.width * 0.5;
        searchWindow.y = trackBox.y + trackBox.height * 0.5 - searchWindow.height * 0.5;
        searchWindow &= cv::Rect(0, 0, frame_.cols, frame_.rows);
        cv::Mat similarity;
        matchTemplate(gray(searchWindow), gray_model, similarity, CV_TM_CCOEFF_NORMED);
        double mag_r;
        cv::Point point;
        minMaxLoc(similarity, 0, &mag_r, 0, &point);
        trackBox.x = point.x + searchWindow.x;
        trackBox.y = point.y + searchWindow.y;
        trackBox.x*=scale;
        trackBox.y*=scale;
        trackBox.height*=scale;
        trackBox.width*=scale;
    }

    bool tracking( cv::Mat &image, Face &face)
    {
        cv::Rect faceROI = face.face_location.convert_cv_rect(image.rows,image.cols);
        cv::Mat faceROI_Image;
        tracking_corrfilter(image,face.frame_face_prev,faceROI,2);
        image(faceROI).copyTo(faceROI_Image);
        clock_t start_time = clock();
        (*face.landmark).resize(2);
        doingLandmark_onet(faceROI_Image,*face.landmark,faceROI.x,faceROI.y,face.frameId>1);
        //doingLandmark_112(faceROI_Image,*face.landmark,faceROI.x,faceROI.y,face.frameId>1);
        clock_t finish_time = clock();
        double total_time = (double) (finish_time - start_time) / CLOCKS_PER_SEC;
        cv::Rect bdbox((*face.landmark)[0].x,(*face.landmark)[0].y,(*face.landmark)[1].x-(*face.landmark)[0].x,(*face.landmark)[1].y-(*face.landmark)[0].y);
       // cv::Rect bdbox = cv::boundingRect((*face.landmark));
       // bdbox = Face::SquarePadding(bdbox, static_cast<int>(bdbox.height*0.55));
        bdbox = Face::SquarePadding(bdbox, static_cast<int>(bdbox.height*-0.05));
        bdbox = Face::SquarePadding(bdbox,image.rows,image.cols,1);
        Shape::Rect<float> boxfloat(bdbox.x/ static_cast<float>(image.cols),
                                    bdbox.y/ static_cast<float>(image.rows),
                                    bdbox.width/ static_cast<float>(image.cols),
                                    bdbox.height/ static_cast<float>(image.rows));

//        face.face_location.height = boxfloat.height;
//        face.face_location.width= boxfloat.width;
//        face.face_location.x= (faceROI.x*0.5 +bdbox.x*0.5)/static_cast<float>(image.cols);
//        face.face_location.y = (faceROI.y*0.5 + bdbox.y*0.5)/static_cast<float>(image.rows);

        face.face_location= boxfloat;
        faceROI = face.face_location.convert_cv_rect(image.rows,image.cols);
        image(faceROI).copyTo(face.frame_face_prev);
        face.frameId+=1;
        ncnn::Extractor Rnet= detector->Rnet.create_extractor();
        const float mean_vals[3] = {127.5, 127.5, 127.5};
        const float norm_vals[3] = {0.0078125, 0.0078125, 0.0078125};
        ncnn::Mat rnet_data= ncnn::Mat::from_pixels_resize(faceROI_Image.data, ncnn::Mat::PIXEL_BGR2RGB, faceROI_Image.cols, faceROI_Image.rows, 24, 24);
        rnet_data.substract_mean_normalize(mean_vals,norm_vals);
        Rnet.input("data", rnet_data);
        ncnn::Mat out_origin;
        Rnet.extract("prob1", out_origin);
        face.isCanShow = true;
        if(out_origin[1]>0.1){
            //stablize
            float diff_x = 0 ;
            float diff_y = 0 ;
            return true;
        }
        return false;

    }
    void setMask(cv::Mat &image,cv::Rect &rect_mask)
    {

        int height = image.rows;
        int width = image.cols;
        cv::Mat subImage  = image(rect_mask);
        subImage.setTo(0);
    }

    void update( cv::Mat &image)
    {
        ImageHighDP = image;
        std::cout<<trackingFace.size()<<std::endl;
        if(candidateFaces.size()>0 && !candidateFaces_lock)
        {
            for(int i = 0 ; i < candidateFaces.size() ; i++)
            {
                trackingFace.push_back(candidateFaces[i]);
            }
            candidateFaces.clear();
        }
        for(vector<Face>::iterator iter = trackingFace.begin();iter != trackingFace.end();)
        {
            if(!tracking(image,*iter))
            {
                iter = trackingFace.erase(iter); //追踪失败 则删除此人脸
            }
            else{
                iter++;
            }
        }

        if(detection_Time < 0 )
        {
            detection_Time = (double)cvGetTickCount();
        }
        else{
            double diff= (double)cvGetTickCount() - detection_Time ;
            diff /=(cvGetTickFrequency()*1000);
            if(diff>detection_Interval )
            {
                cv::Size lowDpSize(ImageHighDP.cols/downSimpilingFactor,ImageHighDP.rows/downSimpilingFactor);
                cv::resize(image,ImageLowDP,lowDpSize);
                //set Mask to protect the tracking face not to be detected.
                for( auto &face:trackingFace)
                {
                    Shape::Rect<float> rect = face.face_location;
                    cv::Rect rect1 = rect.convert_cv_rect(ImageLowDP.rows,ImageLowDP.cols);
                    setMask(ImageLowDP,rect1);
                }
                detection_Time = (double)cvGetTickCount();
                // do detection in thread
                detecting(&ImageLowDP);
            }

        }
    }



    vector<Face> trackingFace; //跟踪中的人脸
    int UI_width;
    int UI_height;


private:

//    float template_face[];
    // 姿态角估计
    int isLostDetection;
    int isTracking;
    int isDetection;
    cv::Mat ImageHighDP;
    cv::Mat ImageLowDP;
    int downSimpilingFactor;
    int faceMinSize;
    MTCNN *detector;
    vector<Face> candidateFaces; // 将检测到的人脸放入此列队 待跟踪的人脸
    bool candidateFaces_lock;
    double detection_Time;
    double detection_Interval;
    ncnn::Net net_landmark_;
    ncnn::Net net_eye_classifier;
    float stable_factor_stage1 = 0.2f;
    float stable_factor_stage2 = 2.0f;

    int trackingID;
    bool stabilization;


};
#endif //ZEUSEESFACETRACKING_H
