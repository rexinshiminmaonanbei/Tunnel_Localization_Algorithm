#include "match.h"
#include <algorithm>
#include <iostream>

int size;
int Index;
int startIndex;
int endIndex;
int real_size;
std::vector<std::string> wall_mapORB1;
std::vector<std::string> wall_mapORB2;
std::vector<std::string> wall_mapORB4;
std::vector<std::string> front_mapORB1;
std::vector<std::string> front_mapORB2;
std::vector<std::string> front_mapORB4;
std::vector<std::string> mapImageNames;

// 初始化地图ORB特征，filepath 为文件夹路径
void match_init(const std::vector<ImageData>& MapDataArray, int Index) {
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
    // startIndex = 0;
    // endIndex = size;
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

// 匹配
// int match_progress(const std::string& wall_match_descriptor1, const std::string& wall_match_descriptor2, const std::string& wall_match_descriptor4, 
//                   const std::string& front_match_descriptor1, const std::string& front_match_descriptor2, const std::string& front_match_descriptor4,
//                   const std::vector<double>& LoFRT_descriptor,  const std::vector<ImageData>& MapDataArray) {
//     int final_index = -1;
//     int min_distance = 4096 * 16;
//     std::vector<double> distance_vec;
//     distance_vec.resize(real_size);
//     std::vector<double> distance_prob_vec;
//     distance_prob_vec.resize(real_size);
//     double sun_distance = 0;
//     double max_prob = 0;

//     for (int i = 0; i < real_size; ++i) {
//         int wall_distance1 = hammingDistance(wall_match_descriptor1, MapDataArray[startIndex + i].wall_descriptor1);
//         int wall_distance2 = hammingDistance(wall_match_descriptor2, MapDataArray[startIndex + i].wall_descriptor2);
//         int wall_distance4 = hammingDistance(wall_match_descriptor4, MapDataArray[startIndex + i].wall_descriptor4);
//         int front_distance1 = hammingDistance(front_match_descriptor1, MapDataArray[startIndex + i].front_descriptor1);
//         int front_distance2 = hammingDistance(front_match_descriptor2, MapDataArray[startIndex + i].front_descriptor2);
//         int front_distance4 = hammingDistance(front_match_descriptor4, MapDataArray[startIndex + i].front_descriptor4);
//         // int distance = 16 * distance1 + 4 * distance2 + distance4;
//         int distance = 5000 * wall_distance1 + wall_distance2 / 4 + wall_distance4 / 16;
//         // int distance = wall_distance1 + wall_distance2 + wall_distance4 /*+ front_distance1 + front_distance2 + front_distance4*/;
//         // int distance = wall_distance1 + front_distance1;
//         double double_distance = static_cast<double>(distance);

//         distance_vec[i] = 1 / double_distance;
//         sun_distance += 1 / double_distance;
        
//         // if (distance < min_distance) { // 找到更小的海明距离
//         //     min_distance = distance;
//         //     final_index = startIndex + i; // 更新最终索引
//         // }
//     }

//     for (int i = 0; i < real_size; ++i) {
//         distance_prob_vec[i] = distance_vec[i] / sun_distance;
//     }

//     for (int i = 0; i < real_size; ++i) {
//         // double prob = 0.73 * distance_prob_vec[i] + 0.27 * LoFRT_descriptor[i];
//         double prob = distance_prob_vec[i];
//         // 更新最大值和最大值的索引
//         if (prob > max_prob) {
//             max_prob = prob;
//             final_index =startIndex  + i;
//         }
//     }
    
//     return final_index; // 返回找到的最小海明距离对应的索引
// }

int match_progress(const std::string& match_path, const std::vector<ImageData>& MapDataArray) {
    int final_index = -1;
    std::vector<std::string> map_path_vec;

    std::string map_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_maps/";
    // std::string map_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_maps/";
    for (int i = 0; i < real_size; ++i) {
        std::string each_map_path = map_path + MapDataArray[startIndex + i].imageName + ".png";
        map_path_vec.push_back(each_map_path);
    }

    std::string result_name = matchORBFeatures(match_path, map_path_vec);

    for (size_t i = 0; i < MapDataArray.size(); ++i) {
        if (result_name == MapDataArray[i].imageName) {
            final_index = i;
            break;
        }
    }
    
    return final_index; // 返回找到的最小海明距离对应的索引
}