#include "viterbi.h"
#include <algorithm>
#include <iostream>

// 初始化地图ORB特征，filepath 为文件夹路径
void Viterbi::init(const std::vector<ImageData>& MapDataArray, int Index) {
    wall_mapORB1.clear();
    wall_mapORB2.clear();
    wall_mapORB4.clear();
    front_mapORB1.clear();
    front_mapORB2.clear();
    front_mapORB4.clear();
    mapImageNames.clear();

    size = MapDataArray.size();
    startIndex = std::max(0, Index - 150);
    endIndex = std::min(static_cast<int>(size), Index + 150);
    
    if (startIndex == 0) {
        endIndex = std::min(static_cast<int>(size), 300);
    }

    if (endIndex == size) {
        startIndex = std::max(0, static_cast<int>(size) - 300);
    }
    real_size = endIndex - startIndex;


    // 遍历范围内的图片
    for (int i = startIndex; i < endIndex; ++i) {
        const auto& entry = MapDataArray[i];
        wall_mapORB1.push_back(MapDataArray[i].wall_descriptor1);
        wall_mapORB2.push_back(MapDataArray[i].wall_descriptor2);
        wall_mapORB4.push_back(MapDataArray[i].wall_descriptor4);
        front_mapORB1.push_back(MapDataArray[i].front_descriptor1);
        front_mapORB2.push_back(MapDataArray[i].front_descriptor2);
        front_mapORB4.push_back(MapDataArray[i].front_descriptor4);
        mapImageNames.push_back(MapDataArray[i].imageName);
    }
}

// 计算发射概率
void Viterbi::upobr_prob(const std::string& wall_descriptor1, const std::string& wall_descriptor2, const std::string& wall_descriptor4,
                         const std::string& front_descriptor4, const std::vector<double>& LoFRT_descriptor) {
    if (!obr_prob.empty()) obr_prob.clear();

    double sum_wall_probabilities = 0.0; // 用于归一化侧视概率
    double sum_front_probabilities = 0.0; // 用于归一化前视概率
    std::vector<double> wall_probabilities(real_size, 0.0); // 用于计算侧视概率
    std::vector<double> front_probabilities(real_size, 0.0); // 用于计算前视概率

    for (int i = 0; i < real_size; ++i) {
        double wall_distance1 = hammingDistance(wall_descriptor1, wall_mapORB1[i]);
        double wall_distance2 = hammingDistance(wall_descriptor2, wall_mapORB2[i]);
        double wall_distance4 = hammingDistance(wall_descriptor4, wall_mapORB4[i]);
        double front_distance4 = hammingDistance(front_descriptor4, front_mapORB4[i]);

        // 如果距离为零，则将其设为0.5以避免除以零
        if (wall_distance1 == 0) wall_distance1 = 0.5;
        if (wall_distance2 == 0) wall_distance2 = 0.5;
        if (wall_distance4 == 0) wall_distance4 = 0.5;
        if (front_distance4 == 0) front_distance4 = 0.5;

        // 累加概率用于归一化
        wall_probabilities[i] = 1 * (1.0 / wall_distance1);  
        // wall_probabilities[i] = 1 * (1.0 / wall_distance1) + 4 * (1.0 / wall_distance2) + 16 * (1.0 / wall_distance4);  
        front_probabilities[i] = (1.0 / front_distance4);
        sum_wall_probabilities += wall_probabilities[i];
        sum_front_probabilities += front_probabilities[i];
    }

    // 归一化概率
    std::vector<double> normalized_wall_probabilitys(real_size, 0.0);
    std::vector<double> normalized_front_probabilitys(real_size, 0.0);

    for (int i = 0; i < real_size; ++i) {
        normalized_wall_probabilitys[i] = wall_probabilities[i] / sum_wall_probabilities;
        normalized_front_probabilitys[i] = front_probabilities[i] / sum_front_probabilities;
    }

    // 计算组合概率
    for (int i = 0; i < real_size; ++i) {
        double normalized_wall_probability = normalized_wall_probabilitys[i];
        double normalized_front_probability = normalized_front_probabilitys[i];

        // 组合概率，按权重相加
        double traditional_combined_probability = normalized_wall_probability * 0.3 + normalized_front_probability * 0.7; 
        // 加入LoFRT特征
        double LoFRT_probability = LoFRT_descriptor[i];
        // double combined_probability = traditional_combined_probability * 0.9 + LoFRT_probability * 0.1;
        double combined_probability = normalized_wall_probability;
        obr_prob.push_back(combined_probability);
    }
}

// 更新状态转移概率
void Viterbi::uptrans_prob(std::vector<ImageData> MapDataArray, double x_value, bool is_zero_t) {
    if (!trans_prob.empty()) trans_prob.clear();
    double mu = 0.0;
    double sigma = 10.0;
    double cauculate_value;
    if (!is_zero_t) {
        for (int i = last_start_index; i < last_end_index; ++i) {
            double sum = 0;
            std::vector<double> temp_trans_prob(real_size, 0.0);

            // 预测当前时刻的cauculate_value
            if (forward_match_results.size() > 1) {
                int size = forward_match_results.size();
                int last_two_index = forward_match_results[size - 2];
                cauculate_value = 2 * MapDataArray[i].x_value - MapDataArray[last_two_index].x_value;
            } else {
                cauculate_value = x_value;
            }

            for (int j = 0; j < real_size; ++j) {
                double distance = (cauculate_value - MapDataArray[startIndex + j].x_value);
                // 使用高斯函数计算概率
                double probability = (1.0 / (sigma * std::sqrt(2 * M_PI))) * std::exp(-(std::pow(distance - mu, 2)) / (2 * std::pow(sigma, 2)));
                temp_trans_prob[j] = probability;
                sum += probability;
            }

            // 归一化
            if (sum > 0) {  // 防止除以零
                for (int j = 0; j < real_size; ++j) {
                    temp_trans_prob[j] /= sum;
                }
            }
            last_start_index = startIndex;
            last_end_index = endIndex;

            trans_prob.push_back(temp_trans_prob);  // 将归一化后的结果加入到 trans_prob
        }
    }
    else {
        for (int i = 0; i < real_size; ++i) {
            double distance = x_value - MapDataArray[startIndex + i].x_value;
            // 使用高斯函数计算概率
            double probability = (1.0 / (sigma * std::sqrt(2 * M_PI))) * std::exp(-(std::pow(distance - mu, 2)) / (2 * std::pow(sigma, 2)));
            initial_trans_prob.push_back(probability);
        }

        // 归一化
        double sum = 0.0;
        for (double prob : initial_trans_prob) {
            sum += prob; // 计算总和
        }

        if (sum > 0) { // 防止除以零
            for (int i = 0; i < initial_trans_prob.size(); ++i) {
                initial_trans_prob[i] /= sum; // 每个元素除以总和
            }
        }
        last_start_index = startIndex;
        last_end_index = endIndex;
    }

    // 更新上一个时刻的序列
    last_x_value.clear();
    for (int i = 0; i < real_size; ++i) {
        last_x_value.push_back(MapDataArray[startIndex + i].x_value);
    }
}

// 写入回溯数据到文件
void writeBacktrackDataToFile(const std::vector<BacktrackData>& backtrack_data, const std::string& filename) {
    // 以追加模式打开文件
    std::ofstream outFile(filename, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }

    // 写入内容
    for (const auto& data : backtrack_data) {
        outFile << data.last_state_index << " " << data.probability << " "; // 写入索引和概率
    }
    
    outFile << std::endl; // 在写入完后换行

    outFile.close(); // 关闭文件
}

// 计算当前概率
int Viterbi::calculate_prob(bool is_zero_t, const std::string& backtrack_txt) {
    if (is_zero_t) {
        // 零时刻，设置出发点
        laststates_prob.resize(real_size, 0);
        if (real_size > 0) {
            // 第一段
            // laststates_prob[real_size - 1] = 1;
            // 第二段
            laststates_prob[real_size - 12] = 1;
        }

        int returnIndex = 0;

        std::vector<double> sum_vec;
        for (int i = 0; i < real_size; ++i) {
            double temp = laststates_prob[i] * initial_trans_prob[i] * obr_prob[i];
            sum_vec.push_back(temp);
        }
        // 找到最大值和对应的索引
        auto max_it = std::max_element(sum_vec.begin(), sum_vec.end());
        int max_index = std::distance(sum_vec.begin(), max_it); // 计算最大值的索引
        returnIndex = startIndex + max_index;

        // 归一化 laststates_prob_copy
        double sum = 0.0;
        for (int i = 0; i < real_size; ++i) {
            sum += sum_vec[i];
        }

        if (sum > 0) {  // 防止除以零
            for (int i = 0; i < real_size; ++i) {
                laststates_prob[i] /= sum;  // 每个元素除以总和
                // std::cout << laststates_prob[i] << " ";
            }
        }

        return returnIndex;
    }
    else {

        double maxReturn = 0;
        int returnIndex = 0;

        std::vector<double> sum_vec;
        for (int i = 0; i < real_size; ++i) {
            double temp_sun = 0;
            for (int j = 0; j < last_x_value.size(); ++j) {
                double temp = laststates_prob[j] * trans_prob[j][i] * obr_prob[i];
                // std::cout << temp << " ";
                // std::cout << "laststates_prob[" << j << "] = " << laststates_prob[j] << " ";
                temp_sun += temp;
            }
            sum_vec.push_back(temp_sun);
        }
        // for (int i = 0; i < sum_vec.size(); ++i) {
        //     std::cout << sum_vec[i] << " ";
        // }
        // std::cout << std::endl;
        // 找到最大值和对应的索引
        auto max_it = std::max_element(sum_vec.begin(), sum_vec.end());
        int max_index = std::distance(sum_vec.begin(), max_it); // 计算最大值的索引
        returnIndex = startIndex + max_index;

        // 归一化 laststates_prob_copy
        double sum = 0.0;
        for (int i = 0; i < real_size; ++i) {
            sum += sum_vec[i];
        }

        if (sum > 0) {  // 防止除以零
            for (int i = 0; i < real_size; ++i) {
                laststates_prob[i] /= sum;  // 每个元素除以总和
            }
        }

        return returnIndex;
    }
}