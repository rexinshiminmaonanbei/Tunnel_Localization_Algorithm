cmake_minimum_required(VERSION 3.10)
project(comparision)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(G2O_DIR /home/rxsmmnb/lib/g2o-master/cmake_modules)

list(APPEND CMAKE_MODULE_PATH /home/rxsmmnb/lib/g2o-master/cmake_modules)
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rosbag
  cv_bridge
  sensor_msgs
  std_msgs
)

find_package(OpenCV 4.8.0 REQUIRED)
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES viterbi
  CATKIN_DEPENDS roscpp std_msgs
  DEPENDS OpenCV
)

# 添加 G2O 库的查找
find_package(G2O REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem iostreams)

find_package(VTK REQUIRED)
find_package(PCL REQUIRED)
include(${VTK_USE_FILE})

add_executable(comparision
  src/main.cpp
  src/feature_extraction.cpp
  src/match.cpp
  src/coarse_handle.cpp
  src/match_handle.cpp
  src/verify.cpp
  src/LoFTR.cpp
  src/ORB.cpp
)

# 为 PCL 和 VTK 设置编译定义
include_directories(${PCL_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR})

target_include_directories(comparision PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${G2O_INCLUDE_DIRS}
)

target_link_libraries(comparision
  ${OpenCV_LIBS}
  ${catkin_LIBRARIES}
  ${PCL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${EIGEN3_LIBS}
  ${G2O_LIBRARIES}
)

target_link_libraries(comparision g2o_core g2o_stuff)