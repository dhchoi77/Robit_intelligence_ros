#include <chrono>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

using namespace std::chrono_literals;

class MyPub : public rclcpp::Node {
public:
  MyPub() : Node("my_pub") {
    pub_ = create_publisher<std_msgs::msg::String>("topic", 10);
    timer_ = create_wall_timer(500ms, [this]{ callback(); });
  }

private:
  void callback() {
    auto msg = std_msgs::msg::String();
    msg.data = "Hello World!";
    pub_->publish(msg);
    RCLCPP_INFO(this->get_logger(), "발행: %s", msg.data.c_str());
  }

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MyPub>());
  rclcpp::shutdown();
  return 0;
}
