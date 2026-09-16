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

#include "../include/path_recorder/qnode.hpp"

QNode::QNode()
{
  int argc = 0;
  char** argv = NULL;
  rclcpp::init(argc, argv);
  node = rclcpp::Node::make_shared("path_recorder");

  publisher_ = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);  // ← 추가
  pose_sub_ = node->create_subscription<turtlesim::msg::Pose>(
    "/turtle1/pose", 10,
    [this](const turtlesim::msg::Pose::SharedPtr msg) {
      current_x_ = msg->x;          // ← 추가
      current_y_ = msg->y;          // ← 추가
      current_theta_ = msg->theta;
    });
  pen_client_ = node->create_client<turtlesim::srv::SetPen>("/turtle1/set_pen"); 
  teleport_client_ = node->create_client<turtlesim::srv::TeleportAbsolute>("/turtle1/teleport_absolute");
  this->start();
}
void QNode::teleport(double x, double y, double theta)
{
  if (!teleport_client_ || !teleport_client_->service_is_ready())
  {
    return;
  }

  auto request = std::make_shared<turtlesim::srv::TeleportAbsolute::Request>();
  request->x = x;
  request->y = y;
  request->theta = theta;

  teleport_client_->async_send_request(request);
}
// ... ~QNode(), run()은 그대로 ...
double QNode::getX() { return current_x_; }
double QNode::getY() { return current_y_; }
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