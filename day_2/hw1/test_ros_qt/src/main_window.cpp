/**
 * @file /src/main_window.cpp
 *
 * @brief Implementation for the qt gui.
 *
 * @date August 2024
 **/
/*****************************************************************************
** Includes
*****************************************************************************/

#include "../include/test_ros_qt/main_window.hpp"
#include <QKeyEvent>
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindowDesign)
{
  ui->setupUi(this);
  this->setFocusPolicy(Qt::StrongFocus);
  this->setFocus();

  timer_=new QTimer(this);
  connect(timer_, &QTimer::timeout, this, &MainWindow::onTimer);
  timer_->start(50);
  QIcon icon("://ros-icon.png");

  this->setWindowIcon(icon);

  qnode = new QNode();

  QObject::connect(qnode, SIGNAL(rosShutDown()), this, SLOT(close()));
  connect(ui->btnW, &QPushButton::clicked, this, &MainWindow::onForward);
  connect(ui->btnS, &QPushButton::clicked, this, &MainWindow::onBackward);
  connect(ui->btnA, &QPushButton::clicked, this, &MainWindow::onLeft);
  connect(ui->btnD, &QPushButton::clicked, this, &MainWindow::onRight);
  connect(ui->btnTriangle, &QPushButton::clicked, this, &MainWindow::onTriangle);
  connect(ui->btnRect,   &QPushButton::clicked, this, &MainWindow::onRect);
  connect(ui->btnCircle, &QPushButton::clicked, this, &MainWindow::onCircle);
  connect(qnode, &QNode::velocityUpdated, this, &MainWindow::onVelocityUpdated);

}
void MainWindow::keyPressEvent(QKeyEvent* event)
{
  switch (event->key())
  {
    case Qt::Key_W: cur_linear_  =  2.0; break;
    case Qt::Key_S: cur_linear_  = -2.0; break;
    case Qt::Key_A: cur_angular_ =  2.0; break;
    case Qt::Key_D: cur_angular_ = -2.0; break;
    default: QMainWindow::keyPressEvent(event); break;
  }
}
void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
  switch (event->key())
  {
    case Qt::Key_W:
    case Qt::Key_S: cur_linear_  = 0.0; break;   // 앞뒤 키 떼면 전진 정지
    case Qt::Key_A:
    case Qt::Key_D: cur_angular_ = 0.0; break;   // 좌우 키 떼면 회전 정지
    default: QMainWindow::keyReleaseEvent(event); break;
  }
}
void MainWindow::onTimer()
{
  if (drawing_circle_)
  {
    double current = qnode->getTheta();
    double delta = normalizeAngle(current - last_theta_);
    circle_rotated_ += fabs(delta);
    last_theta_ = current;

    if (circle_rotated_ < 6.28)          // 360도(2π) 돌 때까지
    {
      qnode->publishVelocity(2.0, 2.0);  // 전진+회전 동시 → 원
    }
    else
    {
      qnode->publishVelocity(0.0, 0.0);
      drawing_circle_ = false;
    }
    return;
  }
  if (drawing_)
  {
    // 짝수 단계(0,2,4) = 전진, 홀수 단계(1,3,5) = 회전
    if (step_ >= total_sides_*2 -1)              // 6이면 끝
    {
      qnode->publishVelocity(0.0, 0.0);
      drawing_ = false;
      return;
    }

    if (step_ % 2 == 0)          // 전진 단계
    {
      if (forward_count_ < 30)   // 30번(1.5초) 동안 전진
      {
        qnode->publishVelocity(2.0, 0.0);
        forward_count_++;
      }
      else                        // 전진 끝 → 다음(회전) 단계로
      {
        forward_count_ = 0;
        rotated_ = 0.0;                       // 회전 준비
        last_theta_ = qnode->getTheta();
        step_++;
      }
    }
    else                          // 회전 단계
    {
      double current = qnode->getTheta();
      double delta = normalizeAngle(current - last_theta_);
      rotated_ += fabs(delta);
      last_theta_ = current;

      if (rotated_ < turn_angle_)        // 120도 안 됐으면 계속 회전
      {
        qnode->publishVelocity(0.0, 0.2);
      }
      else                        // 회전 끝 → 다음(전진) 단계로
      {
        step_++;
      }
    }
  }
  else
  {
    qnode->publishVelocity(cur_linear_, cur_angular_);   // 평소엔 WASD
  }
}
void MainWindow::onTriangle()
{
  turn_angle_ = 2.09;    // 120도
  total_sides_ = 3;
  drawing_ = true; step_ = 0; forward_count_ = 0;
}

void MainWindow::onRect()
{
  turn_angle_ = 1.57;    // 90도
  total_sides_ = 4;
  drawing_ = true; step_ = 0; forward_count_ = 0;
}
void MainWindow::onCircle()
{
  drawing_circle_ = true;
  circle_rotated_ = 0.0;
  last_theta_ = qnode->getTheta();
}
double MainWindow::normalizeAngle(double angle)
{
  while (angle >  M_PI) angle -= 2 * M_PI;
  while (angle < -M_PI) angle += 2 * M_PI;
  return angle;
}
void MainWindow::onVelocityUpdated(double linear, double angular)
{
  QString text = QString("linear: %1  angular: %2")
                   .arg(linear, 0, 'f', 2)
                   .arg(angular, 0, 'f', 2);
  ui->lineEdit->setText(text);
}
void MainWindow::closeEvent(QCloseEvent* event)
{
  QMainWindow::closeEvent(event);
}
void MainWindow::onForward()  { qnode->publishVelocity( 2.0,  0.0); }
void MainWindow::onBackward() { qnode->publishVelocity(-2.0,  0.0); }
void MainWindow::onLeft()     { qnode->publishVelocity( 0.0,  2.0); }
void MainWindow::onRight()    { qnode->publishVelocity( 0.0, -2.0); }

MainWindow::~MainWindow()
{
  delete ui;
}
