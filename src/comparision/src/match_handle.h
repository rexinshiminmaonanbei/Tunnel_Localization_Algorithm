#include <vector>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "coarse_handle.h"
#include "match.h"
#include "LoFTR.h"

// 地图和匹配结构体构建
void buildImageData(const std::string& map_pose_path, const std::string& wall_map_path, const std::string& front_map_path,
            const std::string& match_pose_path, const std::string& wall_match_path, const std::string& front_match_path);

// 第一段定义读取函数
void first_readImageDataFromTxt();

// 第二段定义读取函数
void second_readImageDataFromTxt();

// 开始匹配
void startMatch(const std::string& result_path, const std::string& LoFTR_path);