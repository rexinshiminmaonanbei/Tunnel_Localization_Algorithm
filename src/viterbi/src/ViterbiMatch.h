#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <opencv2/opencv.hpp>
#include "viterbi.h"
#include "coarse_handle.h"
#include "LoFRT.h"

// 地图和匹配结构体构建
void buildImageData(const std::string& map_pose_path, const std::string& map_path, const std::string& front_map_path,
            const std::string& match_pose_path, const std::string& match_path, const std::string& front_match_path);

// 第一段定义读取函数，测试用
void first_readImageDataFromTxt();

// 第二段定义读取函数，测试用
void second_readImageDataFromTxt();

// 从 txt 文件读取数据并返回二维数组
std::vector<std::vector<BacktrackData>> readBacktrackDataFromFile(const std::string& filename);

// 开始匹配
void ViterbiMatch(const std::string& result_path, const std::string& LoFRT_path, const std::string& backtrack_txt);