cmake_minimum_required(VERSION 3.8)
project(i2c_multiplexer_driver)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# Find libi2cd (must be installed on the system)
find_library(I2CD_LIB i2cd REQUIRED)
find_path(I2CD_INCLUDE i2cd.h REQUIRED)

# VL53L5CX ULD driver source files (STSW-IMG023)
set(VL53L5CX_SOURCES
  src/vl53l5cx_api.c
  src/platform.c
)

# VL53L5CX ULD include directory (contains vl53l5cx_api.h, vl53l5cx_buffers.h, platform.h)
set(VL53L5CX_INCLUDE src)

add_executable(i2c_multiplexer_driver
  src/i2c_multiplexer_driver.cpp
  ${VL53L5CX_SOURCES}
)

ament_target_dependencies(i2c_multiplexer_driver rclcpp std_msgs)

target_include_directories(i2c_multiplexer_driver PRIVATE
  ${I2CD_INCLUDE}
  ${VL53L5CX_INCLUDE}
)

target_link_libraries(i2c_multiplexer_driver ${I2CD_LIB})

install(TARGETS
  i2c_multiplexer_driver
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()