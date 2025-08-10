#ifndef MATCH_H
#define MATCH_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <filesystem>
#include "feature_extraction.h" 
#include "coarse_handle.h"
#include "ORB.h"

namespace fs = std::filesystem;

// 初始化地图ORB特征
void match_init(const std::vector<ImageData>& MapDataArray, int Index);

// 匹配
int match_progress(const std::string& match_path, const std::vector<ImageData>& MapDataArray);
// int match_progress(const std::string& wall_match_descriptor1, const std::string& wall_match_descriptor2, const std::string& wall_match_descriptor4, 
//                   const std::string& front_match_descriptor1, const std::string& front_match_descriptor2, const std::string& front_match_descriptor4,
//                   const std::vector<double>& LoFRT_descriptor,  const std::vector<ImageData>& MapDataArray);

#endif // MATCH_H