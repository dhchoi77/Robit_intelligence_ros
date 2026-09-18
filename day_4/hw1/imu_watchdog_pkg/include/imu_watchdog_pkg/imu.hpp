#pragma once

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "sensor_msgs/msg/imu.hpp"

class FakeImu : public rclcpp_lifecycle::LifecycleNode
{
public:
  FakeImu();

private:
  // lifecycle 콜백 (상태 전이 화살표마다 하나씩)
  using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override;

  // 실제 발행을 담당하는 함수 (타이머가 주기적으로 부름)
  void publish();

  // --- 멤버 변수 ---
  double rate_hz_;              // 발행 주기 (파라미터)
  double stamp_offset_sec_;     // stamp를 과거로 미는 양 (파라미터, STALE 재현용)

  rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Imu>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};