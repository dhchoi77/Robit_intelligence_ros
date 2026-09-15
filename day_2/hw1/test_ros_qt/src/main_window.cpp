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
  qnode->publishVelocity(cur_linear_, cur_angular_);
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
