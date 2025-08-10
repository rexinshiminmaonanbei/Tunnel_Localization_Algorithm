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

// 判断准确率
AccuracyStruct calculateAccuracy(const std::string& results, const std::string& truths, const std::string& wall_map_path) {
    // 读取结果文件
    std::vector<VerifyStruct> results_array = readFile(results);
    // 读取真值文件
    std::vector<VerifyStruct> truths_array = readFile(truths);
    // 存放错误的结果的数组
    std::vector<std::string> error_names;

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
                        // std::cerr << " 错误: 在路径中未找到对应的真值 " << temp_truth;
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

    // 计算并返回准确率
    double accuracy_10cm = static_cast<double>(correctCount_10cm) / totalCount; 
    double accuracy_7cm = static_cast<double>(correctCount_7cm) / totalCount; 
    double accuracy_4cm = static_cast<double>(correctCount_4cm) / totalCount; 
    return {accuracy_10cm, accuracy_7cm, accuracy_4cm}; // 返回准确率
}