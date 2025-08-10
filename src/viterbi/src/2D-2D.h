#ifndef _2D_2D_H_
#define _2D_2D_H_

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <vector>

using namespace std;
using namespace cv;

// 主功能函数声明
double estimatePose(const std::string& imgPath1, const std::string& imgPath2);

#endif // _2D_2D_H_