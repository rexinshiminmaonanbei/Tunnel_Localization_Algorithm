#ifndef VERIFY_H
#define VERIFY_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <limits>

// 定义图片名称结构体
struct VerifyStruct {
    std::string match_name;
    std::string map_name;
};

// 定义准确率结构体
struct AccuracyStruct {
    double accuracy_10cm;
    double accuracy_7cm;
    double accuracy_4cm;
};

// 读取txt文件，返回数组
std::vector<VerifyStruct> readFile(const std::string& filename);

// 验证结果和真值数量是否匹配
bool areAllFirstElementsEqual(const std::vector<VerifyStruct>& result, const std::vector<VerifyStruct>& truth);

// 读取文件夹中图片文件名
std::vector<std::string> readImageNames(const std::string& folderPath);

// 复制文件
void copyFile(const std::string &sourcePath, const std::string &destPath);

// 删除文本文件中的指定图片名称的行
void removeImageLines(const std::vector<std::string>& imageNames, std::string& txtFilePath);

// 加上图像级定位的误差
void pictureErrorAddition(const std::vector<std::string>& imageNames_10cm, 
                      const std::vector<std::string>& imageNames_7cm,
                      const std::vector<std::string>& imageNames_4cm, 
                      std::string& txtFilePath);

// 保留文件中每行最小的值
void retainMinimumValuesInFile(const std::string& txtFilePath);

//计算准确率
AccuracyStruct calculateAccuracy(const std::string& results, const std::string& truths, const std::string& wall_map_path,
                                std::string& true_text_path);

#endif