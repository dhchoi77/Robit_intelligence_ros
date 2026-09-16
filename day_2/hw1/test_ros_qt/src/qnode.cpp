/**
 * @file /src/qnode.cpp
 *
 * @brief Ros communication central!
 *
 * @date August 2024
 **/

/*****************************************************************************
** Includes
*****************************************************************************/

#include "../include/test_ros_qt/qnode.hpp"

QNode::QNode()
{
  int argc = 0;
  char** argv = NULL;
  rclcpp::init(argc, argv);
  node = rclcpp::Node::make_shared("test_ros_qt");

  publisher_ = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);  // ← 추가
  pose_sub_ = node->create_subscription<turtlesim::msg::Pose>(
    "/turtle1/pose", 10,
    [this](const turtlesim::msg::Pose::SharedPtr msg) {
      current_theta_ = msg->theta;          // 받을 때마다 theta 갱신
      
    });
  pen_client_ = node->create_client<turtlesim::srv::SetPen>("/turtle1/set_pen"); 
  this->start();
}

// ... ~QNode(), run()은 그대로 ...

void QNode::publishVelocity(double linear, double angular)
{
  auto msg = geometry_msgs::msg::Twist();
  msg.linear.x = linear;
  msg.angular.z = angular;
  publisher_->publish(msg);

  Q_EMIT velocityUpdated(linear, angular);   // ← 화면 갱신 신호 발생
}
double QNode::getTheta()      // ← 여기, publishVelocity 아래에 추가
{
  return current_theta_;
}
QNode::~QNode()
{
  if (rclcpp::ok())
  {
    rclcpp::shutdown();
  }
}

void QNode::run()
{
  rclcpp::WallRate loop_rate(20);
  while (rclcpp::ok())
  {
    rclcpp::spin_some(node);
    loop_rate.sleep();
  }
  rclcpp::shutdown();
  Q_EMIT rosShutDown();
}
void QNode::setPen(int r, int g, int b, int width)
{
  if (!pen_client_ || !pen_client_->service_is_ready())
  {
    return;
  }

  auto request = std::make_shared<turtlesim::srv::SetPen::Request>();
  request->r = r;
  request->g = g;
  request->b = b;
  request->width = width;
  request->off = 0;

  pen_client_->async_send_request(request);
}