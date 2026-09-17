#include "rclcpp/rclcpp.hpp"
#include "imu_watchdog_pkg/imu.hpp"          // watch_dog_main.cpp에선 watch_dog.hpp

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::executors::SingleThreadedExecutor exec;
  auto node = std::make_shared<FakeImu>();
  exec.add_node(node->get_node_base_interface());  // ← 일반 노드와 다른 점!
  exec.spin();
  rclcpp::shutdown();
  return 0;
}
