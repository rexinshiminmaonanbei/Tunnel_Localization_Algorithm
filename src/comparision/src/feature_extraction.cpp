#include "feature_extraction.h"
#include <bitset>
#include <iostream>

// 提取图片描述符并转换为二进制（全局）
std::string extractOrbFeaturesAndWriteToFile_256(const std::string& picturePath) {
    cv::Mat image = cv::imread(picturePath);
    if (image.empty()) {
        std::cerr << "无法读取图像: " << picturePath << std::endl;
        return "";
    }

    cv::Mat imageResized;
    cv::resize(image, imageResized, cv::Size(63, 63));

    cv::KeyPoint keypoint(31, 31, 31);
    std::vector<cv::KeyPoint> keypoints{ keypoint };
    cv::Mat descriptors;
    cv::Ptr<cv::ORB> orb = cv::ORB::create();

    orb->compute(imageResized, keypoints, descriptors);

    std::string binaryString;
    for (int i = 0; i < descriptors.rows; i++) {
        for (int j = 0; j < descriptors.cols; j++) {
            int value = static_cast<int>(descriptors.at<uchar>(i, j));
            std::bitset<8> bits(value);  // 转换成8位二进制
            binaryString += bits.to_string();
        }
    }
    return binaryString;
}

// 提取图片描述符并转换为二进制（四分之一）
std::string extractOrbFeaturesAndWriteToFile_1024(const std::string& picturePath) {
    cv::Mat image = cv::imread(picturePath);
    if (image.empty()) {
        std::cerr << "无法读取图像: " << picturePath << std::endl;
        return ""; 
    }
    
    // 将输入图像分成四个部分
    int rows = image.rows;
    int cols = image.cols;
    
    cv::Mat topLeft = image(cv::Rect(0, 0, cols / 2, rows / 2));
    cv::Mat topRight = image(cv::Rect(cols / 2, 0, cols / 2, rows / 2));
    cv::Mat bottomLeft = image(cv::Rect(0, rows / 2, cols / 2, rows / 2));
    cv::Mat bottomRight = image(cv::Rect(cols / 2, rows / 2, cols / 2, rows / 2));
    
    std::string binaryString2;

    // ORB提取和二进制描述符计算
    for (const auto& subImage : {topLeft, topRight, bottomLeft, bottomRight}) {
        cv::Mat imageResized;
        cv::resize(subImage, imageResized, cv::Size(63, 63));

        cv::KeyPoint keypoint(31, 31, 31);
        std::vector<cv::KeyPoint> keypoints{ keypoint };
        cv::Mat descriptors;
        cv::Ptr<cv::ORB> orb = cv::ORB::create();

        orb->compute(imageResized, keypoints, descriptors);
        
        // 转换成二进制字符串
        for (int i = 0; i < descriptors.rows; i++) {
            for (int j = 0; j < descriptors.cols; j++) {
                int value = static_cast<int>(descriptors.at<uchar>(i, j));
                std::bitset<8> bits(value);  // 转换成8位二进制
                binaryString2 += bits.to_string();
            }
        }
    }

    // 返回拼接后的1024位二进制字符串 (4个256位)
    return binaryString2; 
}

// 提取图片描述符并转换为二进制（十六分之一）
std::string extractOrbFeaturesAndWriteToFile_4096(const std::string& picturePath) {
    cv::Mat image = cv::imread(picturePath);
    if (image.empty()) {
        std::cerr << "无法读取图像: " << picturePath << std::endl;
        return ""; 
    }
    
    int rows = image.rows;
    int cols = image.cols;

    // 计算每个部分的尺寸
    int partRows = rows / 4;
    int partCols = cols / 4;

    std::string binaryString3;

    // 遍历16个子图像
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            // 定义每个子图像的区域
            cv::Mat subImage = image(cv::Rect(j * partCols, i * partRows, partCols, partRows));
            cv::Mat imageResized;
            cv::resize(subImage, imageResized, cv::Size(63, 63));

            cv::KeyPoint keypoint(31, 31, 31);
            std::vector<cv::KeyPoint> keypoints{ keypoint };
            cv::Mat descriptors;
            cv::Ptr<cv::ORB> orb = cv::ORB::create();

            orb->compute(imageResized, keypoints, descriptors);

            // 转换成二进制字符串
            for (int i = 0; i < descriptors.rows; i++) {
                for (int j = 0; j < descriptors.cols; j++) {
                    int value = static_cast<int>(descriptors.at<uchar>(i, j));
                    std::bitset<8> bits(value);  // 转换成8位二进制
                    binaryString3 += bits.to_string();
                }
            }
        }
    }

    // 返回拼接后的4096位二进制字符串 (16个256位)
    return binaryString3; 
}

// 获得多尺度描述符
std::tuple<std::string, std::string, std::string> extractMultiScaleDescriptors(const std::string& picturePath) {
    std::string binaryString1 = extractOrbFeaturesAndWriteToFile_256(picturePath);
    std::string binaryString2 = extractOrbFeaturesAndWriteToFile_1024(picturePath);
    std::string binaryString3 = extractOrbFeaturesAndWriteToFile_4096(picturePath);
    
    // 返回三个二进制字符串
    return std::make_tuple(binaryString1, binaryString2, binaryString3);
}

// 计算两个二进制数之间的海明距离
int hammingDistance(const std::string& str1, const std::string& str2) {
    int distance = 0;
    for (size_t i = 0; i < str1.length(); i++) {
        if (str1[i] != str2[i]) {
            distance++;
        }
    }
    return distance;
}