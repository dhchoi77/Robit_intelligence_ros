/**
 * @file /include/test_ros_qt/qnode.hpp
 *
 * @brief Communications central!
 *
 * @date February 2011
 **/
/*****************************************************************************
** Ifdefs
*****************************************************************************/

#ifndef test_ros_qt_QNODE_HPP_
#define test_ros_qt_QNODE_HPP_

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

class QNode : public QThread
{
  Q_OBJECT
public:
  QNode();
  ~QNode();

  void publishVelocity(double linear, double angular);
  double getTheta();                        // ← 현재 각도 읽기
  void setPen(int r, int g, int b, int width);


protected:
  void run();

private:
  std::shared_ptr<rclcpp::Node> node;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_sub_;   // ← 구독자
  double current_theta_ = 0.0;              // ← 최신 theta 저장
  rclcpp::Client<turtlesim::srv::SetPen>::SharedPtr pen_client_;

Q_SIGNALS:
  void rosShutDown();
  void velocityUpdated(double linear, double angular);
};

#endif /* test_ros_qt_QNODE_HPP_ */
