#include "imu_watchdog_pkg/watch_dog.hpp"

#include <chrono>
#include <memory>

ImuWatchdog::ImuWatchdog()
: rclcpp_lifecycle::LifecycleNode("imu_watchdog")
{
  this->declare_parameter<double>("timeout_sec", 0.5);
  this->declare_parameter<double>("check_rate_hz", 10.0);
}

// Unconfigured → Inactive : 파라미터 검증 + 퍼블리셔 준비
ImuWatchdog::CallbackReturn
ImuWatchdog::on_configure(const rclcpp_lifecycle::State &)
{
  timeout_sec_   = this->get_parameter("timeout_sec").as_double();
  check_rate_hz_ = this->get_parameter("check_rate_hz").as_double();

  // ★ 과제 필수: 파라미터 0 이하면 FAILURE
  if (timeout_sec_ <= 0.0 || check_rate_hz_ <= 0.0) {
    RCLCPP_ERROR(get_logger(),
      "잘못된 파라미터: timeout_sec=%.3f, check_rate_hz=%.3f (둘 다 0보다 커야 함)",
      timeout_sec_, check_rate_hz_);
    return CallbackReturn::FAILURE;
  }

  pub_ = this->create_publisher<sensor_msgs::msg::Imu>(
    "imu_checked", rclcpp::SensorDataQoS());

  client_group_ = this->create_callback_group(
    rclcpp::CallbackGroupType::MutuallyExclusive);

  state_client_ = this->create_client<lifecycle_msgs::srv::GetState>(
    "/fake_imu/get_state",
    rclcpp::ServicesQoS(),
    client_group_);        // ← 이 그룹에 소속시킴

  RCLCPP_INFO(get_logger(), "configured: timeout=%.2fs, check_rate=%.1fHz",
              timeout_sec_, check_rate_hz_);
  return CallbackReturn::SUCCESS;
}

// Inactive → Active : 구독·타이머 생성, 감시 시작
ImuWatchdog::CallbackReturn
ImuWatchdog::on_activate(const rclcpp_lifecycle::State & state)
{
  LifecycleNode::on_activate(state);   // pub_ 켜기

  // 감시 상태 초기화
  received_     = false;
  prev_status_  = Status::Ok;          // 시작 시 헛로그 방지
  last_arrival_ = this->now();
  last_stamp_   = this->now();

  // 구독 — fake_imu와 QoS 반드시 일치! (SensorDataQoS)
  sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
    "imu", rclcpp::SensorDataQoS(),
    [this](const sensor_msgs::msg::Imu::SharedPtr msg) { imu_callback(msg); });

  // 검사 타이머 — ★ 노드 clock(ROS time) 기준! get_clock()이 핵심
  check_timer_ = rclcpp::create_timer(
    this, this->get_clock(),
    rclcpp::Duration::from_seconds(1.0 / check_rate_hz_),
    [this]() { check(); });

  state_timer_group_ = this->create_callback_group(
    rclcpp::CallbackGroupType::MutuallyExclusive);

  state_timer_ = rclcpp::create_timer(
    this, this->get_clock(),
    rclcpp::Duration::from_seconds(1.0),
    [this]() { state_timer_callback(); },
    state_timer_group_);   // ← 이 그룹에 소속시킴

  
  RCLCPP_INFO(get_logger(), "activated: /imu 감시 시작");
  return CallbackReturn::SUCCESS;
}

// Active → Inactive : 감시 중단
ImuWatchdog::CallbackReturn
ImuWatchdog::on_deactivate(const rclcpp_lifecycle::State & state)
{
  check_timer_.reset();
  sub_.reset();
  LifecycleNode::on_deactivate(state);
  state_timer_.reset();
  RCLCPP_INFO(get_logger(), "deactivated: 감시 중단");
  return CallbackReturn::SUCCESS;
}

ImuWatchdog::CallbackReturn
ImuWatchdog::on_cleanup(const rclcpp_lifecycle::State &)
{
  check_timer_.reset();  sub_.reset();  pub_.reset();
  RCLCPP_INFO(get_logger(), "cleaned up");
  state_client_.reset();
  return CallbackReturn::SUCCESS;
}

ImuWatchdog::CallbackReturn
ImuWatchdog::on_shutdown(const rclcpp_lifecycle::State &)
{
  check_timer_.reset();  sub_.reset();  pub_.reset();
  RCLCPP_INFO(get_logger(), "shutdown");
  return CallbackReturn::SUCCESS;
}

// 메시지 도착 시: 상태 갱신 + 신선하면 재발행
void ImuWatchdog::imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg)
{
  last_arrival_ = this->now();
  last_stamp_   = rclcpp::Time(msg->header.stamp);   // ROS_TIME으로 해석
  received_     = true;

  // 신선한 데이터만 /imu_checked로 통과
  rclcpp::Duration age = this->now() - last_stamp_;
  if (age <= rclcpp::Duration::from_seconds(timeout_sec_)) {
    pub_->publish(*msg);
  }
}

// 주기적으로 age 검사 → 상태가 바뀔 때만 로그
void ImuWatchdog::check()
{
  if (!received_) return;   // 첫 메시지 전엔 판정 보류

  rclcpp::Time now = this->now();
  rclcpp::Duration timeout = rclcpp::Duration::from_seconds(timeout_sec_);

  Status status;
  if ((now - last_arrival_) > timeout) {
    status = Status::NoData;          // 도착 자체가 끊김
  } else if ((now - last_stamp_) > timeout) {
    status = Status::Stale;           // 도착은 하나 stamp가 낡음
  } else {
    status = Status::Ok;
  }

  if (status == prev_status_) return;  // 변화 없으면 조용히 (엣지 트리거)

  switch (status) {
    case Status::NoData:
      RCLCPP_ERROR(get_logger(), "[NO DATA] %.2fs 동안 /imu 수신 없음",
                   (now - last_arrival_).seconds());
      break;
    case Status::Stale:
      RCLCPP_WARN(get_logger(), "[STALE] 데이터 낡음 (age %.2fs > timeout %.2fs)",
                  (now - last_stamp_).seconds(), timeout_sec_);
      break;
    case Status::Ok:
      RCLCPP_INFO(get_logger(), "[RECOVERED] 정상 데이터 복구");
      break;
  }
  prev_status_ = status;
}
void ImuWatchdog::state_timer_callback()
{
  if (!state_client_->service_is_ready()) {
    RCLCPP_WARN(get_logger(), "fake_imu get_state 서비스 없음 — 호출 생략");
    return;
  }
  auto request = std::make_shared<lifecycle_msgs::srv::GetState::Request>();
  auto future  = state_client_->async_send_request(request);
  auto response = future.get();   // 블로킹 유지 — 이번엔 다른 스레드가 응답 처리
  RCLCPP_INFO(get_logger(), "fake_imu state: %s",
              response->current_state.label.c_str());
}