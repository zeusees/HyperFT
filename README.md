#  HyperFT

### Introduce
开源视频人脸跟踪算法,基于mtcnn人脸加测+onet人脸跟踪，移动端速度可以达到150fps+。该项目基于Android工程，提供底层JNI实现，使用者可以自行编译移植到其他平台。算法依赖ncnn深度学习计算库，体积小，易于集成。

#### 依赖库版本

Cmake：3.6  

OpenCV：3.4.7 

ncnn：20190611 bade132    

ndk：15c

#### Related Resources


+ [项目所需计算库ncnn](https://github.com/Tencent/ncnn/releases/download/20190611/ncnn-android-lib.zip)

+ [MTCNN的另类用法](https://blog.csdn.net/relocy/article/details/84075570)


### Features

+ 速度快，体积小，易于集成。
+ 支持人脸角度计算
+ 提供OpenGL图形绘制代码，支持后续显示优化及贴纸集成


### TODO

+ Sorry,目前还是急需要依赖opencv，稍后提交精简版，~~~~(>_<)~~~~

+ iOS Project Develop
