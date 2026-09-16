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

#include "../include/path_recorder/main_window.hpp"
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

  connect(ui->sliderR,     &QSlider::valueChanged, this, &MainWindow::onPenChanged);
  connect(ui->sliderG,     &QSlider::valueChanged, this, &MainWindow::onPenChanged);
  connect(ui->sliderB,     &QSlider::valueChanged, this, &MainWindow::onPenChanged);
  connect(ui->sliderWidth, &QSlider::valueChanged, this, &MainWindow::onPenChanged);

  connect(ui->btnRecStart, &QPushButton::clicked, this, &MainWindow::onRecStart);
  connect(ui->btnRecStop,  &QPushButton::clicked, this, &MainWindow::onRecStop);
  connect(ui->btnReplay, &QPushButton::clicked, this, &MainWindow::onReplay);

}
void MainWindow::onRecStart()
{
  recorded_path_.clear();
  total_distance_ = 0.0;        // ← 거리도 초기화
  recording_ = true;
  ui->labelStatus->setText("기록 중...");
}

void MainWindow::onRecStop()
{
  recording_ = false;
  double elapsed = recorded_path_.size() * 0.05;
  ui->labelStatus->setText(
    QString("기록 완료 | 점 %1개 | 거리: %2 | 시간: %3초")
      .arg(recorded_path_.size())
      .arg(total_distance_, 0, 'f', 2)
      .arg(elapsed, 0, 'f', 1));
}
void MainWindow::onPenChanged()
{
  int r = ui->sliderR->value();       // 각 슬라이더의 현재 값 읽기
  int g = ui->sliderG->value();
  int b = ui->sliderB->value();
  int w = ui->sliderWidth->value();

  qnode->setPen(r, g, b, w);          // 펜 설정 적용

  // ui->labelR->setText(QString::number(r));
  // ui->labelG->setText(QString::number(g));
  // ui->labelB->setText(QString::number(b));
  // ui->labelWidth->setText(QString::number(w));  
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
  if (replaying_)
  {
    if (replay_index_ < (int)recorded_path_.size())   // 아직 점 남음
    {
      PosePoint p = recorded_path_[replay_index_];     // 현재 점 꺼내기
      qnode->teleport(p.x, p.y, p.theta);              // 그 점으로 이동
      replay_index_++;                                  // 다음 점으로
    }
    else                                                // 다 재생함
    {
      replaying_ = false;
      ui->labelStatus->setText("재생 완료");
    }
    return;   // 재생 중엔 다른 처리 안 함
  }
  if (recording_)
{
  PosePoint p;
  p.x = qnode->getX();
  p.y = qnode->getY();
  p.theta = qnode->getTheta();

  // 직전 점이 있으면 거리 누적
  if (!recorded_path_.empty())
  {
    PosePoint prev = recorded_path_.back();   // 마지막에 저장된 점
    double dx = p.x - prev.x;
    double dy = p.y - prev.y;
    total_distance_ += std::sqrt(dx*dx + dy*dy);   // 피타고라스로 거리 더하기
  }

  recorded_path_.push_back(p);

  // 상태 + 거리 + 시간 실시간 표시
  double elapsed = recorded_path_.size() * 0.05;   // 점 개수 × 0.05초
  ui->labelStatus->setText(
    QString("기록 중 | 거리: %1 | 시간: %2초")
      .arg(total_distance_, 0, 'f', 2)
      .arg(elapsed, 0, 'f', 1));
}
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
void MainWindow::onReplay()
{
  if (recorded_path_.empty())   // 기록이 없으면
  {
    ui->labelStatus->setText("기록된 경로가 없습니다");
    return;
  }
  replaying_ = true;
  replay_index_ = 0;            // 처음부터
  ui->labelStatus->setText("재생 중...");
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
