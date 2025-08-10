#ifndef _3D_2D_H
#define _3D_2D_H

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <Eigen/Core>
#include <g2o/core/base_vertex.h>
#include <g2o/core/base_unary_edge.h>
#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/solver.h>
#include <g2o/core/optimization_algorithm_gauss_newton.h>
#include <g2o/solvers/dense/linear_solver_dense.h>
#include <sophus/se3.hpp>

using namespace cv;
using namespace std;

// 定义二维点的动态数组类型
typedef std::vector<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d>> VecVector2d;
// 定义三维点的动态数组类型
typedef std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> VecVector3d;

class VertexPose : public g2o::BaseVertex<6, Sophus::SE3d> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    // 构造函数
    VertexPose() = default;
    
    // 初始化估计到单位变换
    virtual void setToOriginImpl() override;

    // 左乘操作，用于更新相机位姿
    virtual void oplusImpl(const double *update) override;

    // 从输入流读取数据
    virtual bool read(std::istream &in) override;

    // 将数据写入输出流
    virtual bool write(std::ostream &out) const override;
};

class EdgeProjection : public g2o::BaseUnaryEdge<2, Eigen::Vector2d, VertexPose> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    // 构造函数，初始化投影边缘
    EdgeProjection(const Eigen::Vector3d &pos, const Eigen::Matrix3d &K);

    // 计算误差
    virtual void computeError() override;

    // 计算雅可比矩阵
    virtual void linearizeOplus() override;

    // 从输入流读取数据
    virtual bool read(std::istream &in) override;

    // 将数据写入输出流
    virtual bool write(std::ostream &out) const override;

private:
    Eigen::Vector3d _pos3d; // 3D点的位置
    Eigen::Matrix3d _K;     // 相机内参矩阵
};

// 寻找特征匹配
void find_feature_matches(const cv::Mat &img_1, const cv::Mat &img_2, std::vector<cv::KeyPoint> &keypoints_1,
                          std::vector<cv::KeyPoint> &keypoints_2, std::vector<cv::DMatch> &matches);

// 将像素坐标转换为相机的归一化坐标
Point2d pixel2cam(const Point2d &p, const Mat &K);

// 高斯-牛顿法进行捆绑调整
Eigen::Matrix4d bundleAdjustmentGaussNewton(const VecVector3d &points_3d, const VecVector2d &points_2d,
                                const cv::Mat &K, Sophus::SE3d &pose);

// 使用g2o进行捆绑调整
Eigen::Matrix4d bundleAdjustmentG2O(const VecVector3d &points_3d, const VecVector2d &points_2d,
                        const cv::Mat &K, Sophus::SE3d &pose);

// 输入两个相机的RT（旋转矩阵和平移向量），返回相对空间关系的RT
Eigen::Matrix4d computeRelativePose(const Eigen::Matrix4d &T1, const Eigen::Matrix4d &T2);

// 定义处理图像和深度图的函数
std::string computeRT(const std::string &img_path_1, const std::string &img_path_2,
                    const std::string &depth_path_1, const std::string &depth_path_2);

#endif // _3D_2D_H