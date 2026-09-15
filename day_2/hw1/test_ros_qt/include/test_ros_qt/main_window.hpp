/**
 * @file /include/test_ros_qt/main_window.hpp
 *
 * @brief Qt based gui for %(package)s.
 *
 * @date August 2024
 **/

#ifndef test_ros_qt_MAIN_WINDOW_H
#define test_ros_qt_MAIN_WINDOW_H

/*****************************************************************************
** Includes
*****************************************************************************/

#include <QMainWindow>
#include <QTimer>
#include "QIcon"
#include "qnode.hpp"
#include "ui_mainwindow.h"

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

private:
  Ui::MainWindowDesign* ui;
  void closeEvent(QCloseEvent* event);
  void keyPressEvent(QKeyEvent* event);
  void keyReleaseEvent(QKeyEvent* event);   // ← 키 뗌 감지

  QTimer* timer_;          // ← 타이머
  double cur_linear_ = 0.0;   // ← 현재 전진 속도 상태
  double cur_angular_ = 0.0;  // ← 현재 회전 속도 상태
};

#endif  // test_ros_qt_MAIN_WINDOW_H
