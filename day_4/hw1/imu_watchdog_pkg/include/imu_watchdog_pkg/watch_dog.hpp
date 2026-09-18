#pragma once

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "lifecycle_msgs/srv/get_state.hpp"

class ImuWatchdog : public rclcpp_lifecycle::LifecycleNode
{
public:
  ImuWatchdog();

private:
  using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  // lifecycle 콜백
  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override;

  // 콜백 함수
  void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg);  // 메시지 도착 시
  void check();                                                   // 주기적으로 age 검사
  void state_timer_callback();   // ← 추가: 1초마다 fake_imu 상태 물어보기
  // --- 파라미터 ---
  double timeout_sec_;      // 이 시간 넘게 소식 없거나 stamp가 낡으면 문제
  double check_rate_hz_;    // check()를 부르는 주기

  // --- 감시 상태 ---
  rclcpp::Time last_arrival_;   // 마지막으로 메시지가 "도착한" 시각 → [NO DATA] 판단
  rclcpp::Time last_stamp_;     // 마지막 메시지의 header.stamp → [STALE] 판단
    enum class Status { NoData, Stale, Ok };   // ← 추가

  // ...(기존 멤버들)...
  bool received_ = false;                 // 한 번이라도 받았나 (시작 직후 오판 방지)
  Status prev_status_ = Status::Ok;       // ← healthy_ 대신. 직전 상태(엣지 감지용)
  // --- 통신 ---
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr sub_;   // /imu 구독 (일반!)
  rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Imu>::SharedPtr pub_;  // /imu_checked 재발행
  rclcpp::TimerBase::SharedPtr check_timer_;                     // check() 주기 타이머
  rclcpp::Client<lifecycle_msgs::srv::GetState>::SharedPtr state_client_;  // ← 추가
  rclcpp::TimerBase::SharedPtr state_timer_;                              // ← 추가
  rclcpp::CallbackGroup::SharedPtr client_group_;       // ← 추가
  rclcpp::CallbackGroup::SharedPtr state_timer_group_;  // ← 추가
};