#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define DEVICE "/dev/video0" // 默认相机设备

// 输出错误信息并退出
void perror_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// 打开视频设备
int open_device(const char *device) {
    int fd = open(device, O_RDWR);
    if (fd == -1) {
        perror_exit("Error opening video device");
    }
    return fd;
}

// 设置视频格式与分辨率
void set_video_format(int fd, uint32_t width, uint32_t height) {
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;  // 设置为 UYVY 422 格式
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror_exit("Error setting video format");
    }

    std::cout << "Format set to " << width << "x" << height << " UYVY 422\n";
}

// 请求缓冲区
void request_buffers(int fd, struct v4l2_buffer &buffer, uint32_t &buffer_count) {
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 1;  // 请求一个缓冲区
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror_exit("Error requesting buffers");
    }

    // 查询缓冲区信息
    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) == -1) {
        perror_exit("Error querying buffer");
    }

    buffer_count = buffer.length;
}

// 映射缓冲区
void *map_buffer(int fd, const struct v4l2_buffer &buffer) {
    void *buffer_start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);
    if (buffer_start == MAP_FAILED) {
        perror_exit("Error mapping buffer");
    }
    return buffer_start;
}

// 读取图像并以16进制输出（指定行范围并输出字节编号）
void capture_and_output(int fd, struct v4l2_buffer &buffer, void *buffer_start, uint32_t start_row, uint32_t end_row, uint32_t width) {
    if (ioctl(fd, VIDIOC_QBUF, &buffer) == -1) {
        perror_exit("Error queueing buffer");
    }

    if (ioctl(fd, VIDIOC_STREAMON, &buffer.type) == -1) {
        perror_exit("Error starting stream");
    }

    if (ioctl(fd, VIDIOC_DQBUF, &buffer) == -1) {
        perror_exit("Error dequeuing buffer");
    }

    // 输出图像数据为16进制，按行输出，附加字节编号
    uint8_t *data = static_cast<uint8_t *>(buffer_start);
    uint32_t byte_position = 1;  // 字节位置从1开始

    for (uint32_t row = start_row; row <= end_row; row++) {
        uint32_t row_start_idx = row * width * 2;  // 每行的起始字节位置，UYVY每个像素2字节
        uint32_t row_end_idx = row_start_idx + width * 2;

        for (uint32_t i = row_start_idx; i < row_end_idx; i++) {
            if (i % 16 == 0 && i != row_start_idx) std::cout << std::endl;
            printf("[%04d] %02X ", byte_position++, data[i]);
        }
        std::cout << std::endl;
    }

    if (ioctl(fd, VIDIOC_STREAMOFF, &buffer.type) == -1) {
        perror_exit("Error stopping stream");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <device> <width> <height> <start_row> <end_row>" << std::endl;
        return EXIT_FAILURE;
    }

    const char *device = argv[1];
    uint32_t width = std::stoi(argv[2]);
    uint32_t height = std::stoi(argv[3]);
    uint32_t start_row = std::stoi(argv[4]);
    uint32_t end_row = std::stoi(argv[5]);

    // 打开设备
    int fd = open_device(device);

    // 设置视频格式
    set_video_format(fd, width, height);

    // 请求缓冲区
    struct v4l2_buffer buffer;
    uint32_t buffer_count;
    request_buffers(fd, buffer, buffer_count);

    // 映射缓冲区
    void *buffer_start = map_buffer(fd, buffer);

    // 读取数据并以16进制输出，指定行范围和字节编号
    capture_and_output(fd, buffer, buffer_start, start_row, end_row, width);

    // 释放资源
    munmap(buffer_start, buffer_count);
    close(fd);

    return EXIT_SUCCESS;
}

