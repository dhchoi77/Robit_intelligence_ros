#include "imu_watchdog_pkg/imu.hpp"
#include <chrono>     // 1s, 500ms 같은 주기 리터럴
#include <memory>
using namespace std::chrono_literals;

FakeImu::FakeImu()
: rclcpp_lifecycle::LifecycleNode("fake_imu")
{
  // 파라미터 선언 + 기본값. 실제 값 읽기는 on_configure에서.
  this->declare_parameter<double>("rate_hz", 50.0);
  this->declare_parameter<double>("stamp_offset_sec", 0.0);
}

FakeImu::CallbackReturn
FakeImu::on_configure(const rclcpp_lifecycle::State &)
{
  rate_hz_          = this->get_parameter("rate_hz").as_double();
  stamp_offset_sec_ = this->get_parameter("stamp_offset_sec").as_double();

  // 파라미터 검증 → 잘못되면 FAILURE (Inactive로 못 올라감)
  if (rate_hz_ <= 0.0) {
    RCLCPP_ERROR(get_logger(), "rate_hz는 0보다 커야 합니다 (받은 값: %.2f)", rate_hz_);
    return CallbackReturn::FAILURE;
  }

  // LifecyclePublisher 생성 — Active일 때만 실제로 나감
  pub_ = this->create_publisher<sensor_msgs::msg::Imu>(
    "imu", rclcpp::SensorDataQoS());

  RCLCPP_INFO(get_logger(), "configured: rate=%.1fHz, offset=%.2fs",
              rate_hz_, stamp_offset_sec_);
  return CallbackReturn::SUCCESS;
}

// Inactive → Active : 발행 시작
FakeImu::CallbackReturn
FakeImu::on_activate(const rclcpp_lifecycle::State & state)
{
  LifecycleNode::on_activate(state);   // ← pub_를 실제로 켜는 줄

  auto period = std::chrono::duration<double>(1.0 / rate_hz_);
  timer_ = this->create_wall_timer(period, [this]() { publish(); });

  RCLCPP_INFO(get_logger(), "activated: /imu 발행 시작");
  return CallbackReturn::SUCCESS;
}

// Active → Inactive : 발행 중단 (= 워치독의 [NO DATA] 재현)
FakeImu::CallbackReturn
FakeImu::on_deactivate(const rclcpp_lifecycle::State & state)
{
  timer_.reset();                        // 타이머부터 없앰 → publish() 호출 멈춤
  LifecycleNode::on_deactivate(state);   // pub_ 비활성화

  RCLCPP_INFO(get_logger(), "deactivated: /imu 발행 중단");
  return CallbackReturn::SUCCESS;
}

// Inactive → Unconfigured : 자원 반납
FakeImu::CallbackReturn
FakeImu::on_cleanup(const rclcpp_lifecycle::State &)
{
  timer_.reset();
  pub_.reset();
  RCLCPP_INFO(get_logger(), "cleaned up");
  return CallbackReturn::SUCCESS;
}

// 어디서든 → Finalized
FakeImu::CallbackReturn
FakeImu::on_shutdown(const rclcpp_lifecycle::State &)
{
  timer_.reset();
  pub_.reset();
  RCLCPP_INFO(get_logger(), "shutdown");
  return CallbackReturn::SUCCESS;
}

// 타이머가 주기적으로 부르는 함수
void FakeImu::publish()
{
  sensor_msgs::msg::Imu msg;
  double offset = this->get_parameter("stamp_offset_sec").as_double();
  msg.header.stamp = this->now() - rclcpp::Duration::from_seconds(offset);  msg.header.frame_id = "imu_link";
  // 가속도·각속도 필드는 0으로 둠 (이 과제는 stamp만 중요)

  pub_->publish(msg);  // ↓ 아래 설명 참고 — 보통은 그냥 pub_->publish(msg);
}
