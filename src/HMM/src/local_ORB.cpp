#include "local_ORB.h"

using namespace cv;
using namespace std;

// 计算两张图片中匹配点的横坐标差
double calculateXCoordinateDifferences(const std::string& imgPath1, const std::string& imgPath2) {
    // 读取两张图片
    Mat img1 = imread(imgPath1);
    Mat img2 = imread(imgPath2);

    // 检查图片是否成功加载
    if (img1.empty() || img2.empty()) {
        std::cout << "Error loading images!" << std::endl;
        return {};
    }
    
    // 创建 ORB 特征检测器
    Ptr<ORB> orb = ORB::create();

    // 存储关键点和描述子
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;

    // 提取特征点和描述子
    orb->detectAndCompute(img1, noArray(), keypoints1, descriptors1);
    orb->detectAndCompute(img2, noArray(), keypoints2, descriptors2);

    // 如果没有找到足够的匹配点，则返回空数组
    if (keypoints1.empty() || keypoints2.empty()) {
        std::cout << "No keypoints found in one or both images." << std::endl;
        return {};
    }

    // 使用 BFMatcher 进行匹配
    BFMatcher matcher(NORM_HAMMING);
    std::vector<DMatch> matches;
    matcher.match(descriptors1, descriptors2, matches);

    // 如果没有找到匹配点，则返回空数组
    if (matches.empty()) {
        std::cout << "No matches found." << std::endl;
        return {};
    }

    // 根据匹配质量（距离）对匹配结果进行排序
    std::sort(matches.begin(), matches.end(), [](const DMatch& a, const DMatch& b) {
        return a.distance < b.distance;
    });

    // 限制匹配点数量为前50个
    const int maxMatches = 50;
    if (matches.size() > maxMatches) {
        matches.resize(maxMatches);
    }

    // 存储横坐标差
    std::vector<double> xDiffs;

    for (const auto& match : matches) {
        // 获取第一张图片和第二张图片的关键点
        Point2f pt1 = keypoints1[match.queryIdx].pt;
        Point2f pt2 = keypoints2[match.trainIdx].pt;

        // 计算横坐标差
        double xDiff = pt1.x - pt2.x;
        xDiffs.push_back(xDiff);
    }
    // 计算横坐标差的平均值
    double xDiffSum = std::accumulate(xDiffs.begin(), xDiffs.end(), 0.0);
    double xDiffAverage = xDiffSum / xDiffs.size();
    double distance = (xDiffAverage / img1.cols) * 2.942690190256;

    // 返回距离
    return distance;
}