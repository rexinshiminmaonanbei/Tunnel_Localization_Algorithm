#ifndef IMAGEFEATURES_H
#define IMAGEFEATURES_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// 读取LoFRT特征点数据
struct ImageFeatures {
    std::string imageName;      // 图片名称
    std::vector<double> features; // 匹配到的特征点的数目
};

std::vector<ImageFeatures> readImageFeaturesFromFile(const std::string& filename);

// 处理LoFRT数据
std::vector<double> processArray(const std::vector<double>& data);

// 读取并处理LoFRT特征点数据
std::vector<ImageFeatures> readAndProcessImageFeaturesFromFile(const std::string& filename);

#endif // IMAGEFEATURES_H