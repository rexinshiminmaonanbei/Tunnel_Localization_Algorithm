/**
* Date:  2016
* Author: Rafael Muñoz Salinas
* Description: demo application of DBoW3
* License: see the LICENSE.txt file
*/

#include <iostream>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <opencv2/core/mat.hpp>
#include "DBoW3.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "DescManip.h"

#define COLUMNOFCODEBOOK 32 //码本大小
#define DESSIZE 32  //特征维数(orb是32维，sift是128维)

using namespace DBoW3;
using namespace std;
using namespace cv;
namespace fs = std::filesystem;

struct dist {
	int dis;
	int site;
};

// 定义结构体来存储图片名称和 x 轴数据
struct ImageData {
    string image_name;
    double x_axis_data;
};

const bool EXTENDED_SURF = false;

void printProgressBar(int value, int maxValue, const std::string& label) {
    static const int barWidth = 50;
    std::cout << label << " [";
    int pos = barWidth * value / maxValue;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(pos * 100.0 / barWidth) << " %\r";
    std::cout.flush();
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
            }
        }
    } catch (const fs::filesystem_error &e) {
        cerr << "Filesystem error: " << e.what() << endl;
    } catch (const exception &e) {
        cerr << "Exception: " << e.what() << endl;
    }
}

std::vector<std::string> getSortedImageNames(const std::string& directoryPath) {
    std::vector<std::string> imageNames;

    // 检查目录是否存在
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        std::cerr << "目录不存在或不是有效目录: " << directoryPath << std::endl;
        return imageNames;
    }

    // 遍历目录中的所有文件
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            fs::path path = entry.path();
            // 检查文件是否为.png图片
            if (path.extension() == ".png") {
                std::string name = path.stem().string(); // 去掉.png后缀
                imageNames.push_back(name);
            }
        }
    }

    // 对图片名称进行排序
    std::sort(imageNames.begin(), imageNames.end());

    return imageNames;
}

//dbow3 demo.cpp函数 用于提取特征，输入图像数据集地址+所需特征，返回一个存储各图像特征的向量
vector<cv::Mat> loadFeatures1(std::vector<string> path_to_images, string descriptor = "") /*throw (std::exception)*/ {
	//select detector
	cv::Ptr<cv::Feature2D> fdetector;
	if (descriptor == "orb")        fdetector = cv::ORB::create();

	else throw std::runtime_error("Invalid descriptor");
	assert(!descriptor.empty());
	vector<cv::Mat> features;

	cout << "Extracting   features..." << endl;
	
	for (size_t i = 0; i < path_to_images.size(); ++i)
	{
		printProgressBar(i, path_to_images.size(), "Progress1");
		vector<cv::KeyPoint> keypoints;
		cv::Mat descriptors;
		std::string path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_maps/" + path_to_images[i] + ".png";
		cout << "reading image: " << path[i] << endl;
		cv::Mat image = cv::imread(path, 0);
		if (image.empty())throw std::runtime_error("Could not open image" + path);
		cout << "extracting features" << endl;
		fdetector->detectAndCompute(image, cv::Mat(), keypoints, descriptors);
		features.push_back(descriptors);
		cout << "done detecting features" << endl;
	}
	return features;
}

vector<cv::Mat> loadFeatures2(vector<string> path_to_images, string descriptor = "") /*throw (std::exception)*/ {
    // 选择检测器
    cv::Ptr<cv::Feature2D> fdetector;
    if (descriptor == "orb") {
        fdetector = cv::ORB::create();
    } else {
        throw std::runtime_error("Invalid descriptor");
    }
    assert(!descriptor.empty());

    vector<cv::Mat> features;

    cout << "Extracting features..." << endl;
	
    for (size_t i = 0; i < path_to_images.size(); ++i) {
		printProgressBar(i, path_to_images.size(), "Progress2");
        vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        std::string path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_match/" + path_to_images[i] + ".png";
        cout << "reading image: " << path << endl;
        cv::Mat image = cv::imread(path, 0);
        if (image.empty()) {
            throw std::runtime_error("Could not open image: " + path);
        }
        cout << "extracting features" << endl;
        fdetector->detectAndCompute(image, cv::Mat(), keypoints, descriptors);
        features.push_back(descriptors);
        cout << "done detecting features" << endl;
    }
    return features;
}

//dbow3 demo.cpp自带函数，修改后仅用于创建码本
void testVocCreation(const vector<cv::Mat> &features, DBoW3::Vocabulary &codebook)
{
	// branching factor and depth levels
	const int k = COLUMNOFCODEBOOK;
	const int L = 1;
	const WeightingType weight = TF_IDF;
	const ScoringType score = L1_NORM;

	DBoW3::Vocabulary voc(k, L, weight, score);  //初始化码本

	cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
	voc.create(features);   //生成码本
	cout << "... done!" << endl;

	codebook = voc;
	
	// save the vocabulary to disk
	cout << endl << "Saving vocabulary..." << endl;
	voc.save("/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/map_temp/small_voc.yml.gz");
	cout << "Done" << endl;
}

void getFiles(std::string path, std::vector<std::string>& files) {
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(path.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_type == DT_REG) { // DT_REG 表示普通文件
                files.push_back(ent->d_name);
            } else if (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                // 如果你需要递归查找子目录，可以在这里处理
                std::string subpath = path + "/" + ent->d_name;
                getFiles(subpath, files);
            }
        }
        closedir(dir);
    }
}


double euclidean_distance(cv::Mat baseImg, cv::Mat targetImg)
{
	/*
	*计算两个向量的欧氏距离
	*@param baseImg 一个向量
	*@param targetImg 一个向量
	*@return 两个向量的欧氏距离
	*/
	double sumDescriptor = 0;
	for (int i = 0; i < baseImg.cols; i++)
	{
		double numBase = abs(baseImg.at<float>(0, i));
		double numTarget = abs(targetImg.at<float>(0, i));
		sumDescriptor += pow(numBase - numTarget, 2);
	}
	double simility = sqrt(sumDescriptor);
	return simility;
}

int hammingDistance(cv::Mat baseImg, cv::Mat targetImg)
{
	/*
	*计算两个向量的汉明距离
	*@param baseImg 一个向量
	*@param targetImg 一个向量
	*@return 两个向量的汉明距离
	*/
	int sumDescriptor = 0;
	for (int i = 0; i < baseImg.cols; i++)
	{
		uint8_t numBase = baseImg.at<uint8_t>(0, i);
		uint8_t numTarget = targetImg.at<uint8_t>(0, i);
		//sumDescriptor += pow(numBase - numTarget, 2);
		if (numBase != numTarget) sumDescriptor++;
	}

	return sumDescriptor;
}


void calVLAD(const vector<cv::Mat> &features, DBoW3::Vocabulary &codebook, vector<cv::Mat> &vladBase) 
{
	/*
	*使用直接做差的方式计算残差
	*@param features 数据集中所有图片的所有特征
	*@param codebook 生成好的码本
	*@param vladBase 用于返回得到的所有图的vlad向量
	*/
	WordId wi;
	for (int i = 0; i < features.size(); ++i) {

		//初始化每张图片的vlad矩阵
		vector< cv::Mat > vladMatrix;
		for (int j = 0; j < COLUMNOFCODEBOOK; ++j) {
			cv::Mat Z = cv::Mat::zeros(1, DESSIZE, CV_32FC1);
			vladMatrix.push_back(Z);
		}

		for (int j = 0; j < features[i].rows; ++j) {
			//遍历一张图片的所有特征

			cv::Mat A = features[i](cv::Rect(0, j, DESSIZE, 1));
			wi = codebook.transform(A);   //寻找一个特征向量最近的聚类中心的id
			cout << wi << " ";
			cv::Mat central = codebook.getWord(wi);  //取最近的聚类中心向量
			cv::Mat tmpA, tmpCen;

			//将该特征向量和找到的最近聚类中心向量转换为float型用于后续计算和归一化
			A.convertTo(tmpA, CV_32FC1);
			central.convertTo(tmpCen, CV_32FC1);

			vladMatrix[wi] += (tmpA - tmpCen);
		}

		cv::Mat vlad = cv::Mat::zeros(1, 0, CV_32FC1);
		for (int j = 0; j < COLUMNOFCODEBOOK; ++j) {
			cv::hconcat(vladMatrix[j], vlad, vlad);  //将vlad矩阵展开成向量
		}
		cout << vlad << endl;

		cv::Mat vladNorm;
		vlad.copyTo(vladNorm);

		cv::normalize(vlad, vladNorm, 1, 0, cv::NORM_L2);  //对得到的vlad向量进行归一化

		vladBase.push_back(vladNorm);

		cout << vladNorm << endl;
	}


}

void calVLADHamming(const vector<cv::Mat> &features, DBoW3::Vocabulary &codebook, vector<cv::Mat> &vladBase)
{
	/*
	*以bit为单位使用汉明距离计算残差
	*@param features 数据集中所有图片的所有特征
	*@param codebook 生成好的码本
	*@param vladBase 用于返回得到的所有图的vlad向量
	*/

	WordId wi;
	for (int i = 0; i < features.size(); ++i) {
		//遍历每张图片

		//初始化vlad矩阵和vlad位矩阵分别用于返回最终的vlad矩阵和保存每bit的统计结果
		vector< cv::Mat > vladBitMatrix;
		vector <cv::Mat > vladMatrix;
		for (int j = 0; j < COLUMNOFCODEBOOK; ++j) {
			cv::Mat Z = cv::Mat::zeros(1, DESSIZE * sizeof(uint8_t) * 8, CV_32FC1);
			cv::Mat X = cv::Mat::zeros(1, DESSIZE * sizeof(uint8_t) * 8, CV_8U);
			vladBitMatrix.push_back(Z);
			vladMatrix.push_back(X);
		}

		int sum = 0;
		for (int j = 0; j < features[i].rows; ++j) {
			//遍历每张图片的所有特征


			cv::Mat A = features[i](cv::Rect(0, j, DESSIZE, 1));
			wi = codebook.transform(A);
			cv::Mat central = codebook.getWord(wi);
		
			
			int bitCur = 0;
			for (int k = 0; k < A.cols; ++k) {
				//取一张图片的一个特征向量和最近的聚类中心向量
				uint8_t a = A.at<uint8_t>(0, k); 
				uint8_t cen = central.at<uint8_t>(0, k);
				uint8_t tmp = a ^ cen; //遍历向量中每个元素并进行异或操作用于统计不同位数的个数
				uint8_t mask = 0x01;
				for (int l = 0; l < 8; ++l) {
					if (tmp & mask) {
						vladBitMatrix[wi].at<float>(0, bitCur)++;  //如果特征向量与聚类中心不同，则vlad位矩阵该位置+1
						bitCur++;
						sum++;  //统计一张图片中所有不同位数的个数
					}
					else {
						bitCur++;
					}
					mask = mask << 1;
				}
			}
		}
		float thr = sum / (COLUMNOFCODEBOOK * DESSIZE * 8.0f);  //用总的不同位数的个数除以所有位数计算一个阈值thr

		for (int j = 0; j < COLUMNOFCODEBOOK; ++j) {
			int bitCur = 0;
			for (int k = 0; k < DESSIZE; ++k) {
				//uint8_t mask = 0x01;
				for (int l = 0; l < 8; l++) {
					if (vladBitMatrix[j].at<uint8_t>(0, bitCur) > thr) {
						//如果vlad位矩阵中的不同位数累积结果大于阈值thr，则改位记为1，否则记为0
						vladMatrix[j].at<uint8_t>(0, bitCur) = 1;

					}
					//mask = mask << 1;
					bitCur++;
				}
			}
		}

		//将得到的vlad矩阵扩展成为向量
		cv::Mat vlad = cv::Mat::zeros(1, 0, CV_8U);
		for (int j = 0; j < COLUMNOFCODEBOOK; ++j) {
			cv::hconcat(vladMatrix[j], vlad, vlad);
		}
		cout << vlad << endl;

		vladBase.push_back(vlad);
	}


}

bool vecCmp(const dist &a, const dist &b) {
	     return a.dis < b.dis;
}

void processImages(const std::string& map_temp_path, const std::string& match_temp_path, std::ofstream& outfile) {
    std::vector<std::string> files = getSortedImageNames(map_temp_path);
    std::vector<std::string> retFiles = getSortedImageNames(match_temp_path);

    // 提取特征
    std::string descriptor = "orb";
    std::vector<cv::Mat> features = loadFeatures1(files, descriptor);
    std::vector<cv::Mat> retFeatures = loadFeatures2(retFiles, descriptor);

    // 建立码本
    DBoW3::Vocabulary voc;
    testVocCreation(features, voc);

    // 计算图片集和查询集每张图片的vlad向量
    std::vector<cv::Mat> vladBase;
    std::vector<cv::Mat> retVladBase;
    calVLADHamming(features, voc, vladBase);
    calVLADHamming(retFeatures, voc, retVladBase);

    // 测试、评价
    for (int i = 0; i < retVladBase.size(); ++i) {
        std::vector<dist> disVec; // 该向量用于对测试结果进行存储和排序
        for (int j = 0; j < vladBase.size(); ++j) {
            int dis = hammingDistance(vladBase[j], retVladBase[i]); // 计算图片集中每张图片与待查询图片的vlad向量的汉明距离
            dist tmp;
            tmp.dis = dis;
            tmp.site = j;
            disVec.push_back(tmp);
        }

        // 对结果进行排序、输出结果
        sort(disVec.begin(), disVec.end(), vecCmp);
        outfile << retFiles[i] << " " << files[disVec[1].site] << std::endl;
    }
}

int main(int argc, char **argv)
{
		// 读取图片集和查询集所有图片地址
		// std::string filePath = "/home/rxsmmnb/Tunnel_pictures/test";
		// std::string filePath = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_match";
		std::string filePath = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_maps";
    	std::string retFilePath = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/viterbi_wall_match";
    	std::string map_temp_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/map_temp";
    	std::string match_temp_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/match_temp";
		std::string map_txt_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/map_pose.txt";
    	std::string match_txt_path = "/media/rxsmmnb/dying_gull/data/pictures_query_behind/txt文件/query_pose.txt";
    	std::string output_path = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/query_results.txt";

		std::vector<ImageData> map_data_array = readImageDataFromFile(map_txt_path);
    	std::vector<ImageData> match_data_array = readImageDataFromFile(match_txt_path);

		for (int i = 0; i < match_data_array.size(); ++i) {
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
			std::string match_path = retFilePath + "/" + match_data_array[i].image_name + ".png";
			copyImage(match_path, match_temp_path);
			for (int j =0; j < real_size; ++j) {
				std::string image_path = filePath + "/" + map_data_array[startIndex + j].image_name + ".png";
				copyImage(image_path, map_temp_path);
			}
			ofstream outfile(output_path, std::ios_base::app); // 以追加模式打开文件
			if (!outfile.is_open()) {
				cerr << "Could not open output file " << output_path << endl;
				exit(0);
			}
			processImages(map_temp_path, match_temp_path, outfile);

			removeAllFilesInFolder(map_temp_path);
			removeAllFilesInFolder(match_temp_path);
		// vector<string> files;
		// vector<string> retFiles;
		// string results_filename = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/build/DBOW3/utils/query_results.txt";
		// ofstream outfile(results_filename, std::ios_base::app); // 以追加模式打开文件
		// if (!outfile.is_open()) {
		// 	cerr << "Could not open output file " << results_filename << endl;
		// 	exit(0);
		// }
		// files = getSortedImageNames(map_temp_path);
		// retFiles = getSortedImageNames(match_temp_path);

		// //提取特征
		// string descriptor = "orb";
		// vector<cv::Mat> features = loadFeatures1(files, descriptor);
		// vector<cv::Mat> retFeatures = loadFeatures2(retFiles, descriptor);

		// //建立码本
		// DBoW3::Vocabulary voc;
		// testVocCreation(features, voc);


		// //计算图片集和查询集每张图片的vlad向量
		// vector<cv::Mat> vladBase;
		// vector <cv::Mat> retVladBase;
		// calVLADHamming(features, voc, vladBase);
		// calVLADHamming(retFeatures, voc, retVladBase);

		// //测试、评价
		// for (int i = 0; i < retVladBase.size(); ++i) {
		// 	vector<struct dist> disVec; //该向量用于对测试结果进行存储和排序
		// 	for (int j = 0; j < vladBase.size(); ++j) {
				
		// 		int dis = hammingDistance(vladBase[j], retVladBase[i]); //计算图片集中每张图片与待查询图片的vlad向量的汉明距离
		// 		dist tmp;
		// 		tmp.dis = dis;
		// 		tmp.site = j;
		// 		disVec.push_back(tmp);
		// 	}

		// 	//对结果进行排序、输出结果
		// 	sort(disVec.begin(), disVec.end(), vecCmp);
		// 	// std::cout << retFiles[i] << " " << retFiles[disVec[1].site] << endl;
		// 	outfile << retFiles[i] << " " << retFiles[disVec[1].site] << endl;
		}

	return 0;
}
