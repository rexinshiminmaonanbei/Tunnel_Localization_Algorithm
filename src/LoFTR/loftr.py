import os
import cv2
import torch
import numpy as np
from tqdm import tqdm
from src.loftr import LoFTR, default_cfg


# LoFTR 匹配
def find_best_match(base_image_path, query_image_names):
    # 初始化 LoFTR
    matcher = LoFTR(config=default_cfg)
    matcher.load_state_dict(torch.load("weights/indoor_ds.ckpt", weights_only=True)['state_dict'])
    matcher = matcher.eval()
    # matcher = matcher.cuda().eval()

    # 加载基准图像
    base_img_bgr = cv2.imread(base_image_path)

    # 存储每张查询图像的匹配点数量
    match_counts = {}

    # 循环遍历每张待匹配图像
    for query_image_name in query_image_names:
        query_image_path = os.path.join(os.path.dirname(base_image_path), query_image_name)
        query_img_bgr = cv2.imread(query_image_path)

        if query_img_bgr is None:
            print(f"无法加载图像: {query_image_path}")
            continue

        # 图像预处理
        base_img_gray = cv2.cvtColor(base_img_bgr, cv2.COLOR_BGR2GRAY)
        query_img_gray = cv2.cvtColor(query_img_bgr, cv2.COLOR_BGR2GRAY)

        # 准备输入 batch
        base_img_tensor = torch.from_numpy(base_img_gray[None, None]).float() / 255.
        query_img_tensor = torch.from_numpy(query_img_gray[None, None]).float() / 255.
        batch = {'image0': base_img_tensor, 'image1': query_img_tensor}

        # 进行推理
        with torch.no_grad():
            matcher(batch)
            mkpts_base = batch['mkpts0_f'].cpu().numpy()  # 基准图像的匹配点

        # 记录匹配点数量
        match_counts[query_image_name] = len(mkpts_base)

    # 找到匹配点最多的图片
    if match_counts:
        best_match_image = max(match_counts, key=match_counts.get)
        best_match_index = query_image_names.index(best_match_image)
        return best_match_index
    else:
        print("没有找到任何匹配的图像。")
        return None

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


# 获得文件夹中的图片名称，并去掉后缀
def get_image_names_without_extension(folder_path):

    image_names = []
    
    # 确保文件夹路径存在
    if not os.path.exists(folder_path):
        print(f"文件夹不存在: {folder_path}")
        return image_names

    # 遍历文件夹中的所有文件
    for filename in os.listdir(folder_path):
        # 检查文件后缀是否为图片格式
        if filename.endswith(('.png')):
            # 去掉后缀并添加到数组中
            name_without_extension = os.path.splitext(filename)[0]
            image_names.append(name_without_extension)

    return image_names


# 运行匹配的函数
def run_matching(query_folder, map_folder, query_pose, map_pose, result_folder):
    # 获取索引
    indices = get_combined_image_indices(query_folder, query_pose, map_folder, map_pose)

    # 获取图片名称，并去掉后缀
    map_names = get_image_names_without_extension(map_folder)
    match_names = get_image_names_without_extension(query_folder)

    # 排序图片名称
    map_names.sort()
    match_names.sort()

    # 存储所有结果
    results = []
    
    # 读取地图文件夹中的所有图像路径
    map_pths = sorted([os.path.join(map_folder, f) for f in os.listdir(map_folder)
                       if f.endswith(('.png'))])

    # 使用 tqdm 显示进度条
    with tqdm(total=len(match_names), desc="匹配进度", unit="image") as pbar:
        for idx, img0_name in enumerate(match_names):
            img0_pth = os.path.join(query_folder, img0_name + '.png')
            img0_bgr = cv2.imread(img0_pth)
            if img0_bgr is None:
                print(f"未能读取待匹配图像: {img0_pth}")
                pbar.update(1)  # 更新进度条
                continue  # 跳过当前图像

            # 找到当前 index 对应的地图图像范围
            if idx < len(indices):
                center_index = indices[idx]  # 当前索引对应的地图图像索引
                
                # 确定开始和结束索引
                if center_index < 30:
                    start_index = 0
                    end_index = min(len(map_names), 60)  # 最多匹配前60张
                elif center_index > len(map_names) - 30:
                    start_index = max(0, len(map_names) - 60)  # 最多匹配最后60张
                    end_index = len(map_names)
                else:
                    start_index = center_index - 30
                    end_index = center_index + 30
                
                # 匹配指定范围内的地图图像
                img1_pth_vec = []
                for img1_pth in map_pths[start_index:end_index]:
                    img1_bgr = cv2.imread(img1_pth)
                    if img1_bgr is None:
                        print(f"未能读取地图图像: {img1_pth}")
                        continue
                    img1_pth_vec.append(img1_pth)

                # 进行匹配
                final_index = find_best_match(img0_pth, img1_pth_vec)
                if final_index is not None:
                    return_index = start_index + final_index
                    results.append(return_index)

            # 更新进度条
            pbar.update(1)

    # 将结果写入文件，先清空文件
    with open(result_folder, 'w') as f:
        for i in range(len(results)):
            f.write(f"{match_names[i]} {map_names[results[i]]}\n")

    print(f"结果已写入到 {result_folder} 文件中。")


# 使用示例
if __name__ == "__main__":
    front_query_folder = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_match/"
    front_map_folder = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_maps/"
    front_query_pose = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/query_pose.txt"
    front_map_pose = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/map_pose.txt"
    front_result_folder = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/LoFTR结果/LoFTR_results.txt"

    behind_query_folder = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_match/"
    behind_map_folder = "/home/rxsmmnb/Tunnel_pictures/viterbi_wall_maps/"
    behind_query_pose = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/query_pose.txt"
    behind_map_pose = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/map_pose.txt"
    behind_result_folder = "/home/rxsmmnb/workspace/Tunnel-SLAM_ws/txt文件/对比txt/LoFTR结果/LoFTR_results.txt"


    run_matching(front_query_folder, front_map_folder, front_query_pose, front_map_pose, front_result_folder)
    run_matching(behind_query_folder, behind_map_folder, behind_query_pose, behind_map_pose, behind_result_folder)

    # indices = get_combined_image_indices(query_folder, query_pose, map_folder, map_pose)
    # map_names = get_image_names_without_extension(map_folder)
    # match_names = get_image_names_without_extension(query_folder)
    # map_names.sort()
    # match_names.sort()
    # results = []
    # map_pths = sorted([os.path.join(map_folder, f) for f in os.listdir(map_folder)
    #                    if f.endswith(('.png'))])

    # for idx, img0_name in tqdm(enumerate(match_names), total=len(match_names), desc="匹配进度"):
    #     img0_pth = os.path.join(query_folder, img0_name + '.png')
    #     img0_bgr = cv2.imread(img0_pth)
    #     if img0_bgr is None:
    #             print(f"未能读取待匹配图像: {img0_pth}")
    #             continue  # 跳过当前图像

    #     # 找到当前 index 对应的地图图像范围
    #     if idx < len(indices):
    #         center_index = indices[idx]  # 当前索引对应的地图图像索引
                
    #         # 确定开始和结束索引
    #         if center_index < 30:
    #             start_index = 0
    #             end_index = min(len(map_names), 60)  # 最多匹配前60张
    #         elif center_index > len(map_names) - 30:
    #             start_index = max(0, len(map_names) - 60)  # 最多匹配最后60张
    #             end_index = len(map_names)
    #         else:
    #             start_index = center_index - 30
    #             end_index = center_index + 30
            
                
    #         # print(f"当前匹配图像: {img0_name}, 地图图像范围: {start_index} - {end_index}")
    #         # 匹配指定范围内的地图图像
    #         img1_pth_vec = []
    #         for img1_pth in map_pths[start_index : end_index]:
    #             # print(img0_pth)
    #             img1_bgr = cv2.imread(img1_pth)
    #             if img1_bgr is None:
    #                 print(f"未能读取地图图像: {img1_pth}")
    #                 continue
    #             img1_pth_vec.append(img1_pth)

    #         final_index = find_best_match(img0_pth, img1_pth_vec)
    #         return_index = start_index + final_index
    #         results.append(return_index)

    # with open(result_folder, 'w') as f:
    #     for i in range(len(results)):
    #         print(f"{match_names[i]} {map_names[results[i]]}")
    #         f.write(match_names[i] + ' ' + map_names[results[i]] + '\n')

