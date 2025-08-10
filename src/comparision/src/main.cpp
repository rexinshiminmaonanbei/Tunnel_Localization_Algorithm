#include <iostream>
#include <string>
#include <filesystem>
#include "match_handle.h"
#include "verify.h"
#include "ORB.h"

namespace fs = std::filesystem;

int main() {
    // 第一段
    std::string front_map_pose_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/map_pose.txt";
    std::string front_match_pose_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/query_pose.txt";
    std::string front_wall_map_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_maps";
    std::string front_front_map_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_front_match";
    std::string front_wall_match_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_match";
    std::string front_front_match_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_front_match";
    std::string front_result_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/多视角全局ORB/匹配结果.txt";
    std::string front_truth_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/true_value.txt";
    std::string front_LoFTR_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/LoFRT.txt";
    std::string front_LoFTR_results_path = "/media/rxsmmnb/dying_gull/LoFTR/data/front/LoFTR_results.txt";
    std::string front_DBoW3_results_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/DBoW3结果/DBoW3(front).txt";
    std::string front_VLAD_results_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/VLAD结果/VLAD(front).txt";

    // buildImageData(front_map_pose_path, front_wall_map_path, front_front_map_path, front_match_pose_path, front_wall_match_path, front_front_match_path);
    // std::cout << "开始匹配" << std::endl;
    // startMatch(front_result_path, front_LoFTR_path);
    // std::cout << "匹配完成" << std::endl;
    // std::cout << "开始计算准确率..." << std::endl;
    // AccuracyStruct front_accuracies = calculateAccuracy(front_VLAD_results_path, front_truth_path, front_wall_map_path);
    // std::cout << "9.9cm的准确率: " << front_accuracies.accuracy_10cm * 100 << " %" << std::endl;
    // std::cout << "6.6cm的准确率: " << front_accuracies.accuracy_7cm * 100 << " %" << std::endl;
    // std::cout << "3.3cm的准确率: " << front_accuracies.accuracy_4cm * 100 << " %" << std::endl;
    // std::cout << "计算准确率完成！" << std::endl;

    // 第二段
    std::string behind_map_pose_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/map_pose.txt";
    std::string behind_match_pose_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/query_pose.txt";
    std::string behind_wall_map_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_maps";
    std::string behind_front_map_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_front_match";
    std::string behind_wall_match_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_match";
    std::string behind_front_match_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_front_match";
    std::string behind_result_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/对比txt/多视角全局ORB/匹配结果.txt";
    std::string behind_truth_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/true_value.txt";
    std::string behind_LoFTR_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/LoFRT.txt";
    std::string behind_LoFTR_results_path = "/media/rxsmmnb/dying_gull/LoFTR/data/behind/LoFTR_results.txt";
    std::string behind_DBoW3_results_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/DBoW3结果/DBoW3(behind).txt";
    std::string behind_VLAD_results_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/VLAD结果/VLAD(behind).txt";

    // buildImageData(behind_map_pose_path, behind_wall_map_path, behind_front_map_path, behind_match_pose_path, behind_wall_match_path, behind_front_match_path);
    // std::cout << "开始匹配" << std::endl;
    // startMatch(behind_result_path, behind_LoFTR_path);
    // // startMatch(behind_result_path, behind_LoFTR_path);
    // std::cout << "匹配完成" << std::endl;
    std::cout << "开始计算准确率..." << std::endl;
    AccuracyStruct behind_accuracies = calculateAccuracy(behind_VLAD_results_path, behind_truth_path, behind_wall_map_path);
    std::cout << "9.9cm的准确率: " << behind_accuracies.accuracy_10cm * 100 << " %" << std::endl;
    std::cout << "6.6cm的准确率: " << behind_accuracies.accuracy_7cm * 100 << " %" << std::endl;
    std::cout << "3.3cm的准确率: " << behind_accuracies.accuracy_4cm * 100 << " %" << std::endl;
    std::cout << "计算准确率完成！" << std::endl;
    return 0;
}