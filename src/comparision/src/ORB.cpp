#include "ORB.h"
#include <iostream>
#include <limits>
#include <chrono> // 用于时间计算

std::string matchORBFeatures(const std::string& inputImagePath, const std::vector<std::string>& imagePaths) {
    std::string bestMatch;
    int maxMatches = 0;

    // 读取输入图像
    cv::Mat inputImage = cv::imread(inputImagePath, cv::IMREAD_GRAYSCALE);
    if (inputImage.empty()) {
        std::cerr << "无法读取输入图像: " << inputImagePath << std::endl;
        return "";
    }

    // 创建 ORB 检测器
    cv::Ptr<cv::ORB> orb = cv::ORB::create();

    // 提取输入图像的特征点和描述符
    std::vector<cv::KeyPoint> inputKeypoints;
    cv::Mat inputDescriptors;
    orb->detectAndCompute(inputImage, cv::noArray(), inputKeypoints, inputDescriptors);

    // 检查是否成功提取特征描述符
    if (inputDescriptors.empty()) {
        std::cerr << "未能提取输入图像的特征描述符。" << std::endl;
        return "";
    }

    // 遍历待匹配的所有图像
    for (const auto& mapImagePath : imagePaths) {
        cv::Mat targetImage = cv::imread(mapImagePath, cv::IMREAD_GRAYSCALE);
        if (targetImage.empty()) {
            std::cerr << "无法读取地图图像: " << mapImagePath << std::endl;
            continue;
        }

        // 提取目标图像的特征点和描述符
        std::vector<cv::KeyPoint> targetKeypoints;
        cv::Mat targetDescriptors;
        orb->detectAndCompute(targetImage, cv::noArray(), targetKeypoints, targetDescriptors);

        // 检查是否成功提取特征描述符
        if (targetDescriptors.empty()) {
            std::cerr << "未能提取目标图像的特征描述符。" << std::endl;
            std::cerr << "未能读取目标图像: " << mapImagePath << std::endl;
            continue;
        }

        // 设置 BFMatcher
        cv::BFMatcher matcher(cv::NORM_HAMMING);

        // 确保描述符长度和类型一致
        if (inputDescriptors.type() != targetDescriptors.type() || inputDescriptors.cols != targetDescriptors.cols) {
            std::cerr << "描述符的类型或列数不匹配，无法进行匹配。" << std::endl;
            continue;
        }

        std::vector<std::vector<cv::DMatch>> knnMatches;
        matcher.knnMatch(inputDescriptors, targetDescriptors, knnMatches, 2);

        // 应用比率测试来选择良好的匹配
        std::vector<cv::DMatch> goodMatches;
        for (const auto& match : knnMatches) {
            if (match[0].distance < 0.75 * match[1].distance) {
                goodMatches.push_back(match[0]);
            }
        }

        // 更新最好的匹配结果
        if (goodMatches.size() > maxMatches) {
            maxMatches = goodMatches.size();
            bestMatch = mapImagePath; // 更新最佳匹配图像路径
        }
    }

    // 输出最佳匹配结果
    std::string bestMatchName = bestMatch.empty() ? "无匹配" : std::filesystem::path(bestMatch).filename().stem().string();
    // std::cout << "最佳匹配图像: " << bestMatchName << std::endl; // 输出最佳匹配图像名称

    return bestMatchName; // 返回最佳匹配的图像名称
}