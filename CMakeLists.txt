cmake_minimum_required(VERSION 3.20)
project(video_converter)

set(CMAKE_CXX_STANDARD 20)

include(ExternalProject)

ExternalProject_Add(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG        master
        PREFIX         ${CMAKE_BINARY_DIR}/opencv
        INSTALL_DIR    ${CMAKE_BINARY_DIR}/opencv/install
        CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

set(OpenCV_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/opencv/install/include)
set(OpenCV_LIB_DIR ${CMAKE_BINARY_DIR}/opencv/install/x64/mingw/bin)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(test main.cpp)
add_dependencies(test opencv)

if (WIN32)
    target_link_libraries(test
            PRIVATE
            ${OpenCV_LIB_DIR}/libopencv_core490.dll
            ${OpenCV_LIB_DIR}/libopencv_video490.dll
            ${OpenCV_LIB_DIR}/libopencv_videoio490.dll
            ${OpenCV_LIB_DIR}/libopencv_imgproc490.dll
            ${OpenCV_LIB_DIR}/libopencv_highgui490.dll
    )
endif (WIN32)

if (UNIX)
    message(STATUS "OpenCV library status:")
    message(STATUS "    config: ${OpenCV_DIR}")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    libraries: ${OpenCV_LIBS}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

    target_link_libraries(test PRIVATE ${OpenCV_LIBS})
endif (UNIX)