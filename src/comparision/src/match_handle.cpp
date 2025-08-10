#include "match_handle.h"

// 地图结构体数组
std::vector<ImageData> MapDataArray;
// 匹配结构体数组
std::vector<ImageData> MatchDataArray;

// 地图和匹配结构体构建
void buildImageData(const std::string& map_pose_path, const std::string& wall_map_path, const std::string& front_map_path,
            const std::string& match_pose_path, const std::string& wall_match_path, const std::string& front_match_path){
    std::cout <<  "开始读取地图数据" << std::endl;
    MapDataArray = readImageDataFromFile(map_pose_path, wall_map_path, front_map_path); // 读取地图数据
    std::cout <<  "地图数据读取完成" << std::endl;
    std::cout <<  "开始读取匹配数据" << std::endl;
    MatchDataArray = readImageDataFromFile(match_pose_path, wall_match_path, front_match_path); // 读取匹配数据
    std::cout <<  "匹配数据读取完成" << std::endl;

    // 测试用
    // 将 MapDataArray 写入文件
    {
        // 第一段
        std::ofstream mapFile("/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/多视角全局ORB/front_map_descriptors.txt");
        // 第二段
        // std::ofstream mapFile("/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/对比txt/多视角全局ORB/behind_map_descriptors.txt");
        if (!mapFile.is_open()) {
            std::cerr << "无法打开 map_descriptors.txt 文件" << std::endl;
            return;
        }
        for (const auto& data : MapDataArray) {
            mapFile << data.imageName << " " 
                     << data.x_value << " " 
                     << data.wall_descriptor1 << " " 
                     << data.wall_descriptor2 << " " 
                     << data.wall_descriptor4 << " "
                     << data.front_descriptor1 << " " 
                     << data.front_descriptor2 << " " 
                     << data.front_descriptor4 << std::endl;
        }
        mapFile.close();
    }

    // 将 MatchDataArray 写入文件
    {
        // 第一段
        std::ofstream matchFile("/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/多视角全局ORB/front_match_descriptors.txt");
        // 第二段
        // std::ofstream matchFile("/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/对比txt/多视角全局ORB/behind_match_descriptors.txt");
        if (!matchFile.is_open()) {
            std::cerr << "无法打开 match_descriptors.txt 文件" << std::endl;
            return;
        }
        for (const auto& data : MatchDataArray) {
            matchFile << data.imageName << " " 
                     << data.x_value << " " 
                     << data.wall_descriptor1 << " " 
                     << data.wall_descriptor2 << " " 
                     << data.wall_descriptor4 << " "
                     << data.front_descriptor1 << " " 
                     << data.front_descriptor2 << " " 
                     << data.front_descriptor4 << std::endl;
        }
        matchFile.close();
    }

    std::cout << "数据写入 map_descriptors.txt 和 match_descriptors.txt 完成" << std::endl;
}

// 第一段定义读取函数，测试用
void first_readImageDataFromTxt() {
    // 定义文件路径
    const std::string mapFilePath = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/多视角全局ORB/front_map_descriptors.txt";
    const std::string matchFilePath = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/多视角全局ORB/front_match_descriptors.txt";

    // 清空之前的数据
    MapDataArray.clear();
    MatchDataArray.clear();

    // 读取 MapDataArray
    std::ifstream mapFile(mapFilePath);
    if (!mapFile.is_open()) {
        std::cerr << "无法打开 " << mapFilePath << " 文件" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(mapFile, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream iss(line);
        ImageData data;
        iss >> data.imageName >> data.x_value 
            >> data.wall_descriptor1 >> data.wall_descriptor2 
            >> data.wall_descriptor4 >> data.front_descriptor1 
            >> data.front_descriptor2 >> data.front_descriptor4;
        MapDataArray.push_back(data);
    }
    mapFile.close();

    // 读取 MatchDataArray
    std::ifstream matchFile(matchFilePath);
    if (!matchFile.is_open()) {
        std::cerr << "无法打开 " << matchFilePath << " 文件" << std::endl;
        return;
    }
    while (std::getline(matchFile, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream iss(line);
        ImageData data;
        iss >> data.imageName >> data.x_value 
            >> data.wall_descriptor1 >> data.wall_descriptor2 
            >> data.wall_descriptor4 >> data.front_descriptor1 
            >> data.front_descriptor2 >> data.front_descriptor4;
        MatchDataArray.push_back(data);
    }
    matchFile.close();

    std::cout << "数据读取完成，MapDataArray 和 MatchDataArray 已填充。" << std::endl;
}

// 第二段定义读取函数，测试用
void second_readImageDataFromTxt() {
    // 定义文件路径
    const std::string mapFilePath = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/对比txt/多视角全局ORB/behind_map_descriptors.txt";
    const std::string matchFilePath = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/对比txt/多视角全局ORB/behind_match_descriptors.txt";

    // 清空之前的数据
    MapDataArray.clear();
    MatchDataArray.clear();

    // 读取 MapDataArray
    std::ifstream mapFile(mapFilePath);
    if (!mapFile.is_open()) {
        std::cerr << "无法打开 " << mapFilePath << " 文件" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(mapFile, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream iss(line);
        ImageData data;
        iss >> data.imageName >> data.x_value 
            >> data.wall_descriptor1 >> data.wall_descriptor2 
            >> data.wall_descriptor4 >> data.front_descriptor1 
            >> data.front_descriptor2 >> data.front_descriptor4;
        MapDataArray.push_back(data);
    }
    mapFile.close();

    // 读取 MatchDataArray
    std::ifstream matchFile(matchFilePath);
    if (!matchFile.is_open()) {
        std::cerr << "无法打开 " << matchFilePath << " 文件" << std::endl;
        return;
    }
    while (std::getline(matchFile, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream iss(line);
        ImageData data;
        iss >> data.imageName >> data.x_value 
            >> data.wall_descriptor1 >> data.wall_descriptor2 
            >> data.wall_descriptor4 >> data.front_descriptor1 
            >> data.front_descriptor2 >> data.front_descriptor4;
        MatchDataArray.push_back(data);
    }
    matchFile.close();

    std::cout << "数据读取完成，MapDataArray 和 MatchDataArray 已填充。" << std::endl;
}

// 开始匹配
void startMatch(const std::string& result_path, const std::string& LoFTR_path) {
    // 开始计时
    auto startTime = std::chrono::high_resolution_clock::now();

    // 读取数据
    first_readImageDataFromTxt();
    // second_readImageDataFromTxt();

    // 打开输出文件
    std::ofstream outputFile(result_path, std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "无法打开输出文件" << std::endl;
        return ;
    }

    // 读取LoFRT特征
    std::vector<ImageFeatures> imgFeaturesList = readAndProcessImageFeaturesFromFile(LoFTR_path);

    for(int i = 0; i < MatchDataArray.size(); i++){
        // 粗定位
        int Index = findClosestXCoordinateIndex(MatchDataArray[i].x_value, MapDataArray);
        // std::string LoFRT_name = imgFeaturesList[i].imageName;
        // std::string match_name = MatchDataArray[i].imageName;
        // if (LoFRT_name != match_name)
        // {
        //     std::cerr << "LoFRT特征文件和匹配数据文件不匹配" << std::endl;
        //     std::cerr << "LoFRT特征文件名：" << LoFRT_name << std::endl;
        //     std::cerr << "匹配数据文件名：" << match_name << std::endl;
        //     return ;
        // }
        // std::vector<double> LoFRT_descriptor = imgFeaturesList[i].features;
        // match_init(MapDataArray, Index);
        // int match_index = match_progress(MatchDataArray[i].wall_descriptor1, MatchDataArray[i].wall_descriptor2, MatchDataArray[i].wall_descriptor4, 
        //                                  MatchDataArray[i].front_descriptor1, MatchDataArray[i].front_descriptor2, MatchDataArray[i].front_descriptor4,
        //                                  LoFRT_descriptor,  MapDataArray);
        //  outputFile << MatchDataArray[i].imageName << " " << MapDataArray[match_index].imageName << std::endl;

        match_init(MapDataArray, Index);
        std::string match_path = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_match/" + MatchDataArray[i].imageName + ".png";
        // std::string match_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_match/" + MatchDataArray[i].imageName + ".png";
        int match_index = match_progress(match_path, MapDataArray);
        outputFile << MatchDataArray[i].imageName << " " << MapDataArray[match_index].imageName << std::endl;

    }
    outputFile.close(); // 关闭输出文件
    // 结束计时
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
  
    // 输出执行时长
    std::cout << "函数执行时长: " << duration << " 毫秒" << std::endl;
}