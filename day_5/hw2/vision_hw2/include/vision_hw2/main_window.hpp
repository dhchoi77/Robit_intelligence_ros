// ... 기존 include에 더해
#include <QRadioButton>
#include <QMainWindow>       // ← 이게 빠져서 전체가 무너진 것
#include <QTimer>
#include <QLabel>
#include <QSlider>           // 슬라이더 타입 쓰니 추가
#include <opencv2/opencv.hpp>

namespace Ui { class MainWindowDesign; }

struct HsvRange { int h_min, h_max, s_min, s_max, v_min, v_max; };
enum ColorId { WHITE = 0, BLUE, NEON, ORANGE, COLOR_COUNT };

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private slots:
  void updateFrame();
  void onSliderChanged();      // 슬라이더 6개 공용 슬롯
  void onColorSelected();      // 라디오버튼 공용 슬롯

private:
  void showMat(QLabel* label, const cv::Mat& mat);
  cv::Point detectAndDraw(const cv::Mat& mask, cv::Mat& result, const cv::Scalar& color);
  void classify(cv::Point c, int white_x, int blue_y,QString& ud, QString& rl);
  void loadSlidersFromRange();
  void writeRangeFromSliders();
  int findLineY(const cv::Mat& mask);   // 가로선 y
  int findLineX(const cv::Mat& mask);   // 세로선 x (직접 만들어보기)
  void updateSliderLabels();

  Ui::MainWindowDesign* ui;
  cv::VideoCapture cap;
  QTimer* timer;

  HsvRange range_[COLOR_COUNT];  // 색별 저장
  int current_ = ORANGE;         // 지금 튜닝 중인 색
};