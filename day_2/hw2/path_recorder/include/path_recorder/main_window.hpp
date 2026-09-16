/**
 * @file /include/path_recorder/main_window.hpp
 *
 * @brief Qt based gui for %(package)s.
 *
 * @date August 2024
 **/

#ifndef path_recorder_MAIN_WINDOW_H
#define path_recorder_MAIN_WINDOW_H

/*****************************************************************************
** Includes
*****************************************************************************/

#include <QMainWindow>
#include <QTimer>
#include <QSlider>
#include "QIcon"
#include "qnode.hpp"
#include "ui_mainwindow.h"
#include <vector>

// 클래스 밖(위쪽)에, 점 하나를 나타내는 구조체
struct PosePoint {
  double x;
  double y;
  double theta;
};
/*****************************************************************************
** Interface [MainWindow]
*****************************************************************************/
/**
 * @brief Qt central, all operations relating to the view part here.
 */
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();
  QNode* qnode;

private slots:
  void onForward();
  void onBackward();
  void onLeft();
  void onRight();
  void onTimer();
  void onTriangle();
  void onRect();
  void onCircle();
  void onVelocityUpdated(double linear, double angular);
  void onPenChanged(); //슬라이더 4개 공용
  void onRecStart();
  void onRecStop();
  void onReplay();
private:
  Ui::MainWindowDesign* ui;
  void closeEvent(QCloseEvent* event);
  void keyPressEvent(QKeyEvent* event);
  void keyReleaseEvent(QKeyEvent* event);   // ← 키 뗌 감지

  QTimer* timer_;          // ← 타이머
  double cur_linear_ = 0.0;   // ← 현재 전진 속도 상태
  double cur_angular_ = 0.0;  // ← 현재 회전 속도 상태
  bool rotating_ = false;   //지금 회전중이가?
  double target_theta_ = 0.0; //목표 각도

  double normalizeAngle(double angle);    //각도 보정 함수
  double last_theta_= 0.0;                //직전 theta
  double rotated_=0.0;                    //지금까지 돌아간 총량
  bool drawing_ = false;      // 삼각형 그리는 중?
  int step_ = 0;              // 현재 단계 (0~6)
  int forward_count_ = 0;     // 전진 타이머 카운터

  double turn_angle_ =2.09;    // 한 번에 도는 각도(일단 삼각형으로 해놨으나 함수에 들어가면 다시 세팅)
  int total_sides_=3;          // 변 개수

  bool drawing_circle_ = false;
  double circle_rotated_ = 0.0; 

  std::vector<PosePoint> recorded_path_;   // 기록된 경로 (점들의 리스트)
  bool recording_ = false;                 // 지금 기록 중?

  bool replaying_ = false;      // 재생 중?
  int replay_index_ = 0;        // 지금 몇 번째 점 재생 중?

  double total_distance_ = 0.0;   // 총 이동 거리

};

#endif  // path_recorder_MAIN_WINDOW_H
