/**
 * @file /include/path_recorder/qnode.hpp
 *
 * @brief Communications central!
 *
 * @date February 2011
 **/
/*****************************************************************************
** Ifdefs
*****************************************************************************/

#ifndef path_recorder_QNODE_HPP_
#define path_recorder_QNODE_HPP_

/*****************************************************************************
** Includes
*****************************************************************************/

#ifndef Q_MOC_RUN
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <turtlesim/msg/pose.hpp>          // ← pose 메시지 타입
#include <turtlesim/srv/set_pen.hpp>
#endif
#include <QThread>
#include <turtlesim/srv/teleport_absolute.hpp>

class QNode : public QThread
{
  Q_OBJECT
public:
  QNode();
  ~QNode();

  void publishVelocity(double linear, double angular);
  double getTheta();                        // ← 현재 각도 읽기
  double getX();
  double getY();
  void setPen(int r, int g, int b, int width);
  void teleport(double x, double y, double theta);

protected:
  void run();

private:
  std::shared_ptr<rclcpp::Node> node;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_sub_;   // ← 구독자
  rclcpp::Client<turtlesim::srv::SetPen>::SharedPtr pen_client_;
  double current_theta_ = 0.0;
  double current_x_ = 0.0;      // ← 추가
  double current_y_ = 0.0;      // ← 추가
  rclcpp::Client<turtlesim::srv::TeleportAbsolute>::SharedPtr teleport_client_;

Q_SIGNALS:
  void rosShutDown();
  void velocityUpdated(double linear, double angular);
};

#endif /* path_recorder_QNODE_HPP_ */
