#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include "viterbi.h"
#include "orbmatch.h"
#include "ViterbiMatch.h"
#include "coarse_handle.h"

Viterbi viterbiInstance;
// 地图结构体数组
std::vector<ImageData> MapDataArray;
// 匹配结构体数组
std::vector<ImageData> MatchDataArray;

// 地图和匹配结构体构建
void buildImageData(const std::string& map_pose_path, const std::string& wall_map_path, const std::string& front_map_path, 
            const std::string& match_pose_path, const std::string& wall_match_path, const std::string& front_match_path){
    std::cout <<  "开始读取地图数据" << std::endl;
    MapDataArray = readImageDataFromFile(map_pose_path, wall_map_path, front_map_path); // 读取地图数据
    std::cout <<  "地图数据读取完成" << std::endl;
    std::cout <<  "开始读取匹配数据" << std::endl;
    MatchDataArray = readImageDataFromFile(match_pose_path, wall_match_path, front_match_path); // 读取匹配数据
    std::cout <<  "匹配数据读取完成" << std::endl;

    // 测试用
    // 将 MapDataArray 写入文件
    {
        // 第一段
        std::ofstream mapFile("/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/map_descriptors.txt");
        // 第二段
        // std::ofstream mapFile("/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/behind_map_descriptors.txt");
        if (!mapFile.is_open()) {
            std::cerr << "无法打开 map_descriptors.txt 文件" << std::endl;
            return;
        }
        for (const auto& data : MapDataArray) {
            mapFile << data.imageName << " " 
                     << data.x_value << " " 
                     << data.wall_descriptor1 << " " 
                     << data.wall_descriptor2 << " " 
                     << data.wall_descriptor4 << " " 
                     << data.front_descriptor1 << " " 
                     << data.front_descriptor2 << " " 
                     << data.front_descriptor4 << std::endl;
        }
        mapFile.close();
    }

    // 将 MatchDataArray 写入文件
    {
        // 第一段
        std::ofstream matchFile("/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/match_descriptors.txt");
        // 第二段
        // std::ofstream matchFile("/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/behind_match_descriptors.txt");
        if (!matchFile.is_open()) {
            std::cerr << "无法打开 match_descriptors.txt 文件" << std::endl;
            return;
        }
        for (const auto& data : MatchDataArray) {
            matchFile << data.imageName << " " 
                      << data.x_value << " " 
                      << data.wall_descriptor1 << " " 
                      << data.wall_descriptor2 << " " 
                      << data.wall_descriptor4 << " " 
                      << data.front_descriptor1 << " " 
                      << data.front_descriptor2 << " " 
                      << data.front_descriptor4 << std::endl;
        }
        matchFile.close();
    }

    std::cout << "数据写入 map_descriptors.txt 和 match_descriptors.txt 完成" << std::endl;
}

// 第一段定义读取函数，测试用
void first_readImageDataFromTxt() {
    // 定义文件路径
    const std::string mapFilePath = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/front_map_descriptors.txt";
    const std::string matchFilePath = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/front_match_descriptors.txt";

    // 清空之前的数据
    MapDataArray.clear();
    MatchDataArray.clear();

    // 读取 MapDataArray
    std::ifstream mapFile(mapFilePath);
    if (!mapFile.is_open()) {
        std::cerr << "无法打开 " << mapFilePath << " 文件" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(mapFile, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream iss(line);
        ImageData data;
        iss >> data.imageName >> data.x_value 
            >> data.wall_descriptor1 >> data.wall_descriptor2 
            >> data.wall_descriptor4 >> data.front_descriptor1 
            >> data.front_descriptor2 >> data.front_descriptor4;
        MapDataArray.push_back(data);
    }
    mapFile.close();

    // 读取 MatchDataArray
    std::ifstream matchFile(matchFilePath);
    if (!matchFile.is_open()) {
        std::cerr << "无法打开 " << matchFilePath << " 文件" << std::endl;
        return;
    }
    while (std::getline(matchFile, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream iss(line);
        ImageData data;
        iss >> data.imageName >> data.x_value 
            >> data.wall_descriptor1 >> data.wall_descriptor2 
            >> data.wall_descriptor4 >> data.front_descriptor1 
            >> data.front_descriptor2 >> data.front_descriptor4;
        MatchDataArray.push_back(data);
    }
    matchFile.close();

    std::cout << "数据读取完成，MapDataArray 和 MatchDataArray 已填充。" << std::endl;
}

// 第二段定义读取函数，测试用
void second_readImageDataFromTxt() {
    // 定义文件路径
    const std::string mapFilePath = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/behind_map_descriptors.txt";
    const std::string matchFilePath = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/behind_match_descriptors.txt";

    // 清空之前的数据
    MapDataArray.clear();
    MatchDataArray.clear();

    // 读取 MapDataArray
    std::ifstream mapFile(mapFilePath);
    if (!mapFile.is_open()) {
        std::cerr << "无法打开 " << mapFilePath << " 文件" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(mapFile, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream iss(line);
        ImageData data;
        iss >> data.imageName >> data.x_value 
            >> data.wall_descriptor1 >> data.wall_descriptor2 
            >> data.wall_descriptor4 >> data.front_descriptor1 
            >> data.front_descriptor2 >> data.front_descriptor4;
        MapDataArray.push_back(data);
    }
    mapFile.close();

    // 读取 MatchDataArray
    std::ifstream matchFile(matchFilePath);
    if (!matchFile.is_open()) {
        std::cerr << "无法打开 " << matchFilePath << " 文件" << std::endl;
        return;
    }
    while (std::getline(matchFile, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream iss(line);
        ImageData data;
        iss >> data.imageName >> data.x_value 
            >> data.wall_descriptor1 >> data.wall_descriptor2 
            >> data.wall_descriptor4 >> data.front_descriptor1 
            >> data.front_descriptor2 >> data.front_descriptor4;
        MatchDataArray.push_back(data);
    }
    matchFile.close();

    std::cout << "数据读取完成，MapDataArray 和 MatchDataArray 已填充。" << std::endl;
}

// 从 txt 文件读取数据并返回二维数组
std::vector<std::vector<BacktrackData>> readBacktrackDataFromFile(const std::string& filename) {
    std::vector<std::vector<BacktrackData>> backtrack_data_array;  // 二维数组
    
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return backtrack_data_array; // 返回空的数组
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);  // 使用字符串流来分割行
        std::vector<BacktrackData> backtrack_data_row; // 存储当前行的回溯数据

        BacktrackData data;
        while (iss >> data.last_state_index >> data.probability) {
            backtrack_data_row.push_back(data);  // 添加到当前行
        }

        backtrack_data_array.push_back(backtrack_data_row);  // 将当前行添加到二维数组
    }

    inputFile.close(); // 关闭文件
    return backtrack_data_array; // 返回填充好的二维数组
}

// 开始匹配
void ViterbiMatch(const std::string& result_path, const std::string& LoFRT_path, const std::string& backtrack_txt) {
    // 清空 backtrack_txt 文件
    std::ofstream backtrackFile(backtrack_txt, std::ios::trunc);  // 使用 trunch 模式清空文件
    if (!backtrackFile.is_open()) {
        std::cerr << "无法打开回溯数据文件: " << backtrack_txt << std::endl;
        return;
    }
    backtrackFile.close();  // 关闭文件

    // 测试用
    // 读取数据
    first_readImageDataFromTxt();
    // second_readImageDataFromTxt();

    // 打开输出文件
    std::ofstream outputFile(result_path, std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "无法打开输出文件" << std::endl;
        return ;
    }
    // 读取LoFRT特征
    std::vector<ImageFeatures> imgFeaturesList = readAndProcessImageFeaturesFromFile(LoFRT_path);

    // 获取最后一个隐藏状态，回溯用
    int last_hidden_state = -1;

    for(int i = 0; i < MatchDataArray.size(); i++){
        // 判断是否为0时刻
        bool is_time_zero;
        if (i == 0) {
            is_time_zero = true;
        } else {
            is_time_zero = false;
        }

        // 粗定位
        int Index = findClosestXCoordinateIndex(MatchDataArray[i].x_value, MapDataArray);
        
        // viterbi
        std::string LoFRT_name = imgFeaturesList[i].imageName;
        std::string match_name = MatchDataArray[i].imageName;
        if (LoFRT_name != match_name)
        {
            std::cerr << "LoFRT特征文件和匹配数据文件不匹配" << std::endl;
            std::cerr << "LoFRT特征文件名：" << LoFRT_name << std::endl;
            std::cerr << "匹配数据文件名：" << match_name << std::endl;
            return ;
        }
        std::vector<double> LoFRT_descriptor = imgFeaturesList[i].features;
        viterbiInstance.init(MapDataArray, Index);
        viterbiInstance.uptrans_prob(MapDataArray, MatchDataArray[i].x_value, is_time_zero); 
        viterbiInstance.upobr_prob(MatchDataArray[i].wall_descriptor1, MatchDataArray[i].wall_descriptor2, MatchDataArray[i].wall_descriptor4,
                                    MatchDataArray[i].front_descriptor4, LoFRT_descriptor);
        int index_caculate = viterbiInstance.calculate_prob(is_time_zero, backtrack_txt);

        if (i ==  MatchDataArray.size() - 1) {
            last_hidden_state = index_caculate;
        }
    }

    // 读取回溯数据
    std::cout << "开始读取回溯数据" << std::endl;
    std::vector<std::vector<BacktrackData>> backtrack_data = readBacktrackDataFromFile(backtrack_txt);

    // 行反转一下，因为 viterbi 算法是从后往前回溯的
    size_t n = backtrack_data.size(); // 获取二维数组的行数
    for (size_t i = 0; i < n / 2; ++i) {
        std::swap(backtrack_data[i], backtrack_data[n - 1 - i]); // 交换 i 行和倒数第 i 行
    }

    // 回溯
    std::cout << "开始回溯" << std::endl;
    std::vector<int> backtrack_result;
    backtrack_result.resize(MatchDataArray.size());

    // 第一个结果无需回溯
    backtrack_result[0] = last_hidden_state;
    double mu = 0.0;
    double sigma = 10.0;
    for (int i = 1; i < backtrack_data.size(); i++) {
        if (i == 1) {
            double backtrack_prob = 0.0;
            int current_backtrack_index = 0;
            double backtrack_predict_x = MapDataArray[last_hidden_state].x_value;
            std::vector<double> backtrack_trans_prob_array;
            for (int j = 0; j < backtrack_data[i].size(); j++) {
                double backtrack_distance = MapDataArray[backtrack_data[i][j].last_state_index].x_value - backtrack_predict_x;
                double backtrack_trans_prob = (1.0 / (sigma * std::sqrt(2 * M_PI))) * std::exp(-(std::pow(backtrack_distance - mu, 2)) / (2 * std::pow(sigma, 2)));
                backtrack_trans_prob_array.push_back(backtrack_trans_prob);
            }

            double max_backtrack_prob = -1.0;
            int max_index = -1;
            // 计算回溯概率
            for (int j = 0; j < backtrack_data[i].size(); j++) {
                backtrack_prob = backtrack_data[i][j].probability * backtrack_trans_prob_array[j];
                // 检查当前的 backtrack_prob 是否大于当前最大值
                if (backtrack_prob > max_backtrack_prob) {
                    max_backtrack_prob = backtrack_prob; // 更新最大概率
                    max_index = j; // 更新对应的索引
                }
            }
            current_backtrack_index = backtrack_data[i][max_index].last_state_index;
            backtrack_result[i] = current_backtrack_index;
        } else {
            double backtrack_prob = 0.0;
            int current_backtrack_index = 0;
            double backtrack_predict_x = 2 * MapDataArray[backtrack_result[i - 1]].x_value - MapDataArray[backtrack_result[i - 2]].x_value;
            std::vector<double> backtrack_trans_prob_array;
            for (int j = 0; j < backtrack_data[i].size(); j++) {
                double backtrack_distance = MapDataArray[backtrack_data[i][j].last_state_index].x_value - backtrack_predict_x;
                double backtrack_trans_prob = (1.0 / (sigma * std::sqrt(2 * M_PI))) * std::exp(-(std::pow(backtrack_distance - mu, 2)) / (2 * std::pow(sigma, 2)));
                backtrack_trans_prob_array.push_back(backtrack_trans_prob);
            }

            double max_backtrack_prob = -1.0;
            int max_index = -1;
            // 计算回溯概率
            for (int j = 0; j < backtrack_data[i].size(); j++) {
                backtrack_prob = backtrack_data[i][j].probability * backtrack_trans_prob_array[j];
                // 检查当前的 backtrack_prob 是否大于当前最大值
                if (backtrack_prob > max_backtrack_prob) {
                    max_backtrack_prob = backtrack_prob; // 更新最大概率
                    max_index = j; // 更新对应的索引
                }
            }
            current_backtrack_index = backtrack_data[i][max_index].last_state_index;
            backtrack_result[i] = current_backtrack_index;
        }
    }
    // 使用 std::reverse() 进行倒序
    std::reverse(backtrack_result.begin(), backtrack_result.end());

    // 打印结果
    for (int i = 0; i < backtrack_result.size(); i++) {
        outputFile << MatchDataArray[i].imageName << " " << MapDataArray[backtrack_result[i]].imageName << std::endl;
    }

    outputFile.close(); // 关闭输出文件
}