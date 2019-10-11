# HyperFT linux-ncnn版本
基于mtcnn人脸检测+onet人脸跟踪（光流跟踪）

#开发环境
ncnn ubuntu18.04 opencv4.01


#开源框架
+ [ncnn](https://github.com/Tencent/ncnn)

+ [opencv](https://github.com/opencv/opencv)

# 引用
[HyperFT](https://github.com/zeusees/HyperFT)
https://github.com/qaz734913414/Ncnn_FaceTrack

编译方步骤：
1、修改CMakeList.txt中的ncnn路径，修改成你自己的路径；
2、mkdir build
3、cd build
4、cmake ..
5、make -j4

注意：运行的时候出现错误了，可能是模型路径不对。


Todo：
1、将MTCNN检测换成MSSD相关的检测模型，这样速度不会随着人脸数目增加而增加
2、使用更快的O网络
3、人头姿态部分，目前使用的版本速度不是很快，正在优化中

