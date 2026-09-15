#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

class MySub : public rclcpp::Node {
public:
  MySub() : Node("my_sub") {
    sub_ = create_subscription<std_msgs::msg::String>(
      "topic", 10,
      [this](const std_msgs::msg::String & msg) {
        RCLCPP_INFO(this->get_logger(), "수신: %s", msg.data.c_str());
      });
  }

private:
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MySub>());
  rclcpp::shutdown();
  return 0;
}
