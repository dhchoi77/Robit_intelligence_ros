cmake_minimum_required(VERSION 3.8)
project(imu_watchdog_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(lifecycle_msgs REQUIRED)

include_directories(include)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# --- 실행파일 1: fake_imu ---
add_executable(fake_imu
  src/imu.cpp
  src/imu_main.cpp)
ament_target_dependencies(fake_imu
  rclcpp rclcpp_lifecycle sensor_msgs)

# --- 실행파일 2: imu_watchdog ---
add_executable(imu_watchdog
  src/watch_dog.cpp
  src/watch_dog_main.cpp)
ament_target_dependencies(imu_watchdog
  rclcpp rclcpp_lifecycle sensor_msgs lifecycle_msgs)

install(TARGETS
  fake_imu
  imu_watchdog
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch          # ← launch 폴더 만든 뒤에 (CP8)
  DESTINATION share/${PROJECT_NAME})
  
ament_package()
