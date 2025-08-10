#include "3D_2D.h"

void VertexPose::setToOriginImpl() {
    _estimate = Sophus::SE3d(); // 将估计初始化为单位变换
}

void VertexPose::oplusImpl(const double *update) {
    Eigen::Matrix<double, 6, 1> update_eigen;
    update_eigen << 
        update[0], update[1], update[2], // 旋转部分
        update[3], update[4], update[5]; // 平移部分

    // 使用SE3的指数映射进行左乘更新
    _estimate = Sophus::SE3d::exp(update_eigen) * _estimate;
}

bool VertexPose::read(std::istream &in) {
    // 实际实现中可以添加读取逻辑
    return true;
}

bool VertexPose::write(std::ostream &out) const {
    // 实际实现中可以添加写入逻辑
    return true;
}

EdgeProjection::EdgeProjection(const Eigen::Vector3d &pos, const Eigen::Matrix3d &K)
    : _pos3d(pos), _K(K) {}

void EdgeProjection::computeError() {
    const VertexPose *v = static_cast<VertexPose *>(_vertices[0]);
    Sophus::SE3d T = v->estimate(); // 获取顶点的位姿估计

    // 将3D点转换为像素坐标
    Eigen::Vector3d pos_pixel = _K * (T * _pos3d);
    pos_pixel /= pos_pixel[2]; // 归一化，将Z轴应用于比例

    // 计算误差，测量值与计算的像素坐标之间的差值
    _error = _measurement - pos_pixel.head<2>(); // 只取前2个坐标
}

void EdgeProjection::linearizeOplus() {
    const VertexPose *v = static_cast<VertexPose *>(_vertices[0]);
    Sophus::SE3d T = v->estimate(); // 获取顶点的位姿估计

    // 计算相机坐标系中的位置
    Eigen::Vector3d pos_cam = T * _pos3d;

    // 从内参矩阵中获取参数
    double fx = _K(0, 0); // x方向焦距
    double fy = _K(1, 1); // y方向焦距
    double cx = _K(0, 2); // 主点x坐标
    double cy = _K(1, 2); // 主点y坐标

    // 获取相机坐标系中的坐标
    double X = pos_cam[0];
    double Y = pos_cam[1];
    double Z = pos_cam[2];
    double Z2 = Z * Z; // Z的平方用于后续计算

    // 构建雅可比矩阵
    _jacobianOplusXi << 
        -fx / Z, 0, fx * X / Z2, fx * X * Y / Z2, -fx - fx * X * X / Z2, fx * Y / Z,
        0, -fy / Z, fy * Y / (Z * Z), fy + fy * Y * Y / Z2, -fy * X * Y / Z2, -fy * X / Z;
}

bool EdgeProjection::read(std::istream &in) {
    // 实际实现中可以添加读取逻辑
    return true;
}

bool EdgeProjection::write(std::ostream &out) const {
    // 实际实现中可以添加写入逻辑
    return true;
}

// 寻找特征匹配
void find_feature_matches(const cv::Mat &img_1, const cv::Mat &img_2, std::vector<cv::KeyPoint> &keypoints_1,
                          std::vector<cv::KeyPoint> &keypoints_2, std::vector<cv::DMatch> &matches) {
    // 初始化描述子矩阵
    cv::Mat descriptors_1, descriptors_2;

    // 创建ORB特征检测器和描述子提取器
    Ptr<cv::FeatureDetector> detector = cv::ORB::create();
    Ptr<cv::DescriptorExtractor> descriptor = cv::ORB::create();

    // 创建描述子匹配器，使用Hamming距离的暴力匹配器
    Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");

    // 第一步: 检测两个图像中的关键点
    detector->detect(img_1, keypoints_1); // 检测第一幅图像的关键点
    detector->detect(img_2, keypoints_2); // 检测第二幅图像的关键点

    // 第二步: 根据角点位置计算BRIEF描述子
    descriptor->compute(img_1, keypoints_1, descriptors_1); // 计算第一幅图像的描述子
    descriptor->compute(img_2, keypoints_2, descriptors_2); // 计算第二幅图像的描述子

    // 第三步: 对两幅图像中的描述子进行匹配，使用Hamming距离
    std::vector<cv::DMatch> match; // 用于存储匹配结果
    matcher->match(descriptors_1, descriptors_2, match); // 匹配描述子

    // 第四步: 匹配点对筛选
    double min_dist = 10000, max_dist = 0;

    // 找出所有匹配之间的最小距离和最大距离
    for (int i = 0; i < descriptors_1.rows; i++) {
        double dist = match[i].distance; // 获取当前匹配点的距离
        if (dist < min_dist) min_dist = dist; // 更新最小距离
        if (dist > max_dist) max_dist = dist; // 更新最大距离
    }

    // 输出最大和最小距离
    // printf("-- Max dist : %f \n", max_dist);
    // printf("-- Min dist : %f \n", min_dist);

    // 筛选良好匹配点，实际距离小于最大距离的两倍（或者设定的下限30.0）
    for (int i = 0; i < descriptors_1.rows; i++) {
        if (match[i].distance <= std::max(2 * min_dist, 30.0)) {
            matches.push_back(match[i]); // 将良好匹配添加到输出向量
        }
    }
}

// 将像素坐标转换为相机的归一化坐标
Point2d pixel2cam(const Point2d &p, const Mat &K) {
    // 使用相机内参矩阵 K 将像素坐标转换为归一化坐标
    return Point2d(
        (p.x - K.at<double>(0, 2)) / K.at<double>(0, 0),  // 归一化 x 坐标
        (p.y - K.at<double>(1, 2)) / K.at<double>(1, 1)   // 归一化 y 坐标
    );
}

// 高斯-牛顿法进行捆绑调整
Eigen::Matrix4d bundleAdjustmentGaussNewton(const VecVector3d &points_3d, const VecVector2d &points_2d,
                                const cv::Mat &K, Sophus::SE3d &pose) {
    typedef Eigen::Matrix<double, 6, 1> Vector6d; // 定义6维向量类型
    const int iterations = 10;                     // 迭代次数
    double cost = 0, lastCost = 0;                 // 成本函数的当前值和上一个值

    // 从相机内参矩阵 K 中提取参数
    double fx = K.at<double>(0, 0); // x方向的焦距
    double fy = K.at<double>(1, 1); // y方向的焦距
    double cx = K.at<double>(0, 2); // 主点x坐标
    double cy = K.at<double>(1, 2); // 主点y坐标

    // 开始迭代
    for (int iter = 0; iter < iterations; iter++) {
        Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero(); // 哈希矩阵
        Vector6d b = Vector6d::Zero(); // 残差向量

        cost = 0; // 初始化成本
        // 计算成本和雅可比矩阵
        for (int i = 0; i < points_3d.size(); i++) {
            // 使用当前位姿计算3D点在相机坐标系中的坐标
            Eigen::Vector3d pc = pose * points_3d[i];
            double inv_z = 1.0 / pc[2]; // Z轴的倒数
            double inv_z2 = inv_z * inv_z; // Z轴倒数的平方
            
            // 投影3D点到2D空间
            Eigen::Vector2d proj(fx * pc[0] * inv_z + cx, fy * pc[1] * inv_z + cy);

            // 计算误差
            Eigen::Vector2d e = points_2d[i] - proj;

            // 更新成本
            cost += e.squaredNorm();

            // 计算雅可比矩阵
            Eigen::Matrix<double, 2, 6> J;
            J << -fx * inv_z, 0, fx * pc[0] * inv_z2, 
                 fx * pc[0] * pc[1] * inv_z2, -fx - fx * pc[0] * pc[0] * inv_z2, 
                 fx * pc[1] * inv_z,
                 0, -fy * inv_z, fy * pc[1] * inv_z2, 
                 fy + fy * pc[1] * pc[1] * inv_z2, -fy * pc[0] * pc[1] * inv_z2, 
                 -fy * pc[0] * inv_z;

            // 累加哈希矩阵和残差向量
            H += J.transpose() * J;
            b += -J.transpose() * e;
        }

        // 解线性系统
        Vector6d dx; 
        dx = H.ldlt().solve(b); // 使用LLT分解求解

        if (isnan(dx[0])) { // 检查是否出现了NaN
            std::cout << "result is nan!" << std::endl;
            break; // 提前退出
        }

        // 如果成本没有减小，则退出迭代
        if (iter > 0 && cost >= lastCost) {
            std::cout << "cost: " << cost << ", last cost: " << lastCost << std::endl;
            break; // 停止优化
        }

        // 更新相机位姿
        pose = Sophus::SE3d::exp(dx) * pose; // 通过指数映射更新位姿
        lastCost = cost; // 更新最后的成本

        // 输出当前迭代的信息
        // std::cout << "iteration " << iter << " cost=" 
        //           << std::setprecision(12) << cost << std::endl;

        // 检查收敛条件
        if (dx.norm() < 1e-6) {
            break; // 收敛，停止迭代
        }
    }

    // 创建一个4x4的RT矩阵来表示位姿关系
    Eigen::Matrix4d RT;   
    RT.block<3, 3>(0, 0) = pose.rotationMatrix(); // 将旋转部分填入RT矩阵
    RT.block<3, 1>(0, 3) = pose.translation();    // 将平移部分填入RT矩阵
    RT(3, 0) = 0; RT(3, 1) = 0; RT(3, 2) = 0; RT(3, 3) = 1; // 设置齐次坐标

    // 输出最终估计的位姿
    std::cout << "pose by g-n: \n" << pose.matrix() << std::endl;
    return RT;
}

// 使用g2o进行捆绑调整
Eigen::Matrix4d bundleAdjustmentG2O(const VecVector3d &points_3d, const VecVector2d &points_2d,
                        const cv::Mat &K, Sophus::SE3d &pose) {
    // 构建图优化，使用g2o的块求解器
    typedef g2o::BlockSolver<g2o::BlockSolverTraits<6, 3>> BlockSolverType; // 维度6，地标3
    typedef g2o::LinearSolverDense<BlockSolverType::PoseMatrixType> LinearSolverType; // 线性求解器类型

    // 创建高斯-牛顿优化算法
    auto solver = new g2o::OptimizationAlgorithmGaussNewton(
        std::make_unique<BlockSolverType>(std::make_unique<LinearSolverType>()));

    g2o::SparseOptimizer optimizer;        // 图模型
    optimizer.setAlgorithm(solver);       // 设置优化算法
    optimizer.setVerbose(false);           // 开启调试输出

    // 创建相机位姿顶点
    VertexPose *vertex_pose = new VertexPose();
    vertex_pose->setId(0);                // 设置顶点ID
    vertex_pose->setEstimate(Sophus::SE3d()); // 初始化估计为单位变换
    optimizer.addVertex(vertex_pose);     // 将顶点添加到优化器中

    // 将相机内参矩阵转换为Eigen类型
    Eigen::Matrix3d K_eigen; 
    K_eigen << 
        K.at<double>(0, 0), K.at<double>(0, 1), K.at<double>(0, 2),
        K.at<double>(1, 0), K.at<double>(1, 1), K.at<double>(1, 2),
        K.at<double>(2, 0), K.at<double>(2, 1), K.at<double>(2, 2);

    // 添加边缘，即从3D点到2D点的投影
    int index = 1; // 边索引
    for (size_t i = 0; i < points_2d.size(); ++i) {
        auto p2d = points_2d[i];           // 当前2D点
        auto p3d = points_3d[i];           // 当前3D点
        EdgeProjection *edge = new EdgeProjection(p3d, K_eigen); // 创建投影边
        edge->setId(index);                 // 设置边ID
        edge->setVertex(0, vertex_pose);    // 连接到相机位姿顶点
        edge->setMeasurement(p2d);          // 设置测量值
        edge->setInformation(Eigen::Matrix2d::Identity()); // 设置信息矩阵
        optimizer.addEdge(edge);            // 将边添加到优化器中
        index++;                             // 增加边索引
    }

    // 初始化优化并执行10次优化迭代
    optimizer.initializeOptimization();    // 初始化优化
    optimizer.optimize(10);                // 执行10次优化迭代

    // 输出优化后的位姿估计
    // std::cout << vertex_pose->estimate().matrix() << std::endl;

    pose = vertex_pose->estimate();        // 更新传入的位姿参数

    // 创建一个4x4的RT矩阵来表示位姿关系
    Eigen::Matrix4d RT;   
    RT.block<3, 3>(0, 0) = pose.rotationMatrix(); // 将旋转部分填入RT矩阵
    RT.block<3, 1>(0, 3) = pose.translation();    // 将平移部分填入RT矩阵
    RT(3, 0) = 0; RT(3, 1) = 0; RT(3, 2) = 0; RT(3, 3) = 1; // 设置齐次坐标
    return RT;
}

// 输入两个相机的RT（旋转矩阵和平移向量），返回相对空间关系的RT
Eigen::Matrix4d computeRelativePose(const Eigen::Matrix4d &T1, const Eigen::Matrix4d &T2) {
    // 提取旋转和位移
    Eigen::Matrix3d R1 = T1.block<3, 3>(0, 0); // 从第一个相机获取旋转矩阵
    Eigen::Vector3d t1 = T1.block<3, 1>(0, 3); // 从第一个相机获取位移向量
    Eigen::Matrix3d R2 = T2.block<3, 3>(0, 0); // 从第二个相机获取旋转矩阵
    Eigen::Vector3d t2 = T2.block<3, 1>(0, 3); // 从第二个相机获取位移向量

    // 计算相机2相对于相机1的旋转和位移
    Eigen::Matrix3d R_relative = R1.transpose() * R2; // 相对旋转矩阵
    Eigen::Vector3d t_relative = R1.transpose() * (t2 - t1); // 相对位移向量

    // 组合成齐次变换矩阵
    Eigen::Matrix4d T_relative = Eigen::Matrix4d::Identity(); // 初始化为单位矩阵
    T_relative.block<3, 3>(0, 0) = R_relative; // 填充旋转部分
    T_relative.block<3, 1>(0, 3) = t_relative; // 填充位移部分

    return T_relative; // 返回计算得到的相对RT矩阵
}

// 定义处理图像和深度图的函数
std::string computeRT(const std::string &img_path_1, const std::string &img_path_2,
                   const std::string &depth_path_1, const std::string &depth_path_2) {
    // 读取图像
    Mat img_1 = imread(img_path_1, cv::IMREAD_COLOR);
    Mat img_2 = imread(img_path_2, cv::IMREAD_COLOR);
    assert(img_1.data && img_2.data && "Cannot load images!");

    vector<KeyPoint> keypoints_1, keypoints_2; // 存储关键点
    vector<DMatch> matches; // 存储匹配结果

    // 找到特征匹配
    find_feature_matches(img_1, img_2, keypoints_1, keypoints_2, matches);

    // 读取深度图
    Mat d1 = imread(depth_path_1, cv::IMREAD_UNCHANGED); // 第一张图像的深度图
    Mat d2 = imread(depth_path_2, cv::IMREAD_UNCHANGED); // 第二张图像的深度图

    // 定义相机内参
    const double fx = 511.2450;
    const double fy = 509.9215;
    const double cx = 320.3848;
    const double cy = 232.5300;

    // 设置相机内参矩阵K
    Mat K = (Mat_<double>(3, 3) << fx, 0, cx,
                                   0, fy, cy,
                                   0, 0, 1);

    // 各个图像的3D点和2D点容器
    vector<Point3f> pts_3d_1, pts_3d_2;
    vector<Point2f> pts_2d_1, pts_2d_2;

    // 处理第一张图像的3D点生成
    for (DMatch m : matches) {
        ushort d_1 = d1.ptr<unsigned short>(int(keypoints_1[m.queryIdx].pt.y))[int(keypoints_1[m.queryIdx].pt.x)];
        if (d_1 == 0) continue; // 跳过无效深度
        float dd_1 = d_1 / 1000.0; // 将深度值归一化为米
        Point2d p1 = pixel2cam(keypoints_1[m.queryIdx].pt, K); // 获取归一化坐标
        pts_3d_1.push_back(Point3f(p1.x * dd_1, p1.y * dd_1, dd_1)); // 生成3D点
        pts_2d_1.push_back(keypoints_2[m.trainIdx].pt); // 存储对应的2D点
    }

    // 处理第二张图像的3D点生成
    for (DMatch m : matches) {
        ushort d_2 = d2.ptr<unsigned short>(int(keypoints_2[m.trainIdx].pt.y))[int(keypoints_2[m.trainIdx].pt.x)];
        if (d_2 == 0) continue; // 跳过无效深度
        float dd_2 = d_2 / 1000.0; // 将深度值归一化为米
        Point2d p2 = pixel2cam(keypoints_2[m.trainIdx].pt, K); // 获取归一化坐标
        pts_3d_2.push_back(Point3f(p2.x * dd_2, p2.y * dd_2, dd_2)); // 生成3D点
        pts_2d_2.push_back(keypoints_1[m.queryIdx].pt); // 存储对应的2D点
    }

    // 调用PnP求解第一张图像的相机位姿
    Mat r_1, t_1;
    solvePnP(pts_3d_1, pts_2d_1, K, Mat(), r_1, t_1, false); // 调用OpenCV 的 PnP 求解

    // 调用PnP求解第二张图像的相机位姿
    Mat r_2, t_2;
    solvePnP(pts_3d_2, pts_2d_2, K, Mat(), r_2, t_2, false); // 调用OpenCV 的 PnP 求解

    // 将旋转向量转换为旋转矩阵
    Mat R_1, R_2;
    cv::Rodrigues(r_1, R_1); // r为旋转向量，用Rodrigues转换为旋转矩阵
    cv::Rodrigues(r_2, R_2); // r为旋转向量，用Rodrigues转换为旋转矩阵

    // 进行优化
    VecVector3d pts_3d_1_eigen, pts_3d_2_eigen;
    VecVector2d pts_2d_1_eigen, pts_2d_2_eigen;

    // 第一张图片的优化
    for (size_t i = 0; i < pts_3d_1.size(); ++i) {
        pts_3d_1_eigen.push_back(Eigen::Vector3d(pts_3d_1[i].x, pts_3d_1[i].y, pts_3d_1[i].z));
        pts_2d_1_eigen.push_back(Eigen::Vector2d(pts_2d_1[i].x, pts_2d_1[i].y));
    }

    // cout << "calling bundle adjustment by Gauss-Newton" << endl;
    // Sophus::SE3d pose_gn_1;
    // Eigen::Matrix4d RT_gn_1 = bundleAdjustmentGaussNewton(pts_3d_1_eigen, pts_2d_1_eigen, K, pose_gn_1);

    // cout << "calling bundle adjustment by g2o" << endl;
    Sophus::SE3d pose_g2o_1;
    Eigen::Matrix4d RT_g2o_1 = bundleAdjustmentG2O(pts_3d_1_eigen, pts_2d_1_eigen, K, pose_g2o_1);

    // 第二张图片的优化
    for (size_t i = 0; i < pts_3d_2.size(); ++i) {
        pts_3d_2_eigen.push_back(Eigen::Vector3d(pts_3d_2[i].x, pts_3d_2[i].y, pts_3d_2[i].z));
        pts_2d_2_eigen.push_back(Eigen::Vector2d(pts_2d_2[i].x, pts_2d_2[i].y));
    }

    // cout << "calling bundle adjustment by Gauss-Newton" << endl;
    // Sophus::SE3d pose_gn_2;
    // Eigen::Matrix4d RT_gn_2 = bundleAdjustmentGaussNewton(pts_3d_2_eigen, pts_2d_2_eigen, K, pose_gn_2);

    // cout << "calling bundle adjustment by g2o" << endl;
    Sophus::SE3d pose_g2o_2;
    Eigen::Matrix4d RT_g2o_2 = bundleAdjustmentG2O(pts_3d_2_eigen, pts_2d_2_eigen, K, pose_g2o_2);

    Eigen::Matrix4d RT = computeRelativePose(RT_g2o_1, RT_g2o_2); // 计算相对位姿

    return std::to_string(RT(0, 3));
}