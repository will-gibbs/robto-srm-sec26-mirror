cmake_minimum_required(VERSION 3.8)
project(navigator)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(pcl_ros REQUIRED)
find_package(PCL REQUIRED)

add_executable(nav_node src/nav.cpp)

# Link ROS 2 dependencies
ament_target_dependencies(nav_node 
    rclcpp 
    rclcpp_action 
    nav2_msgs 
    sensor_msgs 
    pcl_conversions 
    pcl_ros
)

# Link the actual PCL system libraries
target_link_libraries(nav_node ${PCL_LIBRARIES})

# Installation
install(TARGETS nav_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch config scripts DESTINATION share/${PROJECT_NAME})

install(PROGRAMS
  scripts/dummy_odom.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()