#ifndef FEATURE_EXTRACTION_H
#define FEATURE_EXTRACTION_H

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <string>
#include <tuple>

// 提取图片描述符并转换为二进制（全局）
std::string extractOrbFeaturesAndWriteToFile_256(const std::string& picturePath);

// 提取图片描述符并转换为二进制（四分之一）
std::string extractOrbFeaturesAndWriteToFile_1024(const std::string& picturePath);

// 提取图片描述符并转换为二进制（十六分之一）
std::string extractOrbFeaturesAndWriteToFile_4096(const std::string& picturePath);

// 多尺度特征权重分配
std::tuple<std::string, std::string, std::string> extractMultiScaleDescriptors(const std::string& picturePath);

// 计算两个二进制数之间的海明距离
int hammingDistance(const std::string& str1, const std::string& str2);

#endif // FEATURE_EXTRACTION_H