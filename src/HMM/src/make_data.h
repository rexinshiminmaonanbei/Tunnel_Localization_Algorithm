#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <ros/ros.h>
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

// 进度条
void updateProgressBar(size_t processed, size_t total, std::chrono::steady_clock::time_point startTime);

// 读取rosbag文件并保存图像到文件夹中
void saveImagesFromRosbag_RGB(const std::string& bagFilePath, const std::string& topicName, const std::string& saveDirectory);

// 读取rosbag文件并保存图像到文件夹中（深度图）
void saveImagesFromRosbag_Depth(const std::string& bagFilePath, const std::string& topicName, const std::string& saveDirectory);

// 检查输入目录中的所有图像是否可读取
void checkImagesReadable(const std::string& inputDirectory);

// 查找最接近的图片名称
void findAndCopyClosestImages(const std::string& folderA, const std::string& folderB, const std::string& folderC);

// 在txt时间戳部分加小数点
void addDecimalPointToFile(const std::string& filePath);

// 读取图片名称
void saveImageNamesToTxt(const std::string& folderPath, const std::string& txtFilePath);

// txt根据图片名称赋值
void processTxtFiles(const std::string& txtAPath, const std::string& txtBPath);

// txt做插值之前加入空格
void modifyTxtFile(const std::string& filePath);

// 做txt文件插值
void processTxtFile(const std::string& filePath);

// 统一图片名称
void matchImagesAndRename_RGB(const std::string& folderA, const std::string& folderB, const std::string& folderC);

// 统一图片名称（深度）
void matchImagesAndRename_depth(const std::string& folderA, const std::string& folderB, const std::string& folderC);

// 制作地图和查询文件夹（RGB）
void copyImages(const std::string& folderA, const std::string& folderB, const std::string& folderC);

// 比较图片名称是否一致
void compareImageNames(const std::string& folderA, const std::string& folderB);

// 制作查询文件夹
void copyEveryNthImage(const std::string& sourceFolder, const std::string& destFolder, size_t n);

// 制作查询txt
void extractImageNamesToTxt(const std::string& imageFolder, const std::string& txtPath, const std::string& outputTxtPath);

// 制作查询深度图
void copyImagesAndReport(const std::string& folderA, const std::string& folderB, const std::string& folderC);

// 制作地图深度图
void copyImagesFromTxt(const std::string& txtFilePath, const std::string& folderA, const std::string& folderB);