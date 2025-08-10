#include "LoFTR.h"

// 读取LoFRT特征点数据
std::vector<ImageFeatures> readImageFeaturesFromFile(const std::string& filename) {
    std::vector<ImageFeatures> imgFeaturesList; // 存储所有的 ImageFeatures
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) { // 逐行读取
            ImageFeatures imgFeatures;
            // 找到第一个空格的位置
            size_t spacePos = line.find(' ');
            if (spacePos != std::string::npos) {
                // 提取图片名称
                imgFeatures.imageName = line.substr(0, spacePos);
                
                // 提取特征点数据
                std::istringstream iss(line.substr(spacePos + 1));
                double feature;
                while (iss >> feature) {
                    imgFeatures.features.push_back(feature);
                    if (imgFeatures.features.size() >= 300) {
                        break; // 限制为300个特征点
                    }
                }
                imgFeaturesList.push_back(imgFeatures); // 将当前的 ImageFeatures 添加到列表中
            }
        }
        file.close();
    } else {
        std::cerr << "无法打开文件: " << filename << std::endl;
    }

    return imgFeaturesList; // 返回存储的所有 ImageFeatures
}

// 处理LoFRT数据
std::vector<double> processArray(const std::vector<double>& data) {
    std::vector<double> result; // 用于存储处理后的结果

    // 遍历输入数组，并处理每个元素
    for (const double& value : data) {
        result.push_back(value / 100 + 1); // 将每个元素除以100并加1
    }

    // 进行归一化处理
    double totalSum = 0.0;
    for (const double& value : result) {
        totalSum += value; // 计算总和
    }

    std::vector<double> normalizedResult;
    if (totalSum > 0) { // 进行归一化，避免除以零
        for (const double& value : result) {
            double normalizedValue = value / totalSum; // 每个元素除以总和
            normalizedResult.push_back(normalizedValue);
        }
    } else {
        // 如果总和为0，直接返回原始结果(通常不应发生)
        return result;
    }

    return normalizedResult; // 返回归一化后的数组
}

// 读取并处理LoFRT特征点数据
std::vector<ImageFeatures> readAndProcessImageFeaturesFromFile(const std::string& filename) {
    std::vector<ImageFeatures> imgFeaturesList = readImageFeaturesFromFile(filename); // 读取特征点数据

    // 处理每个 ImageFeatures 的 features
    for (ImageFeatures& imgFeatures : imgFeaturesList) {
        imgFeatures.features = processArray(imgFeatures.features); // 对特征进行处理
    }

    return imgFeaturesList; // 返回处理后的结构体数组
}