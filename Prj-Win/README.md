# Ncnn_FaceTrack

基于mtcnn人脸检测+onet人脸跟踪

# 开发环境

win7
vs2015


# 开源框架

+ [ncnn](https://github.com/Tencent/ncnn)

+ [opencv](https://github.com/opencv/opencv)

# 引用

[HyperFT](https://github.com/zeusees/HyperFT)

这是一个移动端快速视频多人脸跟踪的开源项目，这个项目是基于mtcnn人脸检测加上最简单的模板匹配进行人脸跟踪的，算法简单但效果显著，移动端速度可以达到150帧以上，该项目的特点是可实现多人脸跟踪。

# 代码算法解析

HyperFT项目的多人脸跟踪算法分三大部分：

第一部分是初始化，通过mtcnn的人脸检测找出第一帧的人脸位置然后将其结果对人脸跟踪进行初始化；

第二部分是更新，利用模板匹配进行人脸目标位置的初步预判，再结合mtcnn中的onet来对人脸位置进行更加精细的定位，最后通过mtcnn中的rnet的置信度来判断跟踪是否为人脸，防止当有手从面前慢慢挥过去的话，框会跟着手走而无法跟踪到真正的人脸；

第三部分是定时检测，通过在更新的部分中加入一个定时器来做定时人脸检测，从而判断中途是否有新人脸的加入，本项目在定时人脸检测中使用了一个trick就是将已跟踪的人脸所在位置利用蒙版遮蔽起来，避免了人脸检测的重复检测，减少其计算量，从而提高了检测速度。

# 算法改进的思路（加入五个关键点的跟踪）

1、在HyperFT项目中的Face类中仅定义了人脸矩形的变量，如若需要加入五个关键点的跟踪则需要在Face类中需要定义一个Bbox类faceBbox，这样Face即能保存人脸位置又能保存人脸关键点。

2、在原来的doingLandmark_onet函数的基础上重载函数，将传入的std::vector<cv::Point> &pts改为传入Bbox& faceBbox。

3、在tracking函数中修改doingLandmark_onet函数的调用

4、通过人脸跟踪中Face类中的faceBbox即可获得人脸的位置及其五个人脸关键点（main.cpp）
