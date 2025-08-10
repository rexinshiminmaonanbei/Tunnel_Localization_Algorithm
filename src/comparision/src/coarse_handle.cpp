#include "coarse_handle.h"

// 打印进度条的函数
void printProgressBar(double progress) {
    const int barWidth = 70; // 进度条宽度
    std::cout << "[";
    int pos = static_cast<int>(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::setprecision(2) << std::fixed << (progress * 100.0) << " %\r";
    std::cout.flush();
}

// 从文件中读取图片名称和x轴坐标和描述符，并保存到结构体数组中
std::vector<ImageData> readImageDataFromFile(const std::string& pose_path, const std::string& wall_image_path, const std::string& front_image_path) {
    std::ifstream inFile(pose_path);
    if (!inFile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return {};
    }

    std::vector<ImageData> imageDataArray;
    std::string line;
    size_t totalLines = 0;
    // 先统计文件中的行数
    while (std::getline(inFile, line)) {
        totalLines++;
    }

    inFile.clear(); // 清除 EOF 状态
    inFile.seekg(0); // 重置文件流指针到开始位置

    std::cout << "开始读取数据..." << std::endl;

    // 读取数据
    size_t lineCount = 0;

    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        ImageData imageData;

        // 读取图片名称（第一个空格前的数据）
        iss >> imageData.imageName;

        // 读取x轴坐标（第一个空格到第二个空格之间的数据）
        std::string coordinateStr;
        iss >> coordinateStr;
        imageData.x_value = std::stod(coordinateStr);

        // 将结构体添加到数组中
        imageDataArray.push_back(imageData);
        lineCount++;

        // 更新并打印进度条
        printProgressBar(static_cast<double>(lineCount) / totalLines);
    }

    inFile.close();
    std::cout << std::endl << "完成读取文件." << std::endl;

    // 遍历侧视文件夹中的所有图片文件
    std::cout << "开始处理侧视图片描述符..." << std::endl;
    size_t wallImagesProcessed = 0;
    size_t wallTotalImages = std::distance(fs::directory_iterator(wall_image_path), fs::directory_iterator{});
    
    for (const auto& entry : fs::directory_iterator(wall_image_path)) {
        if (entry.is_regular_file()) { // 仅处理文件
            std::string imagePath = entry.path().string();
            std::string imageName = entry.path().filename().stem().string(); // 提取文件名

            // 查找对应的结构体并加载图片
            for (auto& imgData : imageDataArray) {
                if (imgData.imageName == imageName) { // 比较图片名称
                    // 计算描述符
                    auto result = extractMultiScaleDescriptors(imagePath);
                    imgData.wall_descriptor1 = std::get<0>(result);
                    imgData.wall_descriptor2 = std::get<1>(result);
                    imgData.wall_descriptor4 = std::get<2>(result);
                    wallImagesProcessed++;
                    printProgressBar(static_cast<double>(wallImagesProcessed) / wallTotalImages);
                    break; // 找到对应图像后，退出循环
                }
            }
        }
    }
    std::cout << std::endl << "完成处理侧视图片描述符." << std::endl;

    // 遍历前视文件夹中的所有图片文件 
    // 处理前视图片描述符
    std::cout << "开始处理前视图片描述符..." << std::endl;
    size_t frontImagesProcessed = 0;
    size_t frontTotalImages = std::distance(fs::directory_iterator(front_image_path), fs::directory_iterator{});
    
    for (const auto& entry : fs::directory_iterator(front_image_path)) {
        if (entry.is_regular_file()) { // 仅处理文件
            std::string imagePath = entry.path().string();
            std::string imageName = entry.path().filename().stem().string(); // 提取文件名

            // 查找对应的结构体并加载图片
            for (auto& imgData : imageDataArray) {
                if (imgData.imageName == imageName) { // 比较图片名称
                    // 计算描述符
                    auto result = extractMultiScaleDescriptors(imagePath);
                    imgData.front_descriptor1 = std::get<0>(result);
                    imgData.front_descriptor2 = std::get<1>(result);
                    imgData.front_descriptor4 = std::get<2>(result);
                    frontImagesProcessed++;
                    printProgressBar(static_cast<double>(frontImagesProcessed) / frontTotalImages);
                    break; // 找到对应图像后，退出循环
                }
            }
        }
    }
    std::cout << std::endl << "完成处理前视图片描述符." << std::endl;

    return imageDataArray;
}

// 根据图片名称查找x轴坐标的索引
int findClosestXCoordinateIndex(const double& queryXCoordinate, const std::vector<ImageData>& MapDataArray) {
    double minDifference = std::numeric_limits<double>::max();
    int closestIndex = -1;

    // 查找所有元素中绝对值最接近的 x 轴坐标
    for (size_t i = 0; i < MapDataArray.size(); ++i) {
        double difference = std::abs(MapDataArray[i].x_value - queryXCoordinate);
        if (difference < minDifference) {
            minDifference = difference;
            closestIndex = i;
        }
    }

    // 检查是否找到最近的坐标索引
    if (closestIndex == -1) {
        std::cerr << "未找到与查询坐标 " << std::endl;
    }
    return closestIndex;
}