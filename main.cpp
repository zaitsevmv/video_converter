#include <iostream>
#include <vector>
#include <windows.h>

#ifdef _WIN32
    #include <windows.h>

    #include <opencv2/opencv.hpp>
#elif __unix__
    #include <opencv4/opencv2/core.hpp>
    #include <opencv4/opencv2/video.hpp>
    #include <opencv4/opencv2/videoio.hpp>
    #include <opencv4/opencv2/imgproc.hpp>
    #include <opencv4/opencv2/highgui.hpp>
#endif

const std::wstring PIXEL_BRIGHTNESS = L"@#%&?*+;:^~-_,. ";
const std::wstring PIXEL_COLORS_PREFIX = L"\x1b[38;2;";

std::vector<int> GetColorAmount(const int& beginRow, const int& stepRow,
                                 const int& beginCol, const int& stepCol, const cv::Mat& frame){
    auto sum = std::vector<int>{0,0,0};
    for(int r = beginRow; r < beginRow+stepRow; r++){
        for(int c = beginCol; c < beginCol+stepCol; c++){
            auto pixel = frame.at<cv::Vec3b>(r, c);
            sum[0] += pixel[0]; //blue
            sum[1] += pixel[1]; //green
            sum[2] += pixel[2]; //red
        }
    }
    return sum;
}

std::wstring ConvertPixels(cv::Mat& frame, const int cols, const int lines){
    std::wstring ResultedFrame;
    std::wstring colorPrefix, prevColorPrefix;
    int frameCols = frame.cols, frameLines = frame.rows, totalUnity = frameLines/lines*frameCols/cols;
    for(int l = 0; l <= frameLines-frameLines/lines; l += frameLines/lines){
        for(int c = 0; c <= frameCols-frameCols/cols; c += frameCols/cols){
            auto colors = GetColorAmount(l, frameLines/lines,
                                          c, frameCols/cols, frame);
            int colorSum = colors[0] + colors[1] + colors[2];
            colorPrefix = PIXEL_COLORS_PREFIX + std::to_wstring(colors[2]/(totalUnity*64) *64)
                    + L";" + std::to_wstring(colors[1]/(totalUnity*64) *64)
                    + L";" + std::to_wstring(colors[0]/(totalUnity*64) *64) + L"m";
            if(colorPrefix == prevColorPrefix){
                colorPrefix = L"";
            } else{
                prevColorPrefix = colorPrefix;
            }
            ResultedFrame += colorPrefix + PIXEL_BRIGHTNESS
                    [PIXEL_BRIGHTNESS.size()-1-(colorSum*PIXEL_BRIGHTNESS.size())/(totalUnity*256*3)];
        }
        ResultedFrame += L"\n";
    }
    return ResultedFrame;
}

int main(int argc, char* argv[]){
    if(argc != 4){
        std::cerr << "Error getting parameters" << std::endl;
        return -1;
    }

    cv::VideoCapture video(argv[3]);
    if (!video.isOpened()) {
        std::cerr << "Error opening video file." << std::endl;
        return -1;
    }

    std::ios::sync_with_stdio(false);
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        return GetLastError();
    }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        return GetLastError();
    }

    const int VIDEO_FPS = video.get(cv::CAP_PROP_FPS);

    const int COLS = atoi(argv[1]), LINES = atoi(argv[2]);

    std::vector<std::wstring> result;
    cv::Mat frame;
    while(video.read(frame)){
        result.push_back(ConvertPixels(frame, COLS, LINES));
    }
    video.release();
    cv::destroyAllWindows();
    for(auto& asciiFrames: result){
        std::wcout << asciiFrames;

        if (cv::waitKey(1000/VIDEO_FPS) == 'q') {
            break;
        }
    }
    system("pause");
    return 0;
}