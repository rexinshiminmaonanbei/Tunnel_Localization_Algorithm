#include "make_data.h"

// 进度条
void updateProgressBar(size_t processed, size_t total, std::chrono::steady_clock::time_point startTime) {
    if (total == 0) return;

    double progress = static_cast<double>(processed) / total * 100;
    auto elapsedTime = std::chrono::steady_clock::now() - startTime;
    double elapsedSeconds = std::chrono::duration<double>(elapsedTime).count();
    double estimatedTotalTime = elapsedSeconds / (processed + 1) * total; // 估算总时间
    double remainingTime = estimatedTotalTime - elapsedSeconds;

    // 将剩余时间转换为分钟和秒
    int remainingMinutes = static_cast<int>(remainingTime) / 60;
    int remainingSeconds = static_cast<int>(remainingTime) % 60;

    // 输出进度条
    std::cout << "\r处理进度: " << std::fixed << std::setprecision(2) 
              << progress << "% (" << processed << "/" << total << "), 剩余时间: "
              << remainingMinutes << "分 " << remainingSeconds << "秒";
    std::cout.flush();
}

// 读取rosbag文件并保存图像到文件夹中（RGB）
void saveImagesFromRosbag_RGB(const std::string& bagFilePath, const std::string& topicName, const std::string& saveDirectory) {
    std::cout << "正在读取rosbag文件。 " << std::endl;
    rosbag::Bag bag;
    
    try {
        bag.open(bagFilePath, rosbag::bagmode::Read);
    } catch (const rosbag::BagIOException& e) {
        std::cerr << "无法打开bag文件: " << e.what() << std::endl;
        return;
    }

    rosbag::View view(bag, rosbag::TopicQuery(topicName));
    size_t totalMessages = view.size();
    size_t processedMessages = 0;

    auto startTime = std::chrono::steady_clock::now();

    std::cout << "开始提取图像..." << std::endl;

    for (const rosbag::MessageInstance& msg : view) {
        sensor_msgs::Image::ConstPtr imgMsg = msg.instantiate<sensor_msgs::Image>();
        if (imgMsg) {
            try {
                cv_bridge::CvImagePtr cvPtr;

                if (imgMsg->encoding == sensor_msgs::image_encodings::BGR8 || 
                    imgMsg->encoding == sensor_msgs::image_encodings::RGB8) {
                    cvPtr = cv_bridge::toCvCopy(imgMsg, sensor_msgs::image_encodings::BGR8);
                } else if (imgMsg->encoding == sensor_msgs::image_encodings::MONO16) {
                    cvPtr = cv_bridge::toCvCopy(imgMsg, sensor_msgs::image_encodings::MONO8);
                } else {
                    std::cerr << "图像格式不支持: " << imgMsg->encoding << std::endl;
                    continue; 
                }

                cv::Mat image = cvPtr->image;
                if (imgMsg->encoding == sensor_msgs::image_encodings::BGR8) {
                    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
                }

                double timestamp = imgMsg->header.stamp.toSec();
                std::stringstream ss;
                ss << std::fixed << std::setfill('0') << std::setw(10) << static_cast<int>(timestamp) 
                   << "." << std::setfill('0') << std::setw(9) << static_cast<int>((timestamp - static_cast<int>(timestamp)) * 1e9);

                std::string filename = saveDirectory + "/" + ss.str() + ".png";
                
                cv::imwrite(filename, image);
                
                processedMessages++;
                updateProgressBar(processedMessages, totalMessages, startTime);
            } catch (cv_bridge::Exception& e) {
                std::cerr << "图像转换失败: " << e.what() << std::endl;
            }
        }
    }

    std::cout << std::endl << "提取完成!" << std::endl;
    bag.close();
}

// 读取rosbag文件并保存图像到文件夹中（深度图）
void saveImagesFromRosbag_Depth(const std::string& bagFilePath, const std::string& topicName, const std::string& saveDirectory) {
    std::cout << "正在读取rosbag文件。" << std::endl;
    rosbag::Bag bag;

    try {
        bag.open(bagFilePath, rosbag::bagmode::Read);
    } catch (const rosbag::BagIOException& e) {
        std::cerr << "无法打开bag文件: " << e.what() << std::endl;
        return;
    }

    rosbag::View view(bag, rosbag::TopicQuery(topicName));
    size_t totalMessages = view.size();
    size_t processedMessages = 0;

    auto startTime = std::chrono::steady_clock::now();

    std::cout << "开始提取深度图像..." << std::endl;

    for (const rosbag::MessageInstance& msg : view) {
        sensor_msgs::Image::ConstPtr imgMsg = msg.instantiate<sensor_msgs::Image>();
        if (imgMsg) {
            try {
                cv_bridge::CvImagePtr cvPtr = cv_bridge::toCvCopy(imgMsg, sensor_msgs::image_encodings::TYPE_16UC1);
                cv::Mat depthImage = cvPtr->image;

                double timestamp = imgMsg->header.stamp.toSec();
                std::stringstream ss;
                ss << std::fixed << std::setfill('0') << std::setw(10) << static_cast<int>(timestamp)
                   << "." << std::setfill('0') << std::setw(9) << static_cast<int>((timestamp - static_cast<int>(timestamp)) * 1e9);

                std::string filename = saveDirectory + "/" + ss.str() + ".png"; 

                cv::imwrite(filename, depthImage);

                processedMessages++;
                updateProgressBar(processedMessages, totalMessages, startTime);
            } catch (const cv_bridge::Exception& e) {
                std::cerr << "图像转换失败: " << e.what() << std::endl;
            }
        }
    }

    std::cout << std::endl << "提取完成!" << std::endl;
    bag.close();
}

// 检查输入目录中的所有图像是否可读取
void checkImagesReadable(const std::string& inputDirectory) {
    std::cout << "正在检查图片可读性..." << std::endl;

    size_t totalFiles = 0;
    size_t processedFiles = 0;

    // 统计文件夹中的图片文件总数
    for (const auto& entry : fs::directory_iterator(inputDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            totalFiles++;
        }
    }

    // 开始检测过程
    auto startTime = std::chrono::steady_clock::now();

    for (const auto& entry : fs::directory_iterator(inputDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            // 检查图片文件是否可读
            if (fs::exists(entry.path())) {
                // 尝试打开文件以查看是否可读
                std::ifstream file(entry.path(), std::ios::in);
                if (!file) {
                    std::cerr << "文件不可读取: " << entry.path() << std::endl;
                }
                file.close();
            }
            processedFiles++;
            updateProgressBar(processedFiles, totalFiles, startTime);
        }
    }

    std::cout << std::endl << "检查完成，找到 " << totalFiles << " 张图片文件。" << std::endl;
}

// 检查输入目录中的所有图像是否可读取
void findAndCopyClosestImages(const std::string& folderA, const std::string& folderB, const std::string& folderC) {
    std::vector<std::string> filenamesA; // 用于保存 folder A 中的文件名
    std::vector<double> valuesA; // 用于存储 folder A 的对应数值
    std::vector<std::string> filenamesB; // 用于保存 folder B 中的文件名
    std::vector<double> valuesB; // 用于存储 folder B 的对应数值
    std::map<std::string, int> copyCount; // 用于记录复制次数

    // 读取文件夹A中的文件名并转换为double类型，保留字符串格式
    for (const auto& entry : fs::directory_iterator(folderA)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            std::string filename = entry.path().stem().string(); // 获取不带扩展名的文件名
            filenamesA.push_back(filename); // 存储字符串格式文件名
            try {
                double value = std::stod(filename); // 转换为double
                valuesA.push_back(value);
            } catch (const std::invalid_argument&) {
                std::cerr << "无法转换文件名为double: " << filename << std::endl;
            }
        }
    }

    // 读取文件夹B中的文件名并转换为double类型，保留字符串格式
    for (const auto& entry : fs::directory_iterator(folderB)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            std::string filename = entry.path().stem().string(); // 获取不带扩展名的文件名
            filenamesB.push_back(filename); // 存储字符串格式文件名
            try {
                double value = std::stod(filename); // 转换为double
                valuesB.push_back(value);
            } catch (const std::invalid_argument&) {
                std::cerr << "无法转换文件名为double: " << filename << std::endl;
            }
        }
    }

    // 开始统计进度
    size_t processedCount = 0;
    auto startTime = std::chrono::steady_clock::now(); // 开始计时

    // 遍历 A 中的每个文件名
    for (size_t i = 0; i < filenamesA.size(); ++i) {
        std::string searchStr = filenamesA[i]; // 文件名用于匹配

        size_t minIndex = -1;
        double minDiff = std::numeric_limits<double>::max();

        // 遍历 B 中的每个值并计算差值
        for (size_t j = 0; j < valuesB.size(); ++j) {
            double diff = std::abs(valuesA[i] - valuesB[j]);
            if (diff < minDiff) {
                minDiff = diff;
                minIndex = j; // 记录在 B 中的索引
            }
        }

        // 复制到文件夹 C 使用完整文件名
        std::string srcFile = folderB + "/" + filenamesB[minIndex] + ".png"; // 使用完整文件名
        std::string destFile = folderC + "/" + filenamesB[minIndex] + ".png"; // 使用完整文件名

        // 检查文件是否已存在
        if (fs::exists(destFile)) {
            copyCount[destFile]++;
            // std::cout << "\n重复文件: " << destFile << std::endl;

            // 处理重复文件：找到与其对应的两张图片
            double diffA1 = std::abs(valuesA[i] - valuesB[minIndex]);
            double diffA2 = minDiff; // 先前找到的最小差值
            double largerDiff = std::max(diffA1, diffA2);
            double smallerDiff = std::min(diffA1, diffA2);

            size_t newMinIndex = -1;
            double secondMinDiff = std::numeric_limits<double>::max();

            // 在 valuesB 中找到第二小的差值
            for (size_t k = 0; k < valuesB.size(); ++k) {
                if (k != minIndex) { // 排除已经存在的文件索引
                    double newDiff = std::abs(valuesA[i] - valuesB[k]);
                    if (newDiff < secondMinDiff) {
                        secondMinDiff = newDiff;
                        newMinIndex = k; // 记录新找到的第二小差值的索引
                    }
                }
            }

            // 使用第二小差值的索引复制文件
            std::string newSrcFile = folderB + "/" + filenamesB[newMinIndex] + ".png";
            std::string newDestFile = folderC + "/" + filenamesB[newMinIndex] + ".png";

            // 尝试复制和处理异常
            try {
                fs::copy(newSrcFile, newDestFile);
                // std::cout << "复制第二小差值文件: " << newDestFile << std::endl;
            } catch (const fs::filesystem_error& e) {
                // std::cerr << "复制文件失败: " << newSrcFile << " 到 " << newDestFile << "，错误: " << e.what() << std::endl;
            }
        } else {
            // 复制文件
            try {
                fs::copy(srcFile, destFile);
                copyCount[destFile] = 1; // 记录复制次数
            } catch (const fs::filesystem_error& e) {
                // std::cerr << "复制文件失败: " << srcFile << " 到 " << destFile << "，错误: " << e.what() << std::endl;
            }
        }

        processedCount++;
        updateProgressBar(processedCount, filenamesA.size(), startTime); // 更新进度条
    }

    std::cout << std::endl << "已复制所有匹配的图片到文件夹 C。" << std::endl;
}

// 在txt时间戳部分加小数点
void addDecimalPointToFile(const std::string& filePath) {
    // 临时存储每行的内容
    std::ifstream inputFile(filePath);
    std::string tempFilePath = filePath + ".tmp"; // 创建临时文件
    std::ofstream outputFile(tempFilePath);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "无法打开文件。" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.length() >= 11) { // 确保行长足够
            line.insert(10, "."); // 在第10个与第11个字符之间插入小数点
        }
        outputFile << line << std::endl; // 将修改后的行写入输出文件
    }

    inputFile.close();
    outputFile.close();

    // 替换原文件
    if (std::remove(filePath.c_str()) != 0) {
        std::cerr << "无法删除原文件。" << std::endl;
    }
    if (std::rename(tempFilePath.c_str(), filePath.c_str()) != 0) {
        std::cerr << "无法重命名临时文件。" << std::endl;
    }

    std::cout << "已成功在每行的第10个与第11个字符之间插入小数点。" << std::endl;
}

// txt与图片对齐
void saveImageNamesToTxt(const std::string& folderPath, const std::string& txtFilePath) {
    // 创建一个存储图片名称的向量
    std::vector<std::string> imageNames;

    // 遍历文件夹中的所有文件
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            // 获取文件名并去掉后缀
            std::string fileName = entry.path().stem().string();
            imageNames.push_back(fileName);
        }
    }

    // 对文件名进行排序
    std::sort(imageNames.begin(), imageNames.end());

    // 将文件名写入txt文件
    std::ofstream outFile(txtFilePath);
    if (outFile.is_open()) {
        for (const auto& name : imageNames) {
            outFile << name << std::endl;
        }
        outFile.close();
    } else {
        std::cerr << "无法打开输出文件: " << txtFilePath << std::endl;
    }
}

// txt根据图片名称赋值
void processTxtFiles(const std::string& txtAPath, const std::string& txtBPath) {
    std::ifstream txtAFile(txtAPath);
    std::ifstream txtBFile(txtBPath);
    
    if (!txtAFile.is_open() || !txtBFile.is_open()) {
        std::cerr << "无法打开文件!" << std::endl;
        return;
    }

    std::vector<std::string> txtAArray;
    std::vector<std::string> txtBArray;
    std::vector<std::string> txtBAllLines;

    std::string line;

    // 读取txtA的每行前21个字符
    size_t totalALines = 0;
    while (std::getline(txtAFile, line)) {
        totalALines++;
        if (line.length() >= 21) {
            txtAArray.push_back(line.substr(0, 21));
        }
    }
    std::cout << std::endl; // 换行

    // 读取txtB的每行
    size_t totalBLines = 0;
    while (std::getline(txtBFile, line)) {
        totalBLines++;
        if (line.length() >= 20) {
            txtBAllLines.push_back(line);
            txtBArray.push_back(line.substr(0, 20));
        }
    }
    std::cout << std::endl; // 换行

    txtAFile.close();
    txtBFile.close();

    // 更新进度条初始化
    size_t processedCount = 0;

    // 寻找每个txtA元素对应的txtB中最小差值的行，并进行复制操作
    for (int i = 0; i < txtAArray.size(); ++i) {
        double valueA = std::stod(txtAArray[i].substr(0, 17));
        double minDifference = std::numeric_limits<double>::max();
        int minBIndex = -1;

        // 找到差值最小的行
        for (int j = 0; j < txtBArray.size(); ++j) {
            double valueB = std::stod(txtBArray[j].substr(0, 17));
            double difference = std::abs(valueA - valueB);

            if (difference < minDifference) {
                minDifference = difference;
                minBIndex = j;
            }
        }

        // 如果找到了对应的行，则进行内容复制
        if (minBIndex != -1) {
            std::string lineToCopy; // 获取txtA中对应行的内容
            std::ifstream txtAFileAgain(txtAPath);
            for (int k = 0; std::getline(txtAFileAgain, line) && k <= i; ++k) {
                if (k == i) {
                    lineToCopy = line.substr(line.find(' ') + 1); // 复制第一个空格之后的内容
                }
            }
            txtBAllLines[minBIndex] += lineToCopy; // 将内容附加到txtB的对应行
            txtAFileAgain.close();
        }

        // 更新进度条
        processedCount++;
        updateProgressBar(processedCount, txtAArray.size(), std::chrono::steady_clock::now());
    }

    // 写回更新后的内容到txtB
    std::ofstream txtBOutFile(txtBPath);
    for (const auto& outputLine : txtBAllLines) {
        txtBOutFile << outputLine << std::endl;
    }

    std::cout << std::endl << "处理完成!" << std::endl;
}

// txt做插值之前加入空格
void modifyTxtFile(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    std::string tempFilePath = filePath + ".tmp"; // 创建临时文件
    std::ofstream outputFile(tempFilePath);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "无法打开文件。" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.length() > 20) { // 如果行的长度超过20
            line.insert(20, " "); // 在第20个和第21个字符之间插入空格
        }
        outputFile << line << std::endl; // 将修改后的行写入临时文件
    }

    inputFile.close();
    outputFile.close();

    // 替换原文件
    if (std::remove(filePath.c_str()) != 0) {
        std::cerr << "无法删除原文件。" << std::endl;
    }
    if (std::rename(tempFilePath.c_str(), filePath.c_str()) != 0) {
        std::cerr << "无法重命名临时文件。" << std::endl;
    }

    std::cout << "文件修改完成，已插入空格。" << std::endl;
}

// txt文件插值
void processTxtFile(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    std::string tempFilePath = filePath + ".tmp"; // 创建临时文件
    std::ofstream outputFile(tempFilePath);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "无法打开文件。" << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> lines;

    // 读取所有行到一个向量中
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }

    // 遍历每一行，进行相应的处理
    for (size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].length() > 20) {
            // 如果当前行长度大于20，不做修改
            outputFile << lines[i] << std::endl;
        } else {
            // 当前行小于等于20，计算上下行的平均值
            if (i > 0 && i < lines.size() - 1) { // 确保有上下行
                std::vector<double> aboveValues, belowValues;
                
                // 处理上一行
                std::stringstream ssAbove(lines[i - 1]);
                std::string token;
                // 跳过第一个元素
                ssAbove >> token;
                while (ssAbove >> token) {
                    aboveValues.push_back(std::stod(token));
                }

                // 处理下一行
                std::stringstream ssBelow(lines[i + 1]);
                // 跳过第一个元素
                ssBelow >> token;
                while (ssBelow >> token) {
                    belowValues.push_back(std::stod(token));
                }

                // 计算平均值
                std::stringstream resultLine;
                resultLine << lines[i]; // 保留当前行的第一个元素
                resultLine << " "; // 加个空格

                for (size_t j = 0; j < aboveValues.size() && j < belowValues.size(); ++j) {
                    double average = (aboveValues[j] + belowValues[j]) / 2.0;
                    resultLine << std::fixed << std::setprecision(6) << average << " "; // 保留6位小数
                }

                outputFile << resultLine.str() << std::endl; // 写入新内容
            } else {
                // 如果没有上下行就不处理，保持原状
                outputFile << lines[i] << std::endl;
            }
        }
    }

    inputFile.close();
    outputFile.close();

    // 替换原文件为修改后的文件
    if (std::remove(filePath.c_str()) != 0) {
        std::cerr << "无法删除原文件。" << std::endl;
    }
    if (std::rename(tempFilePath.c_str(), filePath.c_str()) != 0) {
        std::cerr << "无法重命名临时文件。" << std::endl;
    }

    std::cout << "文件处理完成，已更新内容。" << std::endl;
}

// 统一图片名称（RGB）
void matchImagesAndRename_RGB(const std::string& folderA, const std::string& folderB, const std::string& folderC) {
    std::vector<std::string> filesA;
    std::vector<std::string> filesB;

    // 从文件夹A读取文件名
    for (const auto& entry : fs::directory_iterator(folderA)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            filesA.push_back(entry.path().filename().string());
        }
    }

    // 从文件夹B读取文件名
    for (const auto& entry : fs::directory_iterator(folderB)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            filesB.push_back(entry.path().filename().string());
        }
    }

    // 处理文件名，提取前17位数字并进行匹配
    auto startTime = std::chrono::steady_clock::now();
    size_t totalFilesA = filesA.size();
    size_t processedCount = 0;

    for (const auto& fileA : filesA) {
        // 提取前17位并转换为double
        std::string numStrA = fileA.substr(0, 17);
        double valueA = std::stod(numStrA); 

        double minDifference = std::numeric_limits<double>::max();
        std::string bestMatchB;

        for (const auto& fileB : filesB) {
            // 提取前17位并转换为double
            std::string numStrB = fileB.substr(0, 17);
            double valueB = std::stod(numStrB);

            // 计算绝对差值
            double difference = std::abs(valueA - valueB);
            
            // 找到差值最小的元素
            if (difference < minDifference) {
                minDifference = difference;
                bestMatchB = fileB; // 更新最佳匹配
            }
        }

        // 找到了最佳匹配，将文件夹B中的对应图片名称复制到文件夹C，并重命名
        if (!bestMatchB.empty()) {
            std::string oldFilePathB = folderB + "/" + bestMatchB; // 原文件路径
            
            // 将文件夹B中的文件复制到文件夹C
            std::string copyFilePathC = folderC + "/" + bestMatchB; // 复制到文件夹C的路径
            try {
                fs::copy(oldFilePathB, copyFilePathC); // 复制文件
                // std::cout << "复制文件 " << oldFilePathB << " 到 " << copyFilePathC << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "复制文件失败: " << e.what() << std::endl;
                continue; // 错误处理后继续
            }

            // 在文件夹C中重命名为文件夹A中对应的文件名
            std::string newFilePathC = folderC + "/" + fileA; // 新文件名路径
            try {
                fs::rename(copyFilePathC, newFilePathC); // 重命名文件
                // std::cout << "将 " << bestMatchB << " 重命名为 " << fileA << " 位于文件夹C" << std::endl; // 打印重命名信息
            } catch (const fs::filesystem_error& e) {
                std::cerr << "重命名文件失败: " << e.what() << std::endl;
            }
        }

        // 更新进度条
        processedCount++;
        updateProgressBar(processedCount, totalFilesA, startTime);
    }
    
    std::cout << std::endl << "处理完成！" << std::endl;
}

// 统一图片名称（深度）
void matchImagesAndRename_depth(const std::string& folderA, const std::string& folderB, const std::string& folderC) {
    std::vector<std::string> filesA;
    std::vector<std::string> filesB;

    // 从文件夹A读取文件名
    for (const auto& entry : fs::directory_iterator(folderA)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            filesA.push_back(entry.path().filename().string());
        }
    }

    // 从文件夹B读取文件名
    for (const auto& entry : fs::directory_iterator(folderB)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            filesB.push_back(entry.path().filename().string());
        }
    }

    // 处理文件名，提取前17位数字并进行匹配
    auto startTime = std::chrono::steady_clock::now();
    size_t totalFilesA = filesA.size();
    size_t processedCount = 0;

    for (const auto& fileA : filesA) {
        // 提取前17位并转换为double
        std::string numStrA = fileA.substr(0, 17);
        double valueA = std::stod(numStrA); 

        double minDifference = std::numeric_limits<double>::max();
        std::string bestMatchB;

        for (const auto& fileB : filesB) {
            // 提取前17位并转换为double
            std::string numStrB = fileB.substr(0, 17);
            double valueB = std::stod(numStrB);

            // 计算绝对差值
            double difference = std::abs(valueA - valueB);
            
            // 找到差值最小的元素
            if (difference < minDifference) {
                minDifference = difference;
                bestMatchB = fileB; // 更新最佳匹配
            }
        }

        // 找到了最佳匹配，进行复制和重命名
        if (!bestMatchB.empty()) {
            std::string oldFilePathB = folderB + "/" + bestMatchB; // 原文件路径
            
            // 将文件夹A中的对应文件复制到文件夹C
            std::string copyFilePathC = folderC + "/" + fileA; // 复制到文件夹C的路径
            try {
                fs::copy(folderA + "/" + fileA, copyFilePathC); // 从文件夹A复制文件
                // std::cout << "复制文件 " << folderA + "/" + fileA << " 到 " << copyFilePathC << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "复制文件失败: " << e.what() << std::endl;
                continue; // 错误处理后继续
            }

            // 在文件夹C中将文件重命名为对应的文件夹B中的文件名
            try {
                fs::rename(copyFilePathC, folderC + "/" + bestMatchB); // 重命名文件
                // std::cout << "将 " << copyFilePathC << " 重命名为 " << bestMatchB << std::endl; // 打印重命名信息
            } catch (const fs::filesystem_error& e) {
                std::cerr << "重命名文件失败: " << e.what() << std::endl;
            }
        }

        // 更新进度条
        processedCount++;
        updateProgressBar(processedCount, totalFilesA, startTime);
    }
    
    std::cout << std::endl << "处理完成！" << std::endl;
}

// 制作地图和总的查询文件夹（RGB）
void copyImages(const std::string& folderA, const std::string& folderB, const std::string& folderC) {
    std::vector<fs::path> imagePaths;

    // 从源文件夹读取所有图片文件路径
    for (const auto& entry : fs::directory_iterator(folderA)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") { // 根据需要修改扩展名
            imagePaths.push_back(entry.path());
        }
    }

    // 按文件名排序
    std::sort(imagePaths.begin(), imagePaths.end());

    // 检查源文件夹中图片数量是否足够
    size_t totalImages = imagePaths.size();
    if (totalImages == 0) {
        std::cerr << "源文件夹中没有可复制的图片文件。" << std::endl;
        return;
    }

    // 确保目标文件夹B和C存在
    if (!fs::exists(folderB)) {
        fs::create_directories(folderB); // 创建目标文件夹B（如果不存在）
    }
    if (!fs::exists(folderC)) {
        fs::create_directories(folderC); // 创建目标文件夹C（如果不存在）
    }

    // 复制前31479张图片到文件夹B
    size_t numFilesToCopyToB = std::min<size_t>(31479, totalImages);
    auto startTime = std::chrono::steady_clock::now();

    for (size_t i = 0; i < numFilesToCopyToB; ++i) {
        try {
            fs::copy(imagePaths[i], folderB + "/" + imagePaths[i].filename().string(), fs::copy_options::overwrite_existing);
            // std::cout << "已复制: " << imagePaths[i] << " 到 " << folderB << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "复制文件到文件夹B失败: " << e.what() << std::endl;
        }
        updateProgressBar(i + 1, numFilesToCopyToB, startTime);
    }

    // 复制其余图片到文件夹C
    startTime = std::chrono::steady_clock::now();
    for (size_t i = numFilesToCopyToB; i < totalImages; ++i) {
        try {
            fs::copy(imagePaths[i], folderC + "/" + imagePaths[i].filename().string(), fs::copy_options::overwrite_existing);
            // std::cout << "已复制: " << imagePaths[i] << " 到 " << folderC << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "复制文件到文件夹C失败: " << e.what() << std::endl;
        }
        updateProgressBar(i + 1 - numFilesToCopyToB, totalImages - numFilesToCopyToB, startTime);
    }

    std::cout << "复制完成！" << std::endl;
}

// 比较图片名称是否一致
void compareImageNames(const std::string& folderA, const std::string& folderB) {
    std::set<std::string> namesA; // 存储文件夹A的文件名
    std::set<std::string> namesB; // 存储文件夹B的文件名

    // 从文件夹A读取文件名
    for (const auto& entry : fs::directory_iterator(folderA)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") { // 根据需要修改扩展名
            namesA.insert(entry.path().stem().string()); // 提取不带扩展名的文件名
        }
    }

    // 从文件夹B读取文件名
    for (const auto& entry : fs::directory_iterator(folderB)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") { // 根据需要修改扩展名
            namesB.insert(entry.path().stem().string()); // 提取不带扩展名的文件名
        }
    }

    // 找出在文件夹A中但不在文件夹B中的文件名
    std::cout << "在文件夹A中但不在文件夹B中的图片文件名:" << std::endl;
    for (const auto& name : namesA) {
        if (namesB.find(name) == namesB.end()) { // 如果名字不在B中
            std::cout << name << std::endl;
        }
    }

    std::cout << "比较完成！" << std::endl;
}

// 制作查询文件夹
void copyEveryNthImage(const std::string& sourceFolder, const std::string& destFolder, size_t n) {
    std::vector<fs::path> imagePaths;

    // 从源文件夹读取所有图片文件路径
    for (const auto& entry : fs::directory_iterator(sourceFolder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") { // 根据需要修改扩展名
            imagePaths.push_back(entry.path());
        }
    }

    // 检查源文件夹中图片数量
    size_t totalImages = imagePaths.size();
    if (totalImages == 0) {
        std::cerr << "源文件夹中没有可复制的图片文件。" << std::endl;
        return;
    }

    // 确保目标文件夹存在
    if (!fs::exists(destFolder)) {
        fs::create_directories(destFolder); // 创建目标文件夹（如果不存在）
    }

    // 每隔n张图片复制一张到目标文件夹
    for (size_t i = 0; i < totalImages; ++i) {
        if (i % n == 0) { // 每隔n（这里是300）进行复制
            try {
                fs::copy(imagePaths[i], destFolder + "/" + imagePaths[i].filename().string(), fs::copy_options::overwrite_existing);
                // std::cout << "已复制: " << imagePaths[i].filename() << " 到 " << destFolder << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "复制文件失败: " << e.what() << std::endl;
            }
        }
    }

    std::cout << "完成复制! 共复制 " << (totalImages / n) + (totalImages % n > 0 ? 1 : 0) << " 张图片." << std::endl;
}

// 制作查询txt
void extractImageNamesToTxt(const std::string& imageFolder, const std::string& txtPath, const std::string& outputTxtPath) {
    std::vector<std::string> imageNames;

    // 从图片文件夹提取文件名
    for (const auto& entry : fs::directory_iterator(imageFolder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            std::string fileName = entry.path().stem().string(); // 获取不带扩展名的文件名
            imageNames.push_back(fileName);
        }
    }

    // 按名称排序
    std::sort(imageNames.begin(), imageNames.end());

    // 准备输出文件
    std::ofstream outputFile(outputTxtPath);
    if (!outputFile.is_open()) {
        std::cerr << "无法打开输出文件: " << outputTxtPath << std::endl;
        return;
    }

    // 打开输入的txt文件
    std::ifstream inputFile(txtPath);
    if (!inputFile.is_open()) {
        std::cerr << "无法打开输入文件: " << txtPath << std::endl;
        return;
    }

    std::string line;
    // 遍历每个提取的图片名称
    for (const auto& imageName : imageNames) {
        // 重置输入文件流到开头
        inputFile.clear();
        inputFile.seekg(0, std::ios::beg);

        while (std::getline(inputFile, line)) {
            // 检查当前行是否包含该图片名称
            if (line.find(imageName) != std::string::npos) {
                // 如果找到，写入到输出文件
                outputFile << line << std::endl;
                break; // 找到后可以跳出循环，只需复制一次
            }
        }
    }

    std::cout << "提取完成，已将匹配的行复制到 " << outputTxtPath << std::endl;

    inputFile.close();
    outputFile.close();
}

// 制作查询深度图
void copyImagesAndReport(const std::string& folderA, const std::string& folderB, const std::string& folderC) {
    std::vector<std::string> imageNames;

    // 从文件夹A读取图片文件名
    for (const auto& entry : fs::directory_iterator(folderA)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") { // 根据需要修改扩展名
            imageNames.push_back(entry.path().filename().string());
        }
    }

    // 确保目标文件夹C存在
    if (!fs::exists(folderC)) {
        fs::create_directories(folderC); // 创建目标文件夹（如果不存在）
    }

    // 记录未找到的文件名
    std::vector<std::string> notFound;

    // 逐个检查文件夹A中的文件名
    for (const auto& imageName : imageNames) {
        std::string sourceFilePath = folderB + "/" + imageName; // 在文件夹B中查找的路径

        // 检查文件夹B是否存在文件
        if (fs::exists(sourceFilePath)) {
            // 复制文件到文件夹C
            std::string destFilePath = folderC + "/" + imageName; // 目标路径
            try {
                fs::copy(sourceFilePath, destFilePath, fs::copy_options::overwrite_existing);
                // std::cout << "已复制: " << sourceFilePath << " 到 " << destFilePath << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "复制文件失败: " << e.what() << std::endl;
            }
        } else {
            // 未找到，记录名称
            notFound.push_back(imageName);
        }
    }

    // 输出未找到的文件名称
    if (!notFound.empty()) {
        std::cout << "未找到以下文件:" << std::endl;
        for (const auto& name : notFound) {
            std::cout << name << std::endl;
        }
    } else {
        std::cout << "所有文件均已找到并复制。" << std::endl;
    }
}

// 制作地图深度图
void copyImagesFromTxt(const std::string& txtFilePath, const std::string& folderA, const std::string& folderB) {
    std::vector<std::string> imageNames;

    // 从txt文件读取需要查找的图片名称
    std::ifstream txtFile(txtFilePath);
    if (!txtFile.is_open()) {
        std::cerr << "无法打开txt文件: " << txtFilePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(txtFile, line)) {
        // 找到第一个空格，并获取从该空格后到行末的字符串
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            std::string imageName = line.substr(spacePos + 1); // 从第一个空格后开始
            imageNames.push_back(imageName);
        }
    }
    txtFile.close();

    // 确保目标文件夹B存在
    if (!fs::exists(folderB)) {
        fs::create_directories(folderB); // 创建目标文件夹（如果不存在）
    }

    // 检查文件夹A中的图片
    for (const auto& imageName : imageNames) {
        std::string imagePathA = folderA + "/" + imageName + ".png"; // 文件夹A中图片的完整路径

        // 检查图片是否存在于文件夹A
        if (fs::exists(imagePathA)) {
            std::string destPathB = folderB + "/" + imageName + ".png"; // 目标路径
            try {
                fs::copy(imagePathA, destPathB, fs::copy_options::overwrite_existing); // 复制文件
                // std::cout << "已复制: " << imageName << " 到 " << destPathB << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "复制文件失败: " << e.what() << std::endl;
            }
        } else {
            // 文件夹A中未找到该图片
            std::cout << "未找到图片: " << imageName << std::endl;
        }
    }
}
