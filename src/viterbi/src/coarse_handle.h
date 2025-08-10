#ifndef COARSE_HANDLE_H
#define COARSE_HANDLE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <limits>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include "orbmatch.h"

namespace fs = std::filesystem;

// 图像结构体
struct ImageData {
    std::string imageName;        // 图像名称
    double x_value;    // 表示X轴数据的double类型数组
    std::string wall_descriptor1;
    std::string wall_descriptor2;
    std::string wall_descriptor4;
    std::string front_descriptor1;
    std::string front_descriptor2;
    std::string front_descriptor4;
};

// 打印进度条的函数
void printProgressBar(double progress);

// 从文件中读取图片名称和x轴坐标，并保存到结构体数组中
std::vector<ImageData> readImageDataFromFile(const std::string& pose_path, const std::string& wall_image_path, const std::string& front_image_path);

// 根据图片名称查找x轴坐标的索引
int findClosestXCoordinateIndex(const double& queryXCoordinate, const std::vector<ImageData>& imageDataArray);

#endif // COARSE_HANDLE_H