cmake_minimum_required(VERSION 3.8)
project(my_cpp_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# 실행 파일 빌드 설정
add_executable(my_pub src/publisher.cpp)
ament_target_dependencies(my_pub rclcpp std_msgs)

add_executable(my_sub src/subscriber.cpp)
ament_target_dependencies(my_sub rclcpp std_msgs)

install(TARGETS my_pub my_sub
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)
ament_package()