#ifndef LOCAL_ORB_H
#define LOCAL_ORB_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

// 计算两张图片的x坐标差值
double calculateXCoordinateDifferences(const std::string& imgPath1, const std::string& imgPath2);

#endif // LOCAL_ORB_H