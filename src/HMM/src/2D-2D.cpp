#include "2D-2D.h"

double estimatePose(const std::string& imgPath1, const std::string& imgPath2) {
    Mat img_1 = imread(imgPath1, cv::IMREAD_COLOR);
    Mat img_2 = imread(imgPath2, cv::IMREAD_COLOR);

    if (!img_1.data || !img_2.data) {
        std::cerr << "无法加载图像!" << std::endl;
        return -1.0;  // 返回 -1 表示读取失败
    }

    vector<KeyPoint> keypoints_1, keypoints_2;
    vector<DMatch> matches;

    Ptr<ORB> detector = ORB::create();
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");

    detector->detect(img_1, keypoints_1);
    detector->detect(img_2, keypoints_2);

    Mat descriptors_1, descriptors_2;
    detector->compute(img_1, keypoints_1, descriptors_1);
    detector->compute(img_2, keypoints_2, descriptors_2);

    vector<DMatch> match;
    matcher->match(descriptors_1, descriptors_2, match);

    double min_dist = 10000, max_dist = 0;

    for (const auto& m : match) {
        double dist = m.distance;
        if (dist < min_dist) min_dist = dist;
        if (dist > max_dist) max_dist = dist;
    }

    for (const auto& m : match) {
        if (m.distance <= std::max(2 * min_dist, 30.0)) {
            matches.push_back(m);
        }
    }

    // 检查是否有足够的匹配点再进行估计
    if (matches.size() < 5) {
        std::cerr << "匹配的点数量不足，无法计算相机位姿!" << std::endl;
        return -1.0; // 返回 -1 表示失败
    }

    Mat R, t;
    Mat K = (Mat_<double>(3, 3) << 511.2450, 0, 320.3848, 
                                    0, 509.9215, 232.5300, 
                                    0, 0, 1);
    K.convertTo(K, CV_32F);
    
    // 确保 K 矩阵正确
    if (K.rows != 3 || K.cols != 3 || K.channels() != 1) {
        std::cerr << "相机内参矩阵 K 不符合要求!" << std::endl;
        return -1.0; // 返回 -1 表示失败
    }

    std::vector<Point2f> points1, points2;
    for (const auto& m : matches) {
        points1.push_back(keypoints_1[m.queryIdx].pt);
        points2.push_back(keypoints_2[m.trainIdx].pt);
    }

    // 计算本质矩阵
    Mat E = findEssentialMat(points1, points2, K);
    // 检查是否成功计算本质矩阵
    if (E.empty()) {
        std::cerr << "本质矩阵计算失败。" << std::endl;
        return -1.0; // 返回 -1 表示失败
    }

    recoverPose(E, points1, points2, K, R, t);

    // 返回 t 的绝对值的 x 轴分量
    return std::abs(t.at<double>(0) * 100);
}