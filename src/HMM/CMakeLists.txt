cmake_minimum_required(VERSION 3.10)
project(HMM)

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

add_executable(HMM
  src/main.cpp
  src/orbmatch.cpp
  src/viterbi.cpp
  src/coarse_handle.cpp
  src/ViterbiMatch.cpp
  src/verify.cpp
  src/precise_position.cpp
  src/3D_2D.cpp
  src/LoFRT.cpp
  src/local_ORB.cpp
  src/make_data.cpp
  src/2D-2D.cpp
)

# 为 PCL 和 VTK 设置编译定义
include_directories(${PCL_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR})

target_include_directories(HMM PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${G2O_INCLUDE_DIRS}
)

target_link_libraries(HMM
  ${OpenCV_LIBS}
  ${catkin_LIBRARIES}
  ${PCL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${EIGEN3_LIBS}
  ${G2O_LIBRARIES}
)

target_link_libraries(HMM g2o_core g2o_stuff)