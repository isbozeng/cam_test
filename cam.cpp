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
#include "ebd_data_type.h"
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


void print_lep_ebd(const lep_ebd_t &ebd) {
    printf("------------ leopard imaging_ebd Information ------------\n");
    
    // 打印每个结构体的字段
    printf("Size: %u\n", ebd.size.size);

    // 打印 prv_inf_e13_t
    printf("Chip ID : %u\n", ebd.indiv_inf.chip_id_e13);

    // 打印 framer_count_e37_t
    printf("Framer Count: %u\n", ebd.frame_count.framer_count);

    // 打印 framer_id_e41_t
    printf("Framer ID: %u\n", ebd.frame_id.framer_id);

    // 打印 lep_temp_e53_t
    printf("Temperature 1: %f, Temperature 2: %f\n", (ebd.temper.temp1&0xfff)/16.0 - 50, (ebd.temper.temp2&0xfff)/16.0 - 50);

    // 打印 lep_err_e61_t
    printf("Error Code E61: %.8x, Error Code E66: %.2x\n", ebd.err_code.code_e61, ebd.err_code.code_e66);

    // 打印 vol_e89_t
    printf("Voltage High: %fv, Voltage Mid: %fv, Voltage Low: %fv\n", ebd.vol.vh * 1e-3, ebd.vol.vm * 1e-3, ebd.vol.vl * 1e-3);

    // 打印 image_size_e189_t
    printf("Image Width: %u, Image Height: %u\n", ebd.img_size.horizontal, ebd.img_size.vertical);

    // 打印 drive_mode_e197_t
    printf("Drive Mode: %u\n", ebd.drive_mode.mode);

    // 打印 sync_mode_e201_t
    printf("Sync Method: %u, Operation Mode: %u\n", ebd.sync_mode.method, ebd.sync_mode.op_mode);

}

void to_ebd_data(uint8_t *p_data, lep_ebd_t *p_ebd) {
    memcpy(&p_ebd->size, p_data + EBD_SIZE - 1, sizeof(p_ebd->size));
    memcpy(&p_ebd->indiv_inf, p_data + INDIVIDUAL_INF - 1, sizeof(p_ebd->indiv_inf));
    memcpy(&p_ebd->frame_count, p_data + FRAME_COUNT - 1, sizeof(p_ebd->frame_count));
    memcpy(&p_ebd->frame_id, p_data + FRAME_ID - 1, sizeof(p_ebd->frame_id));
    memcpy(&p_ebd->temper, p_data + TEMPERATURE_INF - 1, sizeof(p_ebd->temper));   
    memcpy(&p_ebd->err_code, p_data + ERR_CODE - 1, sizeof(p_ebd->err_code));  
    memcpy(&p_ebd->vol, p_data + VOL - 1, sizeof(p_ebd->vol));        
    memcpy(&p_ebd->img_size, p_data + IMAGE_SIZE - 1, sizeof(p_ebd->img_size));        
    memcpy(&p_ebd->drive_mode, p_data + DRIVE_MODE - 1, sizeof(p_ebd->drive_mode));     
    memcpy(&p_ebd->sync_mode, p_data + SYNC_MODE - 1, sizeof(p_ebd->sync_mode));                          
    print_lep_ebd(*p_ebd);
}


// 读取图像并以16进制输出（指定行范围并输出字节编号）
void capture_and_output(int fd, struct v4l2_buffer &buffer, void *buffer_start, size_t fram_count) {
    if (ioctl(fd, VIDIOC_QBUF, &buffer) == -1) {
        perror_exit("Error queueing buffer");
    }

    if (ioctl(fd, VIDIOC_STREAMON, &buffer.type) == -1) {
        perror_exit("Error starting stream");
    }

    if (ioctl(fd, VIDIOC_DQBUF, &buffer) == -1) {
        perror_exit("Error dequeuing buffer");
    }

    for(uint32_t i = 0; i < fram_count; i++) {
    // 输出图像数据为16进制，按行输出，附加字节编号
    uint8_t *data = static_cast<uint8_t *>(buffer_start);

    lep_ebd_t ebd = {0};
    to_ebd_data(data, &ebd); 

    // 输出前600个字节的16进制值
    printf("------------ Image Data (First 600 bytes) ------------\n");
    uint32_t byte_position = 1;  // 字节位置从1开始
    for (uint32_t i = 0; i < 600 && i < buffer.length; ++i) {
        // 按16进制输出字节和位置
        printf("[%03d] %.2X ", byte_position++, data[i]);

        // 每输出16个字节换行
        if (byte_position % 16 == 1) {
            printf("\n");
        }
    }

    // 如果不足600字节，输出剩余部分
    if (byte_position % 16 != 1) {
        printf("\n");
    }
     printf("---------------------------------------------\n");
    }

    if (ioctl(fd, VIDIOC_STREAMOFF, &buffer.type) == -1) {
        perror_exit("Error stopping stream");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <device> <width> <height> <frame_count>" << std::endl;
        return EXIT_FAILURE;
    }

    const char *device = argv[1];
    uint32_t width = std::stoi(argv[2]);
    uint32_t height = std::stoi(argv[3]);
    uint32_t frame_count_max = std::stoi(argv[4]);  // 新增参数：指定要读取的帧数

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

    lep_ebd_t ebd_obj = { 0 };
    // 读取数据并以16进制输出，指定行范围和字节编号
    capture_and_output(fd, buffer, buffer_start, frame_count_max);
    // 释放资源
    munmap(buffer_start, buffer_count);
    close(fd);

    return EXIT_SUCCESS;
}

