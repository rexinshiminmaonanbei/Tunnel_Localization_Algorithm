#include "verify.h"

// 读取文件内容，返回一个结构体数组
std::vector<VerifyStruct> readFile(const std::string& filename) {
    std::vector<VerifyStruct> structArray;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return structArray; // 返回一个空的结构体数组
    }

    std::string line;
    while (std::getline(file, line)) {
        // 检查是否有足够的字符
        if (line.length() >= 40) {
            VerifyStruct verify_struct;
            verify_struct.match_name = line.substr(0, 20); // 第1到20字符
            verify_struct.map_name = line.substr(21, 20); // 第22到41字符
            structArray.push_back(verify_struct);
        }
    }

    file.close();
    return structArray;
}

// 验证结果和真值数量是否匹配
bool areAllFirstElementsEqual(const std::vector<VerifyStruct>& result, const std::vector<VerifyStruct>& truth) {
    // 先检查数组大小是否相同
    if (result.size() != truth.size()) {
        return false; // 如果大小不相同，直接返回false
    }

    // 遍历每个元素，比较第一个字符串
    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i].match_name != truth[i].match_name) {
            std::cerr << "第" << i + 1 << "个结果和真值不匹配: " << result[i].match_name << " vs " << truth[i].match_name << std::endl;
            return false; // 如果有任何一个不相同，返回false
        }
    }
    
    return true; // 如果所有对应元素的第一个字符串都相同，返回true
}

// 读取文件夹中的图片文件名
std::vector<std::string> readImageNames(const std::string& folderPath) {
    std::vector<std::string> imageNames;

    // 使用 std::filesystem 遍历目录
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        // 检查是否是文件且扩展名为 .png
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            // 获取文件名并去掉 .png 后缀
            std::string filename = entry.path().stem().string();
            imageNames.push_back(filename);
        }
    }

    // 按字母顺序排序文件名
    std::sort(imageNames.begin(), imageNames.end());

    return imageNames;
}

// 复制文件
void copyFile(const std::string &sourcePath, const std::string &destPath) {
    std::ifstream sourceFile(sourcePath); // 打开源文件
    std::ofstream destFile(destPath, std::ios::trunc); // 打开目标文件并清空

    // 检查源文件是否成功打开
    if (!sourceFile) {
        std::cerr << "Error opening source file: " << sourcePath << std::endl;
        return;
    }

    // 检查目标文件是否成功打开
    if (!destFile) {
        std::cerr << "Error opening destination file: " << destPath << std::endl;
        return;
    }

    std::string line;
    // 跳过前三行
    for (int i = 0; i < 3; ++i) {
        if (std::getline(sourceFile, line)) {
            // 继续读取以跳过前三行
        }
    }

    // 逐行读取源文件剩余内容并写入目标文件
    while (std::getline(sourceFile, line)) {
        destFile << line << std::endl; // 写入目标文件
    }

    // 关闭文件
    sourceFile.close();
    destFile.close();
    std::cout << "文件复制完成。" << std::endl;
}

// 删除文本文件中的指定图片名称的行
void removeImageLines(const std::vector<std::string>& imageNames, std::string& txtFilePath) {
    std::ifstream inFile(txtFilePath); // 打开文本文件以读取
    if (!inFile) {
        std::cerr << "Error opening file: " << txtFilePath << std::endl;
        return;
    }

    // 创建一个临时向量来存储不需要删除的行
    std::vector<std::string> remainingLines;
    std::string line;

    // 逐行读取文件内容
    while (std::getline(inFile, line)) {
        bool toDelete = false;
        
        // 检查当前行是否包含要删除的图片名称
        for (const auto& imageName : imageNames) {
            if (line.find(imageName) != std::string::npos) {
                toDelete = true; // 找到了该图片名称，标记为删除
                break;
            }
        }

        // 如果没有匹配到图片名称，则保留该行
        if (!toDelete) {
            remainingLines.push_back(line);
        }
    }

    inFile.close(); // 关闭阅读文件

    // 重新打开文件以写入
    std::ofstream outFile(txtFilePath, std::ios::trunc); // 以截断模式打开文件
    if (!outFile) {
        std::cerr << "Error opening file for writing: " << txtFilePath << std::endl;
        return;
    }

    // 将剩余的行写回文件
    for (const auto& remainingLine : remainingLines) {
        outFile << remainingLine << std::endl; // 写入文件
    }

    outFile.close(); // 关闭写入文件

    std::cout << "txt里匹配正确的图片已保留，其余图片已删除。" << std::endl;
}

// 加上图像级定位的误差
void pictureErrorAddition(const std::vector<std::string>& imageNames_10cm, 
                      const std::vector<std::string>& imageNames_7cm,
                      const std::vector<std::string>& imageNames_4cm, 
                      std::string& txtFilePath) {
    std::ifstream inputFile(txtFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件: " << txtFilePath << std::endl;
        return;
    }

    // 用于存储文件中的每一行
    std::vector<std::string> lines;
    std::string line;

    // 读取文件内容到容器
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }
    inputFile.close();

    // 找到并更新每种图像名称对应的行
    // 处理 4cm 的图像名称
    for (const auto& imgName : imageNames_4cm) {
        for (auto& currentLine : lines) {
            // 找到第一个空格的位置
            size_t firstSpacePos = currentLine.find(' ');
            if (firstSpacePos != std::string::npos) {
                // 找到第二个空格的位置
                size_t secondSpacePos = currentLine.find(' ', firstSpacePos + 1);
                // 获取第一个空格和第二个空格之间的内容
                std::string nameToCompare = currentLine.substr(firstSpacePos + 1, 
                                                                secondSpacePos - firstSpacePos - 1);
                if (nameToCompare == imgName) { // 比较内容
                    currentLine += " 3.333"; // 在行末尾添加内容
                }
            }
        }
    }

    // 处理 7cm 的图像名称
    for (const auto& imgName : imageNames_7cm) {
        for (auto& currentLine : lines) {
            // 找到第一个空格的位置
            size_t firstSpacePos = currentLine.find(' ');
            if (firstSpacePos != std::string::npos) {
                // 找到第二个空格的位置
                size_t secondSpacePos = currentLine.find(' ', firstSpacePos + 1);
                // 获取第一个空格和第二个空格之间的内容
                std::string nameToCompare = currentLine.substr(firstSpacePos + 1, 
                                                                secondSpacePos - firstSpacePos - 1);
                if (nameToCompare == imgName) { // 比较内容
                    currentLine += " 6.667"; // 在行末尾添加内容
                }
            }
        }
    }

    // 处理 10cm 的图像名称
    for (const auto& imgName : imageNames_10cm) {
        for (auto& currentLine : lines) {
            // 找到第一个空格的位置
            size_t firstSpacePos = currentLine.find(' ');
            if (firstSpacePos != std::string::npos) {
                // 找到第二个空格的位置
                size_t secondSpacePos = currentLine.find(' ', firstSpacePos + 1);
                // 获取第一个空格和第二个空格之间的内容
                std::string nameToCompare = currentLine.substr(firstSpacePos + 1, 
                                                                secondSpacePos - firstSpacePos - 1);
                if (nameToCompare == imgName) { // 比较内容
                    currentLine += " 9.999"; // 在行末尾添加内容
                }
            }
        }
    }

    // 将更新后的内容写回到文件
    std::ofstream outputFile(txtFilePath, std::ios::trunc); // 清空文件并准备写入
    if (!outputFile) {
        std::cerr << "无法打开输出文件: " << txtFilePath << std::endl;
        return;
    }

    // 输出更新后的行
    for (const auto& updatedLine : lines) {
        outputFile << updatedLine << std::endl;
    }

    outputFile.close();
    std::cout << "图像级定位的误差已添加。" << std::endl;
}

// 保留文件中每行最小的值
void retainMinimumValuesInFile(const std::string& txtFilePath) {
    std::ifstream inputFile(txtFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件: " << txtFilePath << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string line;

    // 读取文件内容到容器
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }
    inputFile.close();

    // 存储更新后的行
    std::vector<std::string> updatedLines;

    for (const auto& currentLine : lines) {
        std::istringstream ss(currentLine);
        std::string name1, name2;
        
        // 假设前两个元素是名称，读取它们
        ss >> name1 >> name2;

        // 存储数字
        std::vector<double> numbers;
        double number;

        // 读取后面的所有数字
        while (ss >> number) {
            numbers.push_back(number);
        }

        // 找到最小的数字
        double minValue = 10.000;
        for (const double& num : numbers) {
            if (num < minValue) {
                minValue = num;
            }
        }

        // 生成更新后的行
        std::string updatedLine = name1 + " " + name2 + " " + std::to_string(minValue);
        updatedLines.push_back(updatedLine);
    }

    // 将更新后的内容写回到文件
    std::ofstream outputFile(txtFilePath, std::ios::trunc); // 清空文件并准备写入
    if (!outputFile.is_open()) {
        std::cerr << "无法打开输出文件: " << txtFilePath << std::endl;
        return;
    }

    for (const auto& updatedLine : updatedLines) {
        outputFile << updatedLine << std::endl;
    }

    outputFile.close();
}

// 判断准确率
AccuracyStruct calculateAccuracy(const std::string& results, const std::string& truths, const std::string& wall_map_path,
                                std::string& true_text_path) {
    // 读取结果文件
    std::vector<VerifyStruct> results_array = readFile(results);
    // 读取真值文件
    std::vector<VerifyStruct> truths_array = readFile(truths);
    // 存放错误的结果的数组
    std::vector<std::string> error_names;

    // 复制文件
    copyFile(results, true_text_path);

    // 判断两个数组是否匹配
    if (!areAllFirstElementsEqual(results_array, truths_array)) {
        std::cerr << "结果和真值不匹配" << std::endl;
        return {0.0, 0.0, 0.0}; // 返回0.0
    }

    // 读取文件夹中的图片文件名
    std::vector<std::string> image_paths = readImageNames(wall_map_path);

    int correctCount_10cm = 0; // 记录前后三张准确数
    int correctCount_7cm = 0;   // 记录前后两张准确数
    int correctCount_4cm = 0;   // 记录前后一张准确数
    int totalCount = results_array.size() - 0; // 结果总数
    std::vector<std::string> correctImages_10cm;
    std::vector<std::string> correctImages_7cm;
    std::vector<std::string> correctImages_4cm;

    // 计算准确率
    for (int i = 0; i < results_array.size(); ++i) {
        if (i > -1){
            // 定义临时变量
            std::string temp_truth = truths_array[i].map_name;
            std::string temp_result = results_array[i].map_name;

            // 在 image_paths 中查找 temp_truth
            auto it = std::find(image_paths.begin(), image_paths.end(), temp_truth);
            if (it != image_paths.end()) {
                // 找到对应的位置
                size_t true_index = std::distance(image_paths.begin(), it);

                // 计算10cm的准确率
                if (true_index == 0) {
                    // 如果是第一个元素，只有与 true_index 及其下三个位置比较
                    if (temp_result == image_paths[true_index]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 2]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 3]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else {
                        error_names.push_back(temp_result);
                    }
                } 
                else if (true_index == 1) {
                    // 如果是第二个元素，开头少两个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 2]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 3]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else {
                        error_names.push_back(temp_result);
                    } 
                }
                else if (true_index == 2) {
                    // 如果是第三个元素，开头少一个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 2]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 2]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 3]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else {
                        error_names.push_back(temp_result);
                    } 
                }
                else if (true_index == image_paths.size() - 1) {
                    // 如果是最后一个元素，最后少三个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 2]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 3]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else {
                        error_names.push_back(temp_result);
                    }
                } 
                else if (true_index == image_paths.size() - 2) {
                    // 如果是倒数第二个元素，最后少两个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 2]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 3]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else {
                        error_names.push_back(temp_result);
                    }
                }
                else if (true_index == image_paths.size() - 3) {
                    // 如果是倒数第三个元素，最后少一个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 2]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 2]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 3]) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else {
                        error_names.push_back(temp_result);
                    }
                }
                else {
                    // 中间的元素
                    if (temp_result == image_paths[true_index - 3] ||
                        temp_result == image_paths[true_index - 2] || 
                        temp_result == image_paths[true_index - 1] || 
                        temp_result == image_paths[true_index] || 
                        temp_result == image_paths[true_index + 1] || 
                        temp_result == image_paths[true_index + 2] ||
                        temp_result == image_paths[true_index + 3] ) {
                        correctCount_10cm++;
                        correctImages_10cm.push_back(temp_result);
                    } else {
                        error_names.push_back(temp_result);
                    }
                }

                // 计算7cm的准确率
                if (true_index == 0) {
                    // 如果是第一个元素，只有与 true_index 及其下两个位置比较
                    if (temp_result == image_paths[true_index]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 2]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else {
                    }
                } 
                else if (true_index == 1) {
                    // 如果是第二个元素，开头少一个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 2]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else {
                    } 
                }
                else if (true_index == image_paths.size() - 1) {
                    // 如果是最后一个元素，最后少两个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 2]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else {
                    }
                } 
                else if (true_index == image_paths.size() - 2) {
                    // 如果是倒数第二个元素，最后少一个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 2]) {
                        correctCount_7cm++;
                        correctImages_7cm.push_back(temp_result);
                    } else {
                    }
                }
                else {
                    // 中间的元素
                    if (temp_result == image_paths[true_index - 2] || 
                            temp_result == image_paths[true_index - 1] || 
                            temp_result == image_paths[true_index] || 
                            temp_result == image_paths[true_index + 1] || 
                            temp_result == image_paths[true_index + 2] ) {
                            correctCount_7cm++;
                            correctImages_7cm.push_back(temp_result);
                        } else {
                    }
                } 
                
                // 计算4cm的准确率
                if (true_index == 0) {
                    // 如果是第一个元素，只有与 true_index 及其下一个位置比较
                    if (temp_result == image_paths[true_index]) {
                        correctCount_4cm++;
                        correctImages_4cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index + 1]) {
                        correctCount_4cm++;
                        correctImages_4cm.push_back(temp_result);
                    } else {
                    }
                } 
                else if (true_index == image_paths.size() - 1) {
                    // 如果是最后一个元素，最后少一个元素
                    if (temp_result == image_paths[true_index]) {
                        correctCount_4cm++;
                        correctImages_4cm.push_back(temp_result);
                    } else if (temp_result == image_paths[true_index - 1]) {
                        correctCount_4cm++;
                        correctImages_4cm.push_back(temp_result);
                    } else {
                    }
                } 
                else {
                        // 中间的元素
                        if (temp_result == image_paths[true_index - 1] || 
                            temp_result == image_paths[true_index] || 
                            temp_result == image_paths[true_index + 1] ) {
                            correctCount_4cm++;
                            correctImages_4cm.push_back(temp_result);
                        } else {
                        }
                }
            } else {
                // 如果 temp_truth 不在 image_paths 中的情况
                std::cerr << "错误: 在路径中未找到对应的真值 " << temp_truth 
                        << " (索引 " << i << ")" << std::endl;
            }
        }
    }

    // 删除错误行
    removeImageLines(error_names, true_text_path);
    // 加上图像级定位的误差
    pictureErrorAddition(correctImages_10cm, correctImages_7cm, correctImages_4cm, true_text_path);
    // 保留最小的误差
    retainMinimumValuesInFile(true_text_path);

    // 计算并返回准确率
    double accuracy_10cm = static_cast<double>(correctCount_10cm) / totalCount; 
    double accuracy_7cm = static_cast<double>(correctCount_7cm) / totalCount; 
    double accuracy_4cm = static_cast<double>(correctCount_4cm) / totalCount; 
    return {accuracy_10cm, accuracy_7cm, accuracy_4cm}; // 返回准确率
}