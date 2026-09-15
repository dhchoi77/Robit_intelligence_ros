#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <example_interfaces/srv/add_two_ints.hpp>

using AddTwoInts = example_interfaces::srv::AddTwoInts;

class MyService : public rclcpp::Node {
public:
  MyService() : Node("calc_server") {
    srv_add_ = create_service<AddTwoInts>(
      "add_two_ints",
      [this](const std::shared_ptr<AddTwoInts::Request> req,
             std::shared_ptr<AddTwoInts::Response> res) { add(req, res); });

    srv_minus_ = create_service<AddTwoInts>(
      "minus_two_ints",
      [this](const std::shared_ptr<AddTwoInts::Request> req,
             std::shared_ptr<AddTwoInts::Response> res) { minus(req, res); });

    srv_multiply_ = create_service<AddTwoInts>(
      "multiply_two_ints",
      [this](const std::shared_ptr<AddTwoInts::Request> req,
             std::shared_ptr<AddTwoInts::Response> res) { multiply(req, res); });

    srv_divide_ = create_service<AddTwoInts>(
      "divide_two_ints",
      [this](const std::shared_ptr<AddTwoInts::Request> req,
             std::shared_ptr<AddTwoInts::Response> res) { divide(req, res); });

    RCLCPP_INFO(get_logger(),
      "서비스 준비 완료: add / minus / multiply / divide");
  }

private:
  void add(const std::shared_ptr<AddTwoInts::Request> req,
           std::shared_ptr<AddTwoInts::Response> res) {
    res->sum = req->a + req->b;
    RCLCPP_INFO(get_logger(), "요청: %ld + %ld = %ld", req->a, req->b, res->sum);
  }

  void minus(const std::shared_ptr<AddTwoInts::Request> req,
             std::shared_ptr<AddTwoInts::Response> res) {
    res->sum = req->a - req->b;
    RCLCPP_INFO(get_logger(), "요청: %ld - %ld = %ld", req->a, req->b, res->sum);
  }

  void multiply(const std::shared_ptr<AddTwoInts::Request> req,
                std::shared_ptr<AddTwoInts::Response> res) {
    res->sum = req->a * req->b;
    RCLCPP_INFO(get_logger(), "요청: %ld * %ld = %ld", req->a, req->b, res->sum);
  }

  void divide(const std::shared_ptr<AddTwoInts::Request> req,
              std::shared_ptr<AddTwoInts::Response> res) {
    if (req->b == 0) {                          // 0으로 나누기 방어
      res->sum = 0;
      RCLCPP_WARN(get_logger(), "0으로 나눌 수 없습니다 (b=0)");
      return;
    }
    res->sum = req->a / req->b;
    RCLCPP_INFO(get_logger(), "요청: %ld / %ld = %ld", req->a, req->b, res->sum);
  }

  rclcpp::Service<AddTwoInts>::SharedPtr srv_add_;
  rclcpp::Service<AddTwoInts>::SharedPtr srv_minus_;
  rclcpp::Service<AddTwoInts>::SharedPtr srv_multiply_;
  rclcpp::Service<AddTwoInts>::SharedPtr srv_divide_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MyService>());
  rclcpp::shutdown();
  return 0;
}