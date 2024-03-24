cmake_minimum_required(VERSION 3.22)
project(video_converter)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG 4.8.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(opencv)

add_executable(test main.cpp)
target_include_directories(test PRIVATE
        ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
        ${OPENCV_MODULE_opencv_core_LOCATION}/include
        ${OPENCV_MODULE_opencv_video_LOCATION}/include
        ${OPENCV_MODULE_opencv_videoio_LOCATION}/include
        ${OPENCV_MODULE_opencv_imgproc_LOCATION}/include
        ${OPENCV_MODULE_opencv_imgcodecs_LOCATION}/include
        ${OPENCV_MODULE_opencv_highgui_LOCATION}/include
)
target_link_libraries(test
        opencv_core
        opencv_video
        opencv_videoio
        opencv_imgproc
        opencv_imgcodecs
        opencv_highgui
)