#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"

const std::string PIXEL_BRIGHTNESS = "@#%&?*+;:^~-_,. ";
//const std::vector<std::string> PIXEL_COLORS_GRAY = {};

std::vector<int> GetColorAmount(const int& beginRow, const int& stepRow,
                                 const int& beginCol, const int& stepCol, const cv::Mat& frame){
    auto sum = std::vector<int>{0,0,0};
    for(int r = beginRow; r < beginRow+stepRow; r++){
        for(int c = beginCol; c < beginCol+stepCol; c++){
            auto pixel = frame.at<cv::Vec3b>(r, c);
            sum[0] += pixel[0];
            sum[1] += pixel[1];
            sum[2] += pixel[2];
        }
    }
    return sum;
}

std::string ConvertPixels(cv::Mat& frame, const int cols, const int lines){
    std::string ResultedFrame = "\n";
    int frameCols = frame.cols, frameLines = frame.rows;
    for(int l = 0; l <= frameLines-frameLines/lines; l += frameLines/lines){
        for(int c = 0; c <= frameCols-frameCols/cols; c += frameCols/cols){
            auto colors = GetColorAmount(l, frameLines/lines,
                                          c, frameCols/cols, frame);
            int colorSum = colors[0] + colors[1] + colors[2];
            ResultedFrame.push_back(PIXEL_BRIGHTNESS[colorSum/((frameLines/lines+frameCols/cols)*48)]);
        }
        ResultedFrame += "\n";
    }
    return ResultedFrame;
}

int main(int argc, char* argv[]){
    std::cout << argc;
    if(argc != 4){
        std::cerr << "Error getting parameters" << std::endl;
        return -1;
    }

    cv::VideoCapture video("C:/Users/Matvey/Downloads/clip.mp4");
    const cv::VideoCapture VIDEO(argv[3]);
    if (!video.isOpened()) {
        std::cerr << "Error opening video file." << std::endl;
        return -1;
    }
    const int VIDEO_FPS = video.get(cv::CAP_PROP_FPS);

    const int COLS = atoi(argv[1]), LINES = atoi(argv[2]);
    std::cout << COLS << LINES << argv[0] << argv[1];

    std::vector<std::string> result;
    cv::Mat frame;
    while(video.read(frame)){
        result.push_back(ConvertPixels(frame, COLS, LINES));
    }
    video.release();
    cv::destroyAllWindows();

    /*for(auto& asciiFrames: result){
        std::cout << asciiFrames;

        if (cv::waitKey(1000/VIDEO_FPS) == 'q') {
            break;
        }
    }*/
    std::cout << result[103];
    return 0;
}