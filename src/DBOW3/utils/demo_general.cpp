#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <dirent.h>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include "DBoW3.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "DescManip.h"

using namespace DBoW3;
using namespace std;
namespace fs = std::filesystem;

// 定义结构体来存储图片名称和 x 轴数据
struct ImageData {
    string image_name;
    double x_axis_data;
};

vector<string> readImagePathsFromFolder(const string &folder_path) {
    vector<string> paths;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder_path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string file_name(ent->d_name);
            if (file_name.find(".png") != string::npos) {
                paths.push_back(folder_path + "/" + file_name);
            }
        }
        closedir(dir);
    } else {
        cerr << "Error opening folder: " << folder_path << endl;
        exit(0);
    }
    return paths;
}

// void saveFeaturesBatchToFile(string filename, const vector<cv::Mat> &features) {
//     std::ofstream ofile(filename, std::ios::binary | std::ios::app); // Use append mode
//     if (!ofile.is_open()) {
//         cerr << "Could not open output file " << filename << endl;
//         throw std::runtime_error("Could not open output file");
//     }
//     uint32_t size = features.size();
//     ofile.write((char*)&size, sizeof(size));
//     for (auto &f : features) {
//         if (!f.isContinuous()) {
//             cerr << "Matrices should be continuous" << endl;
//             throw std::runtime_error("Matrices should be continuous");
//         }
//         uint32_t aux = f.cols;
//         ofile.write((char*)&aux, sizeof(aux));
//         aux = f.rows;
//         ofile.write((char*)&aux, sizeof(aux));
//         aux = f.type();
//         ofile.write((char*)&aux, sizeof(aux));
//         ofile.write((char*)f.ptr<uchar>(0), f.total() * f.elemSize());
//     }
//     ofile.close();
// }

// command line parser
class CmdLineParser {
    int argc;
    char **argv;
public:
    CmdLineParser(int _argc, char **_argv) : argc(_argc), argv(_argv) {}

    bool operator[](string param) {
        int idx = -1;
        for (int i = 0; i < argc && idx == -1; i++)
            if (string(argv[i]) == param)
                idx = i;
        return (idx != -1);
    }

    string operator()(string param, string defvalue = "-1") {
        int idx = -1;
        for (int i = 0; i < argc && idx == -1; i++)
            if (string(argv[i]) == param)
                idx = i;
        if (idx == -1)
            return defvalue;
        else
            return (argv[idx + 1]);
    }
};

vector<string> readImagePaths(const string &folderPath) {
    vector<string> paths;
    for (const auto &entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".jpg" || entry.path().extension() == ".png")) {
            paths.push_back(entry.path().string());
        }
    }
    return paths;
}

vector<cv::Mat> loadFeatures(const vector<string> &path_to_images, const string &descriptor) {
    // select detector
    cv::Ptr<cv::Feature2D> fdetector;
    if (descriptor == "orb")
        fdetector = cv::ORB::create();
    else
        throw std::runtime_error("Invalid descriptor");
    assert(!descriptor.empty());
    vector<cv::Mat> features;

    cout << "Extracting features..." << endl;
    for (size_t i = 0; i < path_to_images.size(); ++i) {
        vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        cout << "reading image: " << path_to_images[i] << endl;
        cv::Mat image = cv::imread(path_to_images[i], 0);
        if (image.empty())
            throw std::runtime_error("Could not open image: " + path_to_images[i]);
        cout << "extracting features" << endl;
        fdetector->detectAndCompute(image, cv::Mat(), keypoints, descriptors);
        features.push_back(descriptors);
        cout << "done detecting features" << endl;
    }
    return features;
}

void testVocCreation(const vector<cv::Mat> &features, const string &tempFolderPath) {
    // branching factor and depth levels
    const int k = 9;
    const int L = 3;
    const WeightingType weight = TF_IDF;
    const ScoringType score = L1_NORM;

    DBoW3::Vocabulary voc(k, L, weight, score);

    cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
    voc.create(features);
    cout << "... done!" << endl;

    cout << "Vocabulary information: " << endl
         << voc << endl << endl;

    // save the vocabulary to disk in the specified temp folder
    string vocFilePath = tempFolderPath + "/small_voc.yml.gz";
    cout << endl << "Saving vocabulary to " << vocFilePath << endl;
    voc.save(vocFilePath);
    cout << "Done" << endl;
}

void testDatabase(const string &queryImagePath, const vector<cv::Mat> &features, const vector<string> &imagePaths, const cv::Mat &queryFeature, const string &tempFolderPath) {
    cout << "Creating a small database..." << endl;

    string results_filename = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/query_results.txt";
    ofstream outfile(results_filename, std::ios_base::app); // 以追加模式打开文件
    if (!outfile.is_open()) {
        cerr << "Could not open output file " << results_filename << endl;
        exit(0);
    }

    // load the vocabulary from disk
    string vocFilePath = tempFolderPath + "/small_voc.yml.gz";
    Vocabulary voc(vocFilePath);

    Database db(voc, false, 0); // false = do not use direct index
    // (so ignore the last param)
    // The direct index is useful if we want to retrieve the features that
    // belong to some vocabulary node.
    // db creates a copy of the vocabulary, we may get rid of "voc" now

    // add images to the database
    for (const auto &feature : features)
        db.add(feature);

    cout << "... done!" << endl;

    cout << "Database information: " << endl << db << endl;

    // and query the database
    cout << "Querying the database: " << endl;

    QueryResults ret;
    db.query(queryFeature, ret, 1); // 查询最接近的一个结果

    // ret[0] is the best match.
    if (!ret.empty()) {
        string matched_name = fs::path(imagePaths[ret[0].Id]).stem().string();
        string query_name = fs::path(queryImagePath).stem().string();
        cout << "Best match for query image: " << matched_name << " Score: " << ret[0].Score << endl;

        // 输出到txt文件，追加模式
        ofstream outfile("/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/query_results.txt", std::ios_base::app);
        if (!outfile.is_open()) {
            cerr << "Could not open output file" << endl;
            return;
        }
        outfile << query_name << " " << matched_name << endl;
        outfile.close();
        // cout << "Best match for query image: " << imagePaths[ret[0].Id] << " Score: " << ret[0].Score << endl;
    } else {
        cout << "No match found for query image." << endl;
    }

    cout << endl;

    // save the database to disk in the specified temp folder
    string dbFilePath = tempFolderPath + "/small_db.yml.gz";
    cout << "Saving database to " << dbFilePath << endl;
    db.save(dbFilePath);
    cout << "... done!" << endl;

    // once saved, we can load it again
    cout << "Retrieving database once again..." << endl;
    Database db2(dbFilePath);
    cout << "... done! This is: " << endl << db2 << endl;
}

cv::Mat loadQueryFeature(const string &queryImagePath, const string &descriptor) {
    // select detector
    cv::Ptr<cv::Feature2D> fdetector;
    if (descriptor == "orb")
        fdetector = cv::ORB::create();
    else
        throw std::runtime_error("Invalid descriptor");
    assert(!descriptor.empty());

    vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    cout << "reading query image: " << queryImagePath << endl;
    cv::Mat image = cv::imread(queryImagePath, 0);
    if (image.empty())
        throw std::runtime_error("Could not open query image: " + queryImagePath);
    cout << "extracting features" << endl;
    fdetector->detectAndCompute(image, cv::Mat(), keypoints, descriptors);
    cout << "done detecting features" << endl;

    return descriptors;
}

void findClosestImageInDatabase(const string &descriptor, const string &queryImagePath, const string &folderPath, const string &tempFolderPath) {
    vector<string> imagePaths = readImagePaths(folderPath);
    if (imagePaths.empty()) {
        throw std::runtime_error("No images found in the folder: " + folderPath);
    }

    vector<cv::Mat> features = loadFeatures(imagePaths, descriptor);

    testVocCreation(features, tempFolderPath);

    cv::Mat queryFeature = loadQueryFeature(queryImagePath, descriptor);

    testDatabase(queryImagePath, features, imagePaths, queryFeature, tempFolderPath);
}

void createOutputFolderIfNotExists(const string& output_folder) {
    if (access(output_folder.c_str(), F_OK) == -1) {
        if (mkdir(output_folder.c_str(), 0777) == -1) {
            cerr << "Failed to create output folder: " << output_folder << endl;
            exit(0);
        }
    }
}

void clearFolderContents(const string& folder_path) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder_path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string file_name(ent->d_name);
            if (file_name == "." || file_name == "..") continue; // Skip current and parent directory
            string full_path = folder_path + "/" + file_name;
            if (unlink(full_path.c_str()) == -1) {
                cerr << "Failed to delete file: " << full_path << endl;
            }
        }
        closedir(dir);
    } else {
        cerr << "Error opening folder: " << folder_path << endl;
        exit(0);
    }
}

// 从txt文件读取内容，写入结构体数组
vector<ImageData> readImageDataFromFile(const string &filename) {
    vector<ImageData> imageDataArray;
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Could not open input file " << filename << endl;
        exit(0);
    }

    string line;
    while (getline(infile, line)) {
        istringstream iss(line);
        string image_name;
        double x_axis_data;

        if (iss >> image_name >> x_axis_data) {
            imageDataArray.push_back({image_name, x_axis_data});
        } else {
            cerr << "Error parsing line: " << line << endl;
        }
    }

    infile.close();
    return imageDataArray;
}

// void runMatching(const string &descriptor, const string &folder_path, const string &query_image_path, const string &output_folder) {
//     createOutputFolderIfNotExists(output_folder);

//     vector<string> image_paths = readImagePathsFromFolder(folder_path);
//     std::cout << 1 << std::endl;
//     if (image_paths.empty()) {
//         cerr << "No images found in folder: " << folder_path << endl;
//         return;
//     }

//     vector<string> query_image_paths = {query_image_path}; // 只处理单个查询图片路径
//     std::cout << 2 << std::endl;
//     processImagesInBatches(image_paths, descriptor, 100, output_folder); // Adjust the batch size as needed
//     std::cout << 3 << std::endl;

//     string features_filename = output_folder + "/features.bin";
//     vector<cv::Mat> all_features = loadFeaturesFromFile(features_filename);
//     std::cout << 4 << std::endl;

//     processImagesInBatches(query_image_paths, descriptor, 100, output_folder);
//     std::cout << 5 << std::endl;
//     vector<cv::Mat> query_features = loadFeaturesFromFile(features_filename);
//     std::cout << 6 << std::endl;

//     all_features.insert(all_features.end(), query_features.begin(), query_features.end());
//     std::cout << 7 << std::endl;

//     testVocCreation(all_features, output_folder);
//     std::cout << 8 << std::endl;

//     testDatabase(all_features, image_paths, query_image_path, output_folder);
//     std::cout << 9 << std::endl;

//     clearFolderContents(output_folder);
// }

// 根据图片名称查找x轴坐标的索引
int findClosestXCoordinateIndex(const double& queryXCoordinate, const std::vector<ImageData>& MapDataArray) {
    double minDifference = std::numeric_limits<double>::max();
    int closestIndex = -1;

    // 查找所有元素中绝对值最接近的 x 轴坐标
    for (size_t i = 0; i < MapDataArray.size(); ++i) {
        double difference = std::abs(MapDataArray[i].x_axis_data - queryXCoordinate);
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

void copyImage(const string &source_path, const string &destination_path) {
    try {
        // 检查源文件是否存在
        if (!fs::exists(source_path)) {
            cerr << "Source file does not exist: " << source_path << endl;
            return;
        }

        // 检查目标文件夹是否存在，如果不存在则创建
        fs::path dest_folder = fs::path(destination_path).parent_path();
        if (!fs::exists(dest_folder)) {
            fs::create_directories(dest_folder);
        }

        // 复制文件
        fs::copy(source_path, destination_path, fs::copy_options::overwrite_existing);
        // cout << "Image copied successfully from " << source_path << " to " << destination_path << endl;
    } catch (const fs::filesystem_error &e) {
        cerr << "Filesystem error: " << e.what() << endl;
    } catch (const exception &e) {
        cerr << "Exception: " << e.what() << endl;
    }
}

void removeAllFilesInFolder(const string &folder_path) {
    try {
        // 检查文件夹是否存在
        if (!fs::exists(folder_path)) {
            cerr << "Folder does not exist: " << folder_path << endl;
            return;
        }

        // 遍历文件夹中的所有条目
        for (const auto &entry : fs::directory_iterator(folder_path)) {
            // 检查是否为文件
            if (fs::is_regular_file(entry.status())) {
                // 删除文件
                fs::remove(entry.path());
                // cout << "Deleted file: " << entry.path() << endl;
            } else if (fs::is_directory(entry.status())) {
                cerr << "Encountered a directory within the folder: " << entry.path() << endl;
                // 如果需要递归删除子目录及其内容，可以取消注释以下代码
                // removeAllFilesInFolder(entry.path().string());
                // fs::remove(entry.path());
            }
        }
    } catch (const fs::filesystem_error &e) {
        cerr << "Filesystem error: " << e.what() << endl;
    } catch (const exception &e) {
        cerr << "Exception: " << e.what() << endl;
    }
}

int main() {
    string descriptor = "orb";
    string map_folder_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_maps";
    string match_folder_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_match";
    string output_folder = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/each_query";
    std::string map_txt_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/map_pose.txt";
    std::string match_txt_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/query_pose.txt";
    std::string map_temp_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/map_temp";

    std::vector<ImageData> map_data_array = readImageDataFromFile(map_txt_path);
    std::vector<ImageData> match_data_array = readImageDataFromFile(match_txt_path);
    for (size_t i = 0; i < match_data_array.size(); ++i) {
        int Index = findClosestXCoordinateIndex(match_data_array[i].x_axis_data, map_data_array);
        int startIndex = -1;
        int endIndex = -1;
        int real_size = -1;
        int size = -1;
        size = map_data_array.size();
        startIndex = std::max(0, Index - 50);
        endIndex = std::min(static_cast<int>(size), Index + 50);
        
        if (startIndex == 0) {
            endIndex = std::min(static_cast<int>(size), 100);
        }

        if (endIndex == size) {
            startIndex = std::max(0, static_cast<int>(size) - 100);
        }
        real_size = endIndex - startIndex;
        for (int j =0; j < real_size; ++j) {
            std::string image_path = map_folder_path + "/" + map_data_array[startIndex + j].image_name + ".png";
            copyImage(image_path, map_temp_path);
        }
        std::string query_image_path = match_folder_path + "/" + match_data_array[i].image_name + ".png";
        findClosestImageInDatabase(descriptor, query_image_path, map_temp_path, output_folder);
        // std::string query_image_path = match_folder_path + "/" + match_data_array[i].image_name + ".png";
        // runMatching(descriptor, map_folder_path, query_image_path, output_folder);
        removeAllFilesInFolder(map_temp_path);
    }
    // runMatching(descriptor, map_folder_path, "/media/rxsmmnb/dying_gull/viterbi_wall_match/1711513924.099062204.png", output_folder);

    // runMatching(descriptor, map_folder_path, query_image_path, output_folder);

    return 0;
}

// #include <iostream>
// #include <vector>
// #include <string>
// #include <filesystem>

// // DBoW3
// #include "DBoW3.h"

// // OpenCV
// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/features2d/features2d.hpp>
// #include "DescManip.h"

// using namespace DBoW3;
// using namespace std;
// namespace fs = std::filesystem;

// // command line parser
// class CmdLineParser {
//     int argc;
//     char **argv;
// public:
//     CmdLineParser(int _argc, char **_argv) : argc(_argc), argv(_argv) {}

//     bool operator[](string param) {
//         int idx = -1;
//         for (int i = 0; i < argc && idx == -1; i++)
//             if (string(argv[i]) == param)
//                 idx = i;
//         return (idx != -1);
//     }

//     string operator()(string param, string defvalue = "-1") {
//         int idx = -1;
//         for (int i = 0; i < argc && idx == -1; i++)
//             if (string(argv[i]) == param)
//                 idx = i;
//         if (idx == -1)
//             return defvalue;
//         else
//             return (argv[idx + 1]);
//     }
// };

// vector<string> readImagePaths(const string &folderPath) {
//     vector<string> paths;
//     for (const auto &entry : fs::directory_iterator(folderPath)) {
//         if (entry.is_regular_file() && (entry.path().extension() == ".jpg" || entry.path().extension() == ".png")) {
//             paths.push_back(entry.path().string());
//         }
//     }
//     return paths;
// }

// vector<cv::Mat> loadFeatures(const vector<string> &path_to_images, const string &descriptor) {
//     // select detector
//     cv::Ptr<cv::Feature2D> fdetector;
//     if (descriptor == "orb")
//         fdetector = cv::ORB::create();
//     else
//         throw std::runtime_error("Invalid descriptor");
//     assert(!descriptor.empty());
//     vector<cv::Mat> features;

//     cout << "Extracting features..." << endl;
//     for (size_t i = 0; i < path_to_images.size(); ++i) {
//         vector<cv::KeyPoint> keypoints;
//         cv::Mat descriptors;
//         cout << "reading image: " << path_to_images[i] << endl;
//         cv::Mat image = cv::imread(path_to_images[i], 0);
//         if (image.empty())
//             throw std::runtime_error("Could not open image: " + path_to_images[i]);
//         cout << "extracting features" << endl;
//         fdetector->detectAndCompute(image, cv::Mat(), keypoints, descriptors);
//         features.push_back(descriptors);
//         cout << "done detecting features" << endl;
//     }
//     return features;
// }

// void testVocCreation(const vector<cv::Mat> &features, const string &tempFolderPath) {
//     // branching factor and depth levels
//     const int k = 9;
//     const int L = 3;
//     const WeightingType weight = TF_IDF;
//     const ScoringType score = L1_NORM;

//     DBoW3::Vocabulary voc(k, L, weight, score);

//     cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
//     voc.create(features);
//     cout << "... done!" << endl;

//     cout << "Vocabulary information: " << endl
//          << voc << endl << endl;

//     // save the vocabulary to disk in the specified temp folder
//     string vocFilePath = tempFolderPath + "/small_voc.yml.gz";
//     cout << endl << "Saving vocabulary to " << vocFilePath << endl;
//     voc.save(vocFilePath);
//     cout << "Done" << endl;
// }

// void testDatabase(const string &queryImagePath, const vector<cv::Mat> &features, const vector<string> &imagePaths, const cv::Mat &queryFeature, const string &tempFolderPath) {
//     cout << "Creating a small database..." << endl;

//     string results_filename = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/query_results.txt";
//     ofstream outfile(results_filename, std::ios_base::app); // 以追加模式打开文件
//     if (!outfile.is_open()) {
//         cerr << "Could not open output file " << results_filename << endl;
//         exit(0);
//     }

//     // load the vocabulary from disk
//     string vocFilePath = tempFolderPath + "/small_voc.yml.gz";
//     Vocabulary voc(vocFilePath);

//     Database db(voc, false, 0); // false = do not use direct index
//     // (so ignore the last param)
//     // The direct index is useful if we want to retrieve the features that
//     // belong to some vocabulary node.
//     // db creates a copy of the vocabulary, we may get rid of "voc" now

//     // add images to the database
//     for (const auto &feature : features)
//         db.add(feature);

//     cout << "... done!" << endl;

//     cout << "Database information: " << endl << db << endl;

//     // and query the database
//     cout << "Querying the database: " << endl;

//     QueryResults ret;
//     db.query(queryFeature, ret, 1); // 查询最接近的一个结果

//     // ret[0] is the best match.
//     if (!ret.empty()) {
//         string matched_name = fs::path(imagePaths[ret[0].Id]).stem().string();
//         string query_name = fs::path(queryImagePath).stem().string();
//         cout << "Best match for query image: " << matched_name << " Score: " << ret[0].Score << endl;

//         // 输出到txt文件，追加模式
//         ofstream outfile("/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/query_results.txt", std::ios_base::app);
//         if (!outfile.is_open()) {
//             cerr << "Could not open output file" << endl;
//             return;
//         }
//         outfile << query_name << " " << matched_name << endl;
//         outfile.close();
//         // cout << "Best match for query image: " << imagePaths[ret[0].Id] << " Score: " << ret[0].Score << endl;
//     } else {
//         cout << "No match found for query image." << endl;
//     }

//     cout << endl;

//     // save the database to disk in the specified temp folder
//     string dbFilePath = tempFolderPath + "/small_db.yml.gz";
//     cout << "Saving database to " << dbFilePath << endl;
//     db.save(dbFilePath);
//     cout << "... done!" << endl;

//     // once saved, we can load it again
//     cout << "Retrieving database once again..." << endl;
//     Database db2(dbFilePath);
//     cout << "... done! This is: " << endl << db2 << endl;
// }

// cv::Mat loadQueryFeature(const string &queryImagePath, const string &descriptor) {
//     // select detector
//     cv::Ptr<cv::Feature2D> fdetector;
//     if (descriptor == "orb")
//         fdetector = cv::ORB::create();
//     else
//         throw std::runtime_error("Invalid descriptor");
//     assert(!descriptor.empty());

//     vector<cv::KeyPoint> keypoints;
//     cv::Mat descriptors;
//     cout << "reading query image: " << queryImagePath << endl;
//     cv::Mat image = cv::imread(queryImagePath, 0);
//     if (image.empty())
//         throw std::runtime_error("Could not open query image: " + queryImagePath);
//     cout << "extracting features" << endl;
//     fdetector->detectAndCompute(image, cv::Mat(), keypoints, descriptors);
//     cout << "done detecting features" << endl;

//     return descriptors;
// }

// void findClosestImageInDatabase(const string &descriptor, const string &queryImagePath, const string &folderPath, const string &tempFolderPath) {
//     vector<string> imagePaths = readImagePaths(folderPath);
//     if (imagePaths.empty()) {
//         throw std::runtime_error("No images found in the folder: " + folderPath);
//     }

//     vector<cv::Mat> features = loadFeatures(imagePaths, descriptor);

//     testVocCreation(features, tempFolderPath);

//     cv::Mat queryFeature = loadQueryFeature(queryImagePath, descriptor);

//     testDatabase(queryImagePath, features, imagePaths, queryFeature, tempFolderPath);
// }

// int main() {
//     // 指定路径
//     string descriptor = "orb";
//     string queryImagePath = "/media/rxsmmnb/dying_gull/viterbi_wall_match/1711513924.099062204.png"; // 替换为你的查询图片路径
//     string folderPath = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/map_temp"; // 替换为你的文件夹路径
//     string tempFolderPath = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/each_query"; // 替换为你的临时文件夹路径

//     // 确保临时文件夹存在
//     if (!fs::exists(tempFolderPath)) {
//         fs::create_directories(tempFolderPath);
//         cout << "Created temp folder: " << tempFolderPath << endl;
//     }

//     findClosestImageInDatabase(descriptor, queryImagePath, folderPath, tempFolderPath);

//     return 0;
// }


// /**
//  * Date:  2016
//  * Author: Rafael Muñoz Salinas
//  * Description: demo application of DBoW3
//  * License: see the LICENSE.txt file
//  */

// #include <iostream>
// #include <vector>

// // DBoW3
// #include "DBoW3.h"

// // OpenCV
// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/features2d/features2d.hpp>
// #ifdef USE_CONTRIB
// #include <opencv2/xfeatures2d/nonfree.hpp>
// #include <opencv2/xfeatures2d.hpp>
// #endif
// #include "DescManip.h"

// using namespace DBoW3;
// using namespace std;


// //command line parser
// class CmdLineParser{int argc; char **argv; public: CmdLineParser(int _argc,char **_argv):argc(_argc),argv(_argv){}  bool operator[] ( string param ) {int idx=-1;  for ( int i=0; i<argc && idx==-1; i++ ) if ( string ( argv[i] ) ==param ) idx=i;    return ( idx!=-1 ) ;    } string operator()(string param,string defvalue="-1"){int idx=-1;    for ( int i=0; i<argc && idx==-1; i++ ) if ( string ( argv[i] ) ==param ) idx=i; if ( idx==-1 ) return defvalue;   else  return ( argv[  idx+1] ); }};


// // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// // extended surf gives 128-dimensional vectors
// const bool EXTENDED_SURF = false;
// // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// void wait()
// {
//     cout << endl << "Press enter to continue" << endl;
//     getchar();
// }


// vector<string> readImagePaths(int argc,char **argv,int start){
//     vector<string> paths;
//     for(int i=start;i<argc;i++)    paths.push_back(argv[i]);
//         return paths;
// }

// vector< cv::Mat  >  loadFeatures( std::vector<string> path_to_images,string descriptor="") throw (std::exception){
//     //select detector
//     cv::Ptr<cv::Feature2D> fdetector;
//     if (descriptor=="orb")        fdetector=cv::ORB::create();
//     else if (descriptor=="brisk") fdetector=cv::BRISK::create();
// #ifdef OPENCV_VERSION_3
//     else if (descriptor=="akaze") fdetector=cv::AKAZE::create();
// #endif
// #ifdef USE_CONTRIB
//     else if(descriptor=="surf" )  fdetector=cv::xfeatures2d::SURF::create(400, 4, 2, EXTENDED_SURF);
// #endif

//     else throw std::runtime_error("Invalid descriptor");
//     assert(!descriptor.empty());
//     vector<cv::Mat>    features;


//     cout << "Extracting   features..." << endl;
//     for(size_t i = 0; i < path_to_images.size(); ++i)
//     {
//         vector<cv::KeyPoint> keypoints;
//         cv::Mat descriptors;
//         cout<<"reading image: "<<path_to_images[i]<<endl;
//         cv::Mat image = cv::imread(path_to_images[i], 0);
//         if(image.empty())throw std::runtime_error("Could not open image"+path_to_images[i]);
//         cout<<"extracting features"<<endl;
//         fdetector->detectAndCompute(image, cv::Mat(), keypoints, descriptors);
//         features.push_back(descriptors);
//         cout<<"done detecting features"<<endl;
//     }
//     return features;
// }

// // ----------------------------------------------------------------------------

// void testVocCreation(const vector<cv::Mat> &features)
// {
//     // branching factor and depth levels
//     const int k = 9;
//     const int L = 3;
//     const WeightingType weight = TF_IDF;
//     const ScoringType score = L1_NORM;

//     DBoW3::Vocabulary voc(k, L, weight, score);

//     cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
//     voc.create(features);
//     cout << "... done!" << endl;

//     cout << "Vocabulary information: " << endl
//          << voc << endl << endl;

//     // lets do something with this vocabulary
//     cout << "Matching images against themselves (0 low, 1 high): " << endl;
//     BowVector v1, v2;
//     for(size_t i = 0; i < features.size(); i++)
//     {
//         voc.transform(features[i], v1);
//         for(size_t j = 0; j < features.size(); j++)
//         {
//             voc.transform(features[j], v2);

//             double score = voc.score(v1, v2);
//             cout << "Image " << i << " vs Image " << j << ": " << score << endl;
//         }
//     }

//     // save the vocabulary to disk
//     cout << endl << "Saving vocabulary..." << endl;
//     voc.save("small_voc.yml.gz");
//     cout << "Done" << endl;
// }

// ////// ----------------------------------------------------------------------------

// void testDatabase(const  vector<cv::Mat > &features)
// {
//     cout << "Creating a small database..." << endl;

//     // load the vocabulary from disk
//     Vocabulary voc("small_voc.yml.gz");

//     Database db(voc, false, 0); // false = do not use direct index
//     // (so ignore the last param)
//     // The direct index is useful if we want to retrieve the features that
//     // belong to some vocabulary node.
//     // db creates a copy of the vocabulary, we may get rid of "voc" now

//     // add images to the database
//     for(size_t i = 0; i < features.size(); i++)
//         db.add(features[i]);

//     cout << "... done!" << endl;

//     cout << "Database information: " << endl << db << endl;

//     // and query the database
//     cout << "Querying the database: " << endl;

//     QueryResults ret;
//     for(size_t i = 0; i < features.size(); i++)
//     {
//         db.query(features[i], ret, 4);

//         // ret[0] is always the same image in this case, because we added it to the
//         // database. ret[1] is the second best match.

//         cout << "Searching for Image " << i << ". " << ret << endl;
//     }

//     cout << endl;

//     // we can save the database. The created file includes the vocabulary
//     // and the entries added
//     cout << "Saving database..." << endl;
//     db.save("small_db.yml.gz");
//     cout << "... done!" << endl;

//     // once saved, we can load it again
//     cout << "Retrieving database once again..." << endl;
//     Database db2("small_db.yml.gz");
//     cout << "... done! This is: " << endl << db2 << endl;
// }


// // ----------------------------------------------------------------------------

// int main(int argc,char **argv)
// {

//     try{
//         CmdLineParser cml(argc,argv);
//         if (cml["-h"] || argc<=2){
//             cerr<<"Usage:  descriptor_name     image0 image1 ... \n\t descriptors:brisk,surf,orb ,akaze(only if using opencv 3)"<<endl;
//              return -1;
//         }

//         string descriptor=argv[1];

//         auto images=readImagePaths(argc,argv,2);
//         vector< cv::Mat   >   features= loadFeatures(images,descriptor);
//         testVocCreation(features);


//         testDatabase(features);

//     }catch(std::exception &ex){
//         cerr<<ex.what()<<endl;
//     }

//     return 0;
// }
