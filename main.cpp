#include <iostream>
#include <numeric>
#include <vector>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>

    #include <opencv2/core.hpp>
    #include <opencv2/video.hpp>
    #include <opencv2/videoio.hpp>
    #include <opencv2/imgproc.hpp>
    #include <opencv2/highgui.hpp>

    constexpr std::wstring PIXEL_COLORS_PREFIX = L"\x1b[38;2;";
    constexpr std::wstring PIXEL_COLORS_DEFAULT = L"\x1b[39m";

    error_status_t FixTerminal(){
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
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
        return 0;
    }

    std::pair<int,int> GetTerminalSize(){
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        return {csbi.srWindow.Bottom - csbi.srWindow.Top,
            csbi.srWindow.Right - csbi.srWindow.Left};
    }

    void ClearTerminal() {
        system("cls");
    }
#elif __unix__
    #include <sys/ioctl.h>
    #include <unistd.h>

    #include <opencv2/core.hpp>
    #include <opencv2/video.hpp>
    #include <opencv2/videoio.hpp>
    #include <opencv2/imgproc.hpp>
    #include <opencv2/highgui.hpp>

    constexpr std::wstring PIXEL_COLORS_PREFIX = L"\033[38;2;";
    constexpr std::wstring PIXEL_COLORS_DEFAULT = L"\033[39m";

    error_t FixTerminal(){
        return 0;
    }

    std::pair<int,int> GetTerminalSize(){
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return {w.ws_row, w.ws_col};
    }

    void ClearTerminal() {
        system("clear");
    }
#endif

const std::wstring PIXEL_BRIGHTNESS = L"@#%&?*+;:^~-_,. ";

std::wstring ConvertPixels(const cv::Mat& frame, const int cols, const int rows){
    std::wstring ResultedFrame;
    std::wstring prevColorPrefix;
    const int frameCols = frame.cols, frameRows = frame.rows;
    const double devisor = 1.0/(frameRows/rows*frameCols/cols * 32);
    for(int r = 0; r < frameRows; r += frameRows/rows){
        for(int c = 0; c < frameCols; c += frameCols/cols){
            auto scColors = sum(frame(cv::Rect(c, r,
                std::min(frameCols/cols, frameCols-c), std::min(frameRows/rows, frameRows-r))));
            std::vector colors{scColors[2], scColors[1], scColors[0]};
            int colorSum = std::accumulate(colors.begin(), colors.end(), 0);

            std::wstring colorPrefix = PIXEL_COLORS_PREFIX + std::to_wstring(static_cast<int>(colors[0] * devisor) * 32)
                                       + L";" + std::to_wstring(static_cast<int>(colors[1] * devisor) * 32)
                                       + L";" + std::to_wstring(static_cast<int>(colors[2] * devisor) * 32) + L"m";
            if(colorPrefix == prevColorPrefix){
                colorPrefix = L"";
            } else{
                prevColorPrefix = colorPrefix;
            }
            int symbolId = PIXEL_BRIGHTNESS.size() - 1 -(colorSum*PIXEL_BRIGHTNESS.size() * devisor * 32) /(256*3);
            ResultedFrame.append(colorPrefix + PIXEL_BRIGHTNESS[symbolId]);
        }
        ResultedFrame += L"\n";
    }
    return ResultedFrame;
}

int main(int argc, char* argv[]){
    const std::string keys =
        "{@path | <none> | path to file}"
        "{fps   |        | video fps}"
        "{help h|        | help}";
    cv::CommandLineParser parser{argc, argv, keys};
    parser.about("Simple video to ASCII converter");
    if(parser.has("help")) {
        parser.printMessage();
        return 0;
    }
    cv::VideoCapture video;
    int latency = 0;
    try {
        video = cv::VideoCapture(parser.get<std::string>("@path"), cv::CAP_ANY);
        if(parser.has("fps")) {
            latency = 1000/parser.get<int>("fps");
        } else {
            latency = 1000/video.get(cv::CAP_PROP_FPS);
        }
    } catch (...) {
        return cv::Error::StsBadArg;
    }

    ClearTerminal();
    try {
        FixTerminal();
        std::ios::sync_with_stdio(false);
        std::cin.tie(NULL);
        std::cout.tie(NULL);
        std::wcout.tie(NULL);
    } catch (...) {
        return -1;
    }

    cv::Mat frame;
    const double res = video.get(cv::CAP_PROP_FRAME_WIDTH)/video.get(cv::CAP_PROP_FRAME_HEIGHT);
    const double ser = video.get(cv::CAP_PROP_FRAME_HEIGHT)/(video.get(cv::CAP_PROP_FRAME_WIDTH)*2);

    auto startTime = std::chrono::steady_clock::now();
    while(video.read(frame)){
        auto terminalSize = GetTerminalSize();
        int rows = terminalSize.first, cols = terminalSize.second;
        if(cols >= 2*rows * res){
            cols = 2*rows * res;
        }else{
            rows = cols * ser;
        }
        std::wcout << ConvertPixels(frame, cols, rows) << std::flush;
        const std::chrono::duration<double> duration = std::chrono::steady_clock::now() - startTime;
        cv::waitKey(std::max(1,
            latency - static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count())));
        startTime = std::chrono::steady_clock::now();
    }
    ClearTerminal();
    video.release();
    return 0;
}