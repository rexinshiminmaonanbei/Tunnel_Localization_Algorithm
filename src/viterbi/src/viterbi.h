#ifndef VITERBI_H
#define VITERBI_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <filesystem>
#include <numeric>
#include "orbmatch.h" 
#include "coarse_handle.h"

namespace fs = std::filesystem;

// 回溯结构体
struct BacktrackData {
    int last_state_index;
    double probability;
};

// 写入回溯数据到文件
void writeBacktrackDataToFile(const std::vector<BacktrackData>& backtrack_data, const std::string& filename);

class Viterbi {
public:
    std::vector<int> states;
    std::vector<int> forward_match_results;
    std::vector<double> laststates_prob; 
    std::vector<std::vector<double>> trans_prob; 
    std::vector<double> obr_prob; 
    std::vector<double> initial_trans_prob;
    std::vector<double> last_x_value;
    std::vector<std::string> wall_mapORB1; 
    std::vector<std::string> wall_mapORB2; 
    std::vector<std::string> wall_mapORB4; 
    std::vector<std::string> front_mapORB1; 
    std::vector<std::string> front_mapORB2; 
    std::vector<std::string> front_mapORB4; 
    std::vector<std::string> mapImageNames;
    int size;
    int Index;
    int startIndex;
    int endIndex;
    int real_size;
    int last_start_index;
    int last_end_index;

    // 初始化地图ORB特征
    void init(const std::vector<ImageData>& MapDataArray, int Index);

    // 更新发射概率
    void upobr_prob(const std::string& wall_descriptor1, const std::string& wall_descriptor2, const std::string& wall_descriptor4,
                    const std::string& front_descriptor4, const std::vector<double>& LoFRT_descriptor);

    // 更新状态转移概率
    void uptrans_prob(std::vector<ImageData> MapDataArray, double x_value, bool is_zero_t);

    // 计算当前概率
    int calculate_prob(bool is_zero_t, const std::string& backtrack_txt);
};

#endif // VITERBI_H