#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <string>

int main(int argc, char* argv[]) {
    // 检查基本命令行参数
    if (argc < 4) {
        std::cerr << "用法: " << argv[0] << " <设备名> <宽度> <高度> [起始行号] [结束行号]" << std::endl;
        return -1;
    }

    // 读取命令行参数
    std::string device = argv[1];
    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);

    // 可选行号区间参数
    int start_row = 0;              // 默认起始行为0
    int end_row = height - 1;       // 默认结束行为图像最大行号
    if (argc >= 5) start_row = std::stoi(argv[4]);
    if (argc >= 6) end_row = std::stoi(argv[5]);

    // 检查行号区间是否有效
    if (start_row < 0 || end_row >= height || start_row > end_row) {
        std::cerr << "无效的行号区间: [" << start_row << ", " << end_row << "]" << std::endl;
        return -1;
    }

    // 打开摄像头
    cv::VideoCapture cap(device);
    if (!cap.isOpened()) {
        std::cerr << "无法打开摄像头: " << device << std::endl;
        return -1;
    }

    // 设置分辨率
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);

    // 读取一帧图像
    cv::Mat frame;
    if (!cap.read(frame)) {
        std::cerr << "无法读取帧" << std::endl;
        return -1;
    }
    std::cout << "Channels: " << frame.channels() 
	      << ",Type: " << frame.type()
              << ",Depth: " << frame.depth()
	      << std::endl;

    // 检查实际分辨率
    if (frame.cols != width || frame.rows != height) {
        std::cerr << "图像分辨率与期望值不匹配。当前分辨率为: "
                  << frame.cols << "x" << frame.rows << std::endl;
        return -1;
    }

    // 提取指定行号区间的图像部分
    cv::Mat cropped_frame = frame(cv::Range(start_row, end_row + 1), cv::Range::all());

    // 输出指定行号区间的每个像素的坐标和 BGR 值
    /*for (int y = start_row; y <= end_row; y++) {
        std::cout << "行 " << y << ": ";
        for (int x = 0; x < frame.cols; x++) {
            cv::Vec3b pixel = frame.at<cv::Vec3b>(y, x);
            int blue = pixel[0];
            int green = pixel[1];
            int red = pixel[2];

            std::cout << "(列 " << x << ": B=" << std::setw(3) << blue
                      << ", G=" << std::setw(3) << green
                      << ", R=" << std::setw(3) << red << ") ";
        }
        std::cout << std::endl;
    }*/
    // 确保图像数据为单字节深度（例如，8位格式）
    CV_Assert(frame.depth() == CV_8U);

    // 获取图像行数和列数
    int rows = frame.rows;
    int cols = frame.cols;
    int channels = frame.channels();

    std::cout << "Frame size: " << rows << "x" << cols << ", Channels: " << channels << std::endl;

    /*for (int i = start_row; i < end_row; ++i) {
        const uchar* row_ptr = frame.ptr<uchar>(i); // 获取行指针
        for (int j = 0; j < cols * channels; ++j) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(row_ptr[j]) << " ";
        }
        std::cout << std::endl; // 每行结束后换行
    }*/    
for (int i = start_row; i < end_row; ++i) {
    const uchar* row_ptr = frame.ptr<uchar>(i); // 获取行指针
    std::cout << "Row " << i << ":" << std::endl; // 输出当前行索引
    for (int j = 0; j < cols * channels; ++j) {
        // 输出数组下标和对应值，使用制表符进行对齐
        std::cout << "[" << std::setw(4) << (i * cols * channels + j) + 1 << "] "
                  << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(row_ptr[j]) << "\t";
        
	std::cout << std::dec;
        // 每8个元素换行以便于查看
        if ((j + 1) % 8 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl; // 每行结束后换行
}

    // 显示指定行区间的图像
    cv::imshow("Cropped Camera Frame", cropped_frame);
    cv::waitKey(0);  // 等待按键关闭窗口

    // 释放摄像头
    cap.release();
    return 0;
}

