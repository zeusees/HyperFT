#  HyperFT

### Introduce
开源视频人脸跟踪算法,基于mtcnn人脸加测+onet人脸跟踪，移动端速度可以达到150fps+。该项目基于Android工程，提供底层JNI实现，使用者可以自行编译移植到其他平台。算法依赖ncnn深度学习计算库，体积小，易于集成。

### 代码编译

#### 环境准备

Android Studio v3.5

CMake:3.6.4

Android SDK Platform-Tools：29.0.3

Android SDK Tools：26.1.1

NDK：r15c

#### 依赖库

OpenCV：3.4.7 

[OpenCV](https://sourceforge.net/projects/opencvlibrary/files/4.1.1/opencv-4.1.1-android-sdk.zip/download)

ncnn：20190611 bade132

[ncnn](https://github.com/Tencent/ncnn/releases/download/20190611/ncnn-android-lib.zip)

#### 编译设置

1.设置Android NDK与Android SDK地址

2.修改CMake编译文件，在app/src/main/cpp/下修改CMakeLists.txt文件:

```
include_directories(D:/Wendell/Develop/libs/ncnn-android-lib/include)
include_directories(D:/Wendell/Develop/libs/OpenCV-android-sdk/sdk/native/jni/include)
set(OpenCV_DIR "D:/Wendell/Develop/libs/OpenCV-android-sdk/sdk/native/jni")
set_target_properties(libncnn PROPERTIES IMPORTED_LOCATION D:/Wendell/Develop/libs/ncnn-android-lib/${ANDROID_ABI}/libncnn.a)
```

### Related Resources


+ [项目所需计算库ncnn](https://github.com/Tencent/ncnn/releases/download/20190611/ncnn-android-lib.zip)

+ [MTCNN的另类用法](https://blog.csdn.net/relocy/article/details/84075570)

+ [Win版HyperFT](https://github.com/qaz734913414/Ncnn_FaceTrack)

### Features

+ 速度快，体积小，易于集成。
+ 支持人脸角度计算
+ 提供OpenGL图形绘制代码，支持后续显示优化及贴纸集成

### 体验

+ [体验apk](https://fir.im/HyperFT)

### TODO

+ Sorry,目前还是急需要依赖opencv，稍后提交精简版，~~~~(>_<)~~~~

+ iOS Project Develop
