#include "precise_position.h"

// 读取输入路径对应的txt文件，保存图片名称
void readImagePaths(const string &txtFilePath, 
                    vector<string> &matchImageNames, 
                    vector<string> &mappedImageNames) {
    // 创建输入文件流
    ifstream inFile(txtFilePath);

    // 检查文件是否成功打开
    if (!inFile) {
        cerr << "Error opening file: " << txtFilePath << endl;
        return;
    }

    // 逐行读取文件内容
    string line;
    while (getline(inFile, line)) {
        istringstream stream(line);
        string matchImageName, mappedImageName;

        // 读取待匹配的图片名称（直到第一个空格）
        stream >> matchImageName; // 读取第一个部分

        // 读取第二个部分，直到第二个空格
        stream >> mappedImageName; // 读取第二个空格之间的内容

        // 将结果保存到对应的向量中
        matchImageNames.push_back(matchImageName);
        mappedImageNames.push_back(mappedImageName);
    }

    // 关闭文件流
    inFile.close();
}

// 检查文件是否存在的函数实现
bool fileExists(const string &path) {
    return filesystem::exists(path);
}

// 计算更小的误差
std::vector<double> compareAndSelectMin(const std::vector<double>& array1, const std::vector<double>& array2) {
    // 检查两个数组的大小是否相同
    if (array1.size() != array2.size()) {
        throw std::invalid_argument("两个数组的元素个数必须相同");
    }

    std::vector<double> minArray; // 用于存储更小的元素

    // 遍历两个数组，比较每个对应的元素
    for (size_t i = 0; i < array1.size(); ++i) {
        // 选择更小的元素
        minArray.push_back(std::min(std::abs(array1[i]), std::abs(array2[i])));
    }

    return minArray; // 返回新数组
}

// 定义处理图像和深度图的函数
void processImages(const std::string &txtFilePath,
                   const std::string &mapRgbPath, 
                   const std::string &matchRgbPath,
                   const std::string &mapDepthPath, 
                   const std::string &matchDepthPath) {
    // 读取图像路径
    std::vector<std::string> matchImageNames, mapImageNames;
    readImagePaths(txtFilePath, matchImageNames, mapImageNames);

    // 读取原有文件内容
    std::ifstream inputFile(txtFilePath);
    std::string line;
    std::vector<std::string> lines;

    // 读取文件内容到容器
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }
    inputFile.close();

    // 确保 lines 的大小与 matchImageNames 一致
    if (lines.size() < matchImageNames.size()) {
        std::cerr << "原文件行数少于图像匹配数，无法完整写入!" << std::endl;
        return;
    }

    std::vector<double> ePNP_distances; 
    std::vector<double> pixel_distances; 
    std::vector<double> true_distances;
    std::vector<double> R_t_distances;
    

    // 遍历匹配的图像
    for (size_t i = 0; i < matchImageNames.size(); i++) {
        // 读取图像
        std::string mapRgbImg = mapRgbPath + "/" + mapImageNames[i] + ".png";
        std::string matchRgbImg = matchRgbPath + "/" + matchImageNames[i] + ".png";
        std::string mapDepthImg = mapDepthPath + "/" + mapImageNames[i] + ".png";
        std::string matchDepthImg = matchDepthPath + "/" + matchImageNames[i] + ".png";

        // 检查文件是否存在
        bool allFilesExist = true; // 标记是否所有文件存在

        if (!fileExists(mapRgbImg)) {
            std::cerr << "Not found: " << mapRgbImg << std::endl;
            allFilesExist = false;
        }

        if (!fileExists(matchRgbImg)) {
            std::cerr << "Not found: " << matchRgbImg << std::endl;
            allFilesExist = false;
        }

        if (!fileExists(mapDepthImg)) {
            std::cerr << "Not found: " << mapDepthImg << std::endl;
            allFilesExist = false;
        }

        if (!fileExists(matchDepthImg)) {
            std::cerr << "Not found: " << matchDepthImg << std::endl;
            allFilesExist = false;
        }

        // 计算 ePNP 距离与像素距离
        if (allFilesExist) {
            ePNP_distances.push_back(std::stod(computeRT(mapRgbImg, matchRgbImg, mapDepthImg, matchDepthImg)));
            pixel_distances.push_back(calculateXCoordinateDifferences(mapRgbImg, matchRgbImg));
            // R_t_distances.push_back(estimatePose(mapRgbImg, matchRgbImg));
        }
    }

    // 比较并选择最小距离
    true_distances = compareAndSelectMin(ePNP_distances, pixel_distances);
    // true_distances = compareAndSelectMin(ePNP_distances, ePNP_distances);
    // true_distances = compareAndSelectMin(pixel_distances, pixel_distances);

    // 2D-2D 距离
    // for (int i = 0; i < R_t_distances.size(); i++) {
    //     if (R_t_distances[i] > -1) {
    //         std::cout << R_t_distances[i] << " ";
    //     }
    // }

    // std::cout << "----------------------------" << std::endl;
    double sum = 0.0;
    for (size_t i = 0; i < true_distances.size(); i++) {
            if (true_distances[i] * 100 > 0.5) { // 若误差小于 0.5，则忽略
            // std::cout << " " << true_distances[i] * 100;
            sum += std::abs(true_distances[i] * 100); // 计算绝对值误差
            }
    }

    double avg_error = sum / true_distances.size();
    // std::cout << "---------------------------------" << std::endl;
    // std::cout << "平均误差: " << avg_error << std::endl;
    // std::cout << "---------------------------------" << std::endl;
    // 更新对应行的内容
    for (size_t i = 0; i < true_distances.size(); i++) {
        if (i < lines.size()) { // 确保不会超出行数
            lines[i] += " " + std::to_string(true_distances[i] * 100); // 在每一行末尾添加 true_distance
        }
    }

    // 写回更新后的内容
    std::ofstream outputFile(txtFilePath, std::ios::trunc); // 清空文件并准备写入
    if (!outputFile) {
        std::cerr << "Error opening output file: " << txtFilePath << std::endl;
        return;
    }

    // 输出更新后的行
    for (const auto& updatedLine : lines) {
        outputFile << updatedLine << std::endl;
    }

    // 关闭输出文件
    outputFile.close();
}

// 得到最终的误差
void processFile(const std::string& filename) {
    std::ifstream inputFile(filename); // 打开文件
    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> processedLines; // 存储处理后的行

    while (std::getline(inputFile, line)) {
        std::istringstream stream(line);
        std::string part1, part2;
        double a = 0.0;
        double b = 0.0;
        int spaceCount = 0;
        std::string newLine;

        while (stream) {
            std::string part;
            stream >> part; // 读取部分内容

            // 根据空格数量处理变量
            if (spaceCount == 0) {
                part1 = part; // 第一部分
            } else if (spaceCount == 1) {
                part2 = part; // 第二部分
            } else if (spaceCount == 2) {
                try {
                    a = std::stod(part); // 读取数 a
                    newLine = part1 + " " + part2 + " " + std::to_string(a); // 修改 newLine
                } catch (const std::invalid_argument& e) {
                    std::cerr << "错误: '" << part << "' 不是有效的数字." << std::endl;
                    return; // 提前结束处理
                } catch (const std::out_of_range& e) {
                    std::cerr << "错误: 数字超出范围: '" << part << "'" << std::endl;
                    return; // 提前结束处理
                }
            } else {
                // 从当前部分到行尾，计算 b 的绝对值
                try {
                    double value = std::stod(part);
                    b += std::abs(value); // 计算 b 的绝对值
                    // std::cout << "b 的绝对值: " << b << std::endl; // 输出 b 的绝对值
                } catch (const std::invalid_argument& e) {
                    // std::cerr << "错误: '" << part << "' 不是有效的数字." << std::endl;
                } catch (const std::out_of_range& e) {
                    // std::cerr << "错误: 数字超出范围: '" << part << "'" << std::endl;
                }
            }

            spaceCount++;
        }

        // 进一步处理 b 变量，按需修改 newLine
        if (b < 0.5) {
            // 当 b < 0.5，保留原行
            processedLines.push_back(newLine);
        } else {
            // 比较 a 和 b，保留较小的
            double minVal = std::min(a, b);
            newLine = part1 + " " + part2 + " " + std::to_string(minVal);
            processedLines.push_back(newLine); // 存储处理后的新行
        }
    }

    inputFile.close(); // 关闭文件

    // 将处理后的内容写回文件
    std::ofstream outputFile(filename); // 打开文件进行写入
    if (!outputFile.is_open()) {
        std::cerr << "无法打开文件进行写入: " << filename << std::endl;
        return;
    }

    for (const auto& newLine : processedLines) {
        outputFile << newLine << std::endl; // 写入处理后的行
    }

    outputFile.close(); // 关闭输出文件
}

// 求平均误差
double calculateAverageFromTxt(const std::string& filePath) {
    std::ifstream inFile(filePath);
    
    // 检查文件是否成功打开
    if (!inFile.is_open()) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return 0.0;  // 返回 0 表示读取失败
    }

    std::string line;
    std::vector<double> numbers;  // 用于存储提取的数字
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string word;
        int spaceCount = 0;
        double number = 0.0;

        // 遍历每个词
        while (iss >> word) {
            spaceCount++;
            // 如果是第二个空格后，我们转换为 double 并保存
            if (spaceCount == 3) {
                try {
                    number = std::stod(word);  // 转换为 double
                    numbers.push_back(number);  // 存储数字
                } catch (const std::invalid_argument& e) {
                    std::cerr << "无效的数字: " << word << " 在行: " << line << std::endl;
                }
            }
        }
    }

    inFile.close();

    // 计算平均值
    double sum = 0.0;
    for (double num : numbers) {
        // std::cout << num << " ";
        sum += num;
    }
    
    double front_avg_error = numbers.empty() ? 0.0 : sum / numbers.size();

    // 返回平均值
    return front_avg_error;
}