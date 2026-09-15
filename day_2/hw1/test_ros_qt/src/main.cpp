#include <QApplication>
#include <iostream>

#include "../include/test_ros_qt/main_window.hpp"

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
