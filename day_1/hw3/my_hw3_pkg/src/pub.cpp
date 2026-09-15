#include <chrono>
#include <rclcpp/rclcpp.hpp>
#include "my_interfaces/msg/my_msg.hpp"   // 커스텀 msg 헤더

using namespace std::chrono_literals;

class MyPub : public rclcpp::Node {
public:
  MyPub() : Node("my_hw3_pub"), count_(0) {
    pub_ = create_publisher<my_interfaces::msg::MyMsg>("my_topic", 10);
    timer_ = create_wall_timer(500ms, [this]() { callback(); });
  }

private:
  void callback() {
    auto msg = my_interfaces::msg::MyMsg();
    msg.id = count_++;
    msg.label = "hello";
    RCLCPP_INFO(get_logger(), "발행: id=%d, obastacle=%s", msg.id, msg.label.c_str());
    pub_->publish(msg);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<my_interfaces::msg::MyMsg>::SharedPtr pub_;
  int count_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MyPub>());
  rclcpp::shutdown();
  return 0;
}
