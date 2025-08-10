#include <iostream>
#include <string>
#include "coarse_handle.h"
#include "ViterbiMatch.h"
#include "verify.h"
#include "precise_position.h"

namespace fs = std::filesystem;

int main() {
    // 第一段匹配
    std::string front_map_pose_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/map_pose.txt";
    std::string front_wall_map_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_maps";
    std::string front_front_map_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_front_maps";
    std::string front_match_pose_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/query_pose.txt";
    std::string front_wall_match_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_match";
    std::string front_front_match_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_front_match";
    std::string front_mapDepth_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_maps_depth";
    std::string front_matchDepth_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_match_depth";
    std::string front_result_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/HMM结果/HMM结果(front).txt";
    std::string front_true_result_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/HMM结果/HMM正确的匹配结果(front).txt";
    std::string front_truth_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/true_value.txt";
    std::string front_LoFRT_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/LoFRT.txt";
    std::string front_backtrack_txt = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/backtrack.txt";

    // 匹配
    // std::cout << "开始第一段匹配" << std::endl;
    // // buildImageData(front_map_pose_path, front_wall_map_path, front_front_map_path, front_match_pose_path, front_wall_match_path, front_front_match_path);
    // ViterbiMatch(front_result_path, front_LoFRT_path, front_backtrack_txt);
    // std::cout << "第一段匹配完成！" << std::endl;
    // std::cout << "开始计算准确率..." << std::endl;
    // AccuracyStruct front_accuracies = calculateAccuracy(front_result_path, front_truth_path, front_wall_map_path, front_true_result_path);
    // std::cout << "9.9cm的准确率: " << front_accuracies.accuracy_10cm * 100 << " %" << std::endl;
    // std::cout << "6.6cm的准确率: " << front_accuracies.accuracy_7cm * 100 << " %" << std::endl;
    // std::cout << "3.3cm的准确率: " << front_accuracies.accuracy_4cm * 100 << " %" << std::endl;
    // std::cout << "计算准确率完成！" << std::endl;
    // std::cout << "开始计算误差。" << std::endl;
    // processImages(front_true_result_path, front_wall_map_path, front_wall_match_path, front_mapDepth_path, front_matchDepth_path);
    // processFile(front_true_result_path);
    // double front_avg_error = calculateAverageFromTxt(front_true_result_path);
    // std::cout << "第一段平均误差: " << front_avg_error << "cm" << std::endl;
    // std::cout << "第一段计算误差完成！" << std::endl;

    // 第二段匹配
    std::string behind_map_pose_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/map_pose.txt";
    std::string behind_wall_map_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_maps";
    std::string behind_front_map_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_front_maps";
    std::string behind_match_pose_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/query_pose.txt";
    std::string behind_wall_match_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_match";
    std::string behind_front_match_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_front_match";
    std::string behind_mapDepth_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_maps_depth";
    std::string behind_matchDepth_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_match_depth";
    std::string behind_result_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/HMM结果/HMM结果(behind).txt";
    std::string behind_true_result_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/HMM结果/HMM正确的匹配结果(behind).txt";
    std::string behind_truth_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/true_value.txt";
    std::string behind_LoFRT_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/LoFRT.txt";
    std::string behind_backtrack_txt = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/backtrack.txt";

    // 匹配
    std::cout << "开始第二段匹配" << std::endl;
    // buildImageData(behind_map_pose_path, behind_wall_map_path, behind_front_map_path, behind_match_pose_path, behind_wall_match_path, behind_front_match_path);
    ViterbiMatch(behind_result_path, behind_LoFRT_path, behind_backtrack_txt);
    std::cout << "第二段匹配完成！" << std::endl;
    std::cout << "开始计算准确率..." << std::endl;
    AccuracyStruct behind_accuracies = calculateAccuracy(behind_result_path, behind_truth_path, behind_wall_map_path, behind_true_result_path);
    std::cout << "9.9cm的准确率: " << behind_accuracies.accuracy_10cm * 100 << " %" << std::endl;
    std::cout << "6.6cm的准确率: " << behind_accuracies.accuracy_7cm * 100 << " %" << std::endl;
    std::cout << "3.3cm的准确率: " << behind_accuracies.accuracy_4cm * 100 << " %" << std::endl;
    std::cout << "计算准确率完成！" << std::endl;
    std::cout << "开始计算误差。" << std::endl;
    processImages(behind_true_result_path, behind_wall_map_path, behind_wall_match_path, behind_mapDepth_path, behind_matchDepth_path);
    processFile(behind_true_result_path);
    double behind_avg_error = calculateAverageFromTxt(behind_true_result_path);
    std::cout << "第二段平均误差: " << behind_avg_error << "cm" << std::endl;
    std::cout << "第二段计算误差完成！" << std::endl;

    return 0;
}