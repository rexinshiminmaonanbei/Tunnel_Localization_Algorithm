# coding:utf-8
import os
import cv2
import torch
import numpy as np
import k_means
from tqdm import tqdm
from typing import Union
from src.loftr import LoFTR, default_cfg

# 定义一个类，方便调用
class loftrInfer(object):

    def __init__(self, model_path="weights/indoor_ds.ckpt"):
        self.matcher = LoFTR(config=default_cfg)
        self.matcher.load_state_dict(torch.load(model_path, weights_only=True)['state_dict'])
        self.matcher = self.matcher.eval()

    def _infer_run(self, img0_raw: np.ndarray, img1_raw: np.ndarray) -> Union[np.ndarray, bool]:
        img0 = torch.from_numpy(img0_raw[None, None]).float() / 255.
        img1 = torch.from_numpy(img1_raw[None, None]).float() / 255.
        batch = {'image0': img0, 'image1': img1}

        with torch.no_grad():
            self.matcher(batch)
            mkpts0 = batch['mkpts0_f'].cpu().numpy()
            mkpts1 = batch['mkpts1_f'].cpu().numpy()
            mconf = batch['mconf'].cpu().numpy()

        if mconf.shape[0] < 4:
            return False
        mconf = mconf[:, np.newaxis]
        np_result = np.hstack((mkpts0, mkpts1, mconf))
        sorted_indices = np.argsort(-mconf[:, 0])
        return np_result[sorted_indices]

    def _points_filter(self, np_result, lenth=200, use_kmeans=True):
        lenth = min(lenth, np_result.shape[0])
        if lenth < 4: lenth = 4

        mkpts0 = np_result[:lenth, 2:4].copy()
        mkpts1 = np_result[:lenth, 2:4].copy()

        if use_kmeans:
            use_mkpts0, _ = k_means.get_group_center(mkpts0, mkpts1)
            if use_mkpts0.shape[0] < 4:
                return use_mkpts0.shape[0]
            return use_mkpts0.shape[0]
        return use_mkpts0.shape[0]

    def run(self, img0_bgr, img1_bgr, lenth=200, use_kmeans=True):
        img0_bgr = cv2.resize(img0_bgr, (640, 480))
        img1_bgr = cv2.resize(img1_bgr, (640, 480))

        img0_raw = cv2.cvtColor(img0_bgr, cv2.COLOR_BGR2GRAY)
        img1_raw = cv2.cvtColor(img1_bgr, cv2.COLOR_BGR2GRAY)

        np_result = self._infer_run(img0_raw, img1_raw)
        if np_result is False:
            return 0
        num_points = self._points_filter(np_result, lenth=lenth, use_kmeans=use_kmeans)

        return num_points

# 得到待匹配图片的x坐标
def get_value_between_spaces(search_string, txt_file):
    
    if not os.path.exists(txt_file):
        print(f"文本文件不存在: {txt_file}")
        return None

    with open(txt_file, 'r') as file:
        for line in file:
            if search_string in line:
                # 切分行数据
                parts = line.split()
                
                if len(parts) > 2:  # 确保有足够的元素
                    # 获取第一个和第二个空格之间的数据
                    value_str = parts[1]  # parts[0] 是 search_string，parts[1] 是所需值
                    try:
                        return float(value_str)
                    except ValueError:
                        print(f"无法将 '{value_str}' 转换为浮点数。")
                        return None

    print(f"未找到字符串 '{search_string}' 在文件中的任何行。")
    return None

# 找到与 x 坐标绝对差值最小的值的索引
def find_closest_index(x, txt_file):

    if not os.path.exists(txt_file):
        print(f"文本文件不存在: {txt_file}")
        return None

    values = []
    
    # 读取文件内容并提取第一个和第二个空格之间的数据
    with open(txt_file, 'r') as file:
        for line in file:
            parts = line.split()
            if len(parts) > 2:  # 确保有足够的元素
                try:
                    value = float(parts[1])  # 将第二个空格之后的值转为 double（浮点型）
                    values.append(value)
                except ValueError:
                    print(f"无法将 '{parts[1]}' 转换为浮点数。")

    # 查找与 x 坐标绝对差值最小的值及其索引
    if not values:
        print("未找到有效数据。")
        return None

    closest_index = 0
    min_difference = float('inf')  # 初始化为无穷大

    for index, value in enumerate(values):
        difference = abs(value - x)
        if difference < min_difference:
            min_difference = difference
            closest_index = index

    return closest_index

# 获得索引
def get_combined_image_indices(folder1, txt1, folder2, txt2):
    # 检查文件夹是否存在
    if not os.path.exists(folder1):
        print(f"文件夹不存在: {folder1}")
        return []
    if not os.path.exists(folder2):
        print(f"文件夹不存在: {folder2}")
        return []

    # 读取文件夹中的图片名称，并去掉 .png 后缀
    query_names = []
    for filename in os.listdir(folder1):
        if filename.endswith('.png'):  # 验证文件类型是否为 PNG
            name_without_extension = os.path.splitext(filename)[0]  # 去掉后缀
            query_names.append(name_without_extension)
    
    # 按照名称排序
    query_names.sort()  # 使用 sort() 方法对列表进行排序
    # 创建一个数组来存储索引
    combined_indices = []

     # 遍历所有 query_names，获取并打印 x 坐标
    for query in query_names:
        x_value = get_value_between_spaces(query, txt1)  # 获取对应的 x 坐标
        if x_value is not None:  # 确保 x_value 不是 None
            index = find_closest_index(x_value, txt2)  # 找到与 x 坐标绝对差值最小的值的索引
            if index is not None:  # 确保 index 不是 None
                combined_indices.append(index)  # 将索引添加到数组中

    return combined_indices

if __name__ == "__main__":
    testInfer = loftrInfer(model_path="weights/indoor_ds.ckpt")

    query_folder = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_match"
    map_folder = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_maps"
    query_pose = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/query_pose.txt"
    map_pose = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/map_pose.txt"
    output_file = 'matching_results.txt'
    
    # 获取index和相关信息
    indices = get_combined_image_indices(query_folder, query_pose, map_folder, map_pose)

    # 读取待匹配图片文件夹中的所有图像
    query_names = sorted([os.path.splitext(filename)[0] for filename in os.listdir(query_folder) 
                   if filename.endswith('.png')])
    query_pths = sorted([os.path.join(query_folder, f) for f in os.listdir(query_folder)
                  if f.endswith(('.png'))])  # 构造待匹配图像路径

    # 读取地图文件夹中的所有图像
    map_pths = sorted([os.path.join(map_folder, f) for f in os.listdir(map_folder)
                  if f.endswith(('.png'))])  # 过滤出图像文件

    # 确保有待匹配和地图图像
    if not query_pths or not map_pths:
        print("待匹配图片文件夹或地图文件夹为空。")
        exit(1)

    # 处理每张待匹配图像并更新进度条
    total_pairs = len(query_pths) * 300  # 每张查询图像可能匹配300张地图图像
    with tqdm(total=total_pairs, desc="匹配进度", unit="图像") as pbar:
        num_points_list = []  # 存储每个查询的匹配结果
        
        for idx, img0_pth in enumerate(query_pths):
            img0_bgr = cv2.imread(img0_pth)
            if img0_bgr is None:
                print(f"未能读取待匹配图像: {img0_pth}")
                continue  # 跳过当前图像

            result_for_current_image = []  # 存储当前待匹配图片的匹配结果
            
            # 找到当前 index 对应的地图图像范围
            if idx < len(indices):
                center_index = indices[idx]  # 当前索引对应的地图图像索引
                
                # 确定开始和结束索引
                if center_index < 150:
                    start_index = 0
                    end_index = min(len(map_pths), 300)  # 最多匹配前300张
                elif center_index > len(map_pths) - 150:
                    start_index = max(0, len(map_pths) - 300)  # 最多匹配最后300张
                    end_index = len(map_pths)
                else:
                    start_index = center_index - 150
                    end_index = center_index + 150
                
                # 匹配指定范围内的地图图像
                for img1_pth in map_pths[start_index:end_index]:
                    img1_bgr = cv2.imread(img1_pth)
                    if img1_bgr is None:
                        print(f"未能读取地图图像: {img1_pth}")
                        continue
                    
                    # 调用模型进行匹配并记录结果
                    num_points = testInfer.run(img0_bgr, img1_bgr, lenth=200, use_kmeans=True)
                    result_for_current_image.append(num_points)

                    # 更新进度条
                    # pbar.set_postfix({"当前查询图像": os.path.basename(img0_pth), "当前匹配图像": os.path.basename(img1_pth)})
                    pbar.update(1)  # 更新进度条，每处理一对图像更新一次

            # 将该图像的匹配结果添加到 num_points_list 中
            num_points_list.append(result_for_current_image)  # 每个结果作为一个单独的列表存储

            print(f"查询图像: {query_names[idx]} 匹配到的点数量: {result_for_current_image}")

    # 打印所有匹配结果并将其写入到文件
    output_file = 'matching_results.txt'  # 输出文件名
    with open(output_file, 'w') as f:
        for i, results in enumerate(num_points_list):
            line = f"{query_names[i]} {results}\n"  # 构造要写入的行
            f.write(line)  # 写入文件