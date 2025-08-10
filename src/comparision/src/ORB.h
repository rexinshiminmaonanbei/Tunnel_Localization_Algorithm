#ifndef ORB_H
#define ORB_H

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <string>
#include <vector>
#include <filesystem>

// 匹配特征的函数，输入一张图片路径和多个待匹配的图片路径
std::string matchORBFeatures(const std::string& inputImagePath, const std::vector<std::string>& imagePaths);

#endif // ORB_H
