#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <vector>
#include <cmath>
#include <limits>
#include <filesystem>
#include <iomanip>
#include "3D_2D.h"
#include "local_ORB.h"
#include "2D-2D.h"

using namespace std;

// 读取图片名称
void readImagePaths(const string &txtFilePath, 
                    vector<string> &matchImageNames, 
                    vector<string> &mappedImageNames);

// 检查文件是否存在的函数声明
bool fileExists(const string &path);

// 计算更小的误差
std::vector<double> compareAndSelectMin(const std::vector<double>& array1, const std::vector<double>& array2);

// 计算精确位置
void processImages(const string &txtFilePath,
                   const std::string &mapRgbPath, 
                   const std::string &matchRgbPath,
                   const std::string &mapDepthPath, 
                   const std::string &matchDepthPath);

// 得到最终的误差
void processFile(const std::string& filename);

// 求平均误差
double calculateAverageFromTxt(const std::string& filePath);