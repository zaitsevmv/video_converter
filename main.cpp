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
            colorPrefix = PIXEL_COLORS_PREFIX + std::to_wstring(colors[2]/(totalUnity*32) *32)
                          + L";" + std::to_wstring(colors[1]/(totalUnity*32) *32)
                          + L";" + std::to_wstring(colors[0]/(totalUnity*32) *32) + L"m";
            if(colorPrefix == prevColorPrefix){
                colorPrefix = L"";
            } else{
                prevColorPrefix = colorPrefix;
            }
            ResultedFrame.append(colorPrefix + PIXEL_BRIGHTNESS
            [PIXEL_BRIGHTNESS.size()-1-(colorSum*PIXEL_BRIGHTNESS.size())/(totalUnity*256*3)]);
        }
        ResultedFrame += L"\n";
    }
    return ResultedFrame;
}

int main(int argc, char* argv[]){
    if(argc != 2){
        std::cerr << "Error getting parameters" << std::endl;
        return -1;
    }

    cv::VideoCapture video(argv[1]);
    if (!video.isOpened()) {
        std::cerr << "Error opening video file." << std::endl;
        return -1;
    }


    system("cls");
    cv::waitKey(3000) == 13;
    std::cout << "Loading...";


    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    const int LINES = 9*(csbi.srWindow.Bottom - csbi.srWindow.Top)/10;
    const int COLS = 9*(csbi.srWindow.Right - csbi.srWindow.Left)/10;

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
    std::ios::sync_with_stdio(false);
    std::cin.tie(NULL);
    std::cout.tie(NULL);
    std::wcout.tie(NULL);

    const int VIDEO_FPS = video.get(cv::CAP_PROP_FPS);

    std::vector<std::wstring> result;
    cv::Mat frame;
    while(video.read(frame)){
        result.push_back(ConvertPixels(frame, COLS, LINES));
    }
    system("cls");
    video.release();
    for(auto& asciiFrames: result){
        std::wcout << asciiFrames << std::flush;

        if (cv::waitKey(1000/VIDEO_FPS) == 'q') {
            std::wcout << L"\x1b[39m";
            return 0;
        }
    }
    std::wcout << L"\x1b[39m";
    return 0;
}