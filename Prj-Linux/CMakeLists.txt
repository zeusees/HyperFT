cmake_minimum_required(VERSION 2.8)

project(tracker)
  
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wextra -pedantic -Wno-long-long")
endif()

#寻找OpenCV库
#修改成捏OpenCV路径即可
set(OpenCV_DIR "/home/xz/opencv3.4.3/opencv-3.4.3/build")
set(OpenCV_INCLUDE_DIRS "/home/xz/opencv3.4.3/opencv-3.4.3/include") 
find_package(OpenCV REQUIRED)

if (OPENMP_FOUND)
    message("OpenMP found!")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

include_directories(/home/xz/ncnn/build/install/include/ncnn)
link_directories(/home/xz/ncnn/build/install/lib)



add_executable(MultiFaceTracking main.cpp mtcnn.cpp colotracker.cpp)
target_link_libraries(MultiFaceTracking ncnn ${OpenCV_LIBS}  -fopenmp  /usr/lib/x86_64-linux-gnu/libsqlite3.so)





