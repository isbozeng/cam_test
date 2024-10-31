#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
    // 检查基本命令行参数
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <视频设备或文件路径> [分辨率宽度] [分辨率高度]" << std::endl;
        return -1;
    }

    // 获取设备名或视频文件路径
    std::string source = argv[1];
    bool isCamera = (source.find("/dev/video") != std::string::npos);

    // 打开视频流（摄像头设备或视频文件）
    cv::VideoCapture cap(source);
    if (!cap.isOpened()) {
        std::cerr << "无法打开视频源: " << source << std::endl;
        return -1;
    }

    // 如果是摄像头，则设置分辨率（可选参数）
    if (isCamera && argc >= 4) {
        int width = std::stoi(argv[2]);
        int height = std::stoi(argv[3]);
        cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    }

    // 获取视频流的帧率
    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps == 0) fps = 30.0;  // 设置一个默认帧率（摄像头可能返回0）

    // 计时器和帧计数器
    auto startTime = std::chrono::steady_clock::now();
    int frameCount = 0;

    cv::Mat frame;
    while (true) {
        // 读取一帧图像
        if (!cap.read(frame) || frame.empty()) break;

        // 显示图像
        cv::imshow("Video Frame", frame);
        frameCount++;

        // 计算并显示帧率信息
        auto currentTime = std::chrono::steady_clock::now();
        double elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsedTime > 0) {
            double currentFPS = frameCount / elapsedTime;
            std::cout << "\r当前帧率: " << currentFPS << " FPS" << std::flush;
        }

        // 按 'q' 键退出
        if (cv::waitKey(1) == 'q') break;
    }

    // 释放资源
    cap.release();
    cv::destroyAllWindows();
    return 0;
}

