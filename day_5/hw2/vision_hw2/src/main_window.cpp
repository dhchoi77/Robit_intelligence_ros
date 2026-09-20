#include "vision_hw2/main_window.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindowDesign)
{
    ui->setupUi(this);

    // TODO: 색별 초기 HSV 범위를 넣어두세요 (튜닝 시작값)
    //   예) range_[ORANGE] = {5,20, 120,255, 120,255};
    //   WHITE, BLUE, NEON도 각각 채우기
        // 색별 초기 HSV 범위 {h_min, h_max, s_min, s_max, v_min, v_max}
    range_[WHITE]  = {0, 0,   0,  40, 230, 255};   // 흰선: 저채도·고명도
    range_[BLUE]   = {90, 130,  80, 255,  80, 255};  // 파란선
    range_[NEON]   = {30,  90,  60, 255,  150, 255};  // 네온(형광 초록) 콘
    range_[ORANGE] = {0,  25, 100, 255, 180, 255};   // 주황 콘

    cap.open("/dev/v4l/by-id/usb-Sonix_Technology_Co.__Ltd._USB_Live_camera_SN0001-video-index0",
         cv::CAP_V4L2);
    if (!cap.isOpened()) { qWarning("USB 카메라를 열 수 없습니다."); return; }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // 슬라이더 range 설정 (Designer에서 해도 됨)
    ui->Hue_min->setRange(0, 179);  ui->Hue_max->setRange(0, 179);
    ui->Sat_min->setRange(0, 255);  ui->Sat_max->setRange(0, 255);
    ui->Val_min->setRange(0, 255);  ui->Val_max->setRange(0, 255);

    // 슬라이더 6개 → 공용 슬롯 연결
    for (QSlider* s : {ui->Hue_min, ui->Hue_max, ui->Sat_min,
                       ui->Sat_max, ui->Val_min, ui->Val_max})
        connect(s, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);

    // 라디오버튼 4개 → 공용 슬롯 연결 (라디오 objectName이 라벨과 같다면 이 이름들)
    for (QRadioButton* r : {ui->radio_white, ui->radio_blue,
                        ui->radio_neon, ui->radio_orange})   // ← 라디오버튼 이름
      connect(r, &QRadioButton::toggled, this, &MainWindow::onColorSelected);
    // ⚠️ 주의: 라벨과 라디오버튼 objectName이 겹치면 안 됨! 아래 설명 참고

    loadSlidersFromRange();   // 시작 시 current_ 색 값을 슬라이더에 표시

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(30);
}
void MainWindow::showMat(QLabel* label, const cv::Mat& mat)
{
    QImage img;
    if (mat.channels() == 3) {              // 컬러 (BGR)
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        img = QImage(rgb.data, rgb.cols, rgb.rows,
                     static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    } else {                                // 마스크 (1채널)
        img = QImage(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
    }
    label->setPixmap(QPixmap::fromImage(img).scaled(
        label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
cv::Point MainWindow::detectAndDraw(const cv::Mat& mask, cv::Mat& result, const cv::Scalar& color)
{
    // 1) 마스크에서 외곽선(컨투어) 찾기
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 2) 가장 큰 덩어리 하나를 콘으로 간주
    double best_area = 0;
    int best_idx = -1;
    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area < 100) continue;        // 너무 작으면 노이즈 → 무시 (값은 조정 가능)
        if (area > best_area) {          // 지금까지 중 제일 큰 것 기억
            best_area = area;
            best_idx = static_cast<int>(i);
        }
    }

    // 3) 찾았으면 바운딩박스 그리기
        if (best_idx >= 0) {
        cv::Rect box = cv::boundingRect(contours[best_idx]);
        cv::rectangle(result, box, color, 2);
        int cx = box.x + box.width / 2;
        int cy = box.y + box.height / 2;
        return cv::Point(cx, cy);   // 콘 중심 반환
    }
    return cv::Point(-1, -1);       // 못 찾음
}
// cx,cy: 콘 중심 / white_x: 세로선 x / blue_y: 가로선 y
// 결과를 ud(위/아래), rl(좌/우) 문자열에 담음
void MainWindow::classify(cv::Point c, int white_x, int blue_y,
              QString& ud, QString& rl)
{
    if (c.x < 0) {                 // 콘 못 찾음
        ud = "Unknown";  rl = "Unknown";
        return;
    }
    // 위/아래 판정
    if (blue_y < 0)            ud = "?";           // 기준선 못 찾음
    else if (c.y >= blue_y-30 && c.y <=blue_y)     ud="on_blue";
    else if (c.y < blue_y-30)     ud = "Up";
    else                       ud = "Down";
    // 좌/우 판정  ← 직접 채워보기 (white_x와 c.x 비교)
    if (white_x < 0)        rl = "?";
    else if (c.x >= white_x-30 && c.x <=white_x+30)     rl="on_white";
    else if (c.x < white_x-30) rl = "Left";
    else                    rl = "Right";
}
MainWindow::~MainWindow()
{
    if (cap.isOpened()) cap.release();
    delete ui;
}

// 현재 색의 저장값을 슬라이더로 (로드 중엔 시그널 차단해 되먹임 방지)
void MainWindow::loadSlidersFromRange()
{
    const HsvRange& r = range_[current_];
    for (QSlider* s : {ui->Hue_min, ui->Hue_max, ui->Sat_min,
                       ui->Sat_max, ui->Val_min, ui->Val_max})
        s->blockSignals(true);

    ui->Hue_min->setValue(r.h_min);  ui->Hue_max->setValue(r.h_max);
    ui->Sat_min->setValue(r.s_min);  ui->Sat_max->setValue(r.s_max);
    ui->Val_min->setValue(r.v_min);  ui->Val_max->setValue(r.v_max);

    for (QSlider* s : {ui->Hue_min, ui->Hue_max, ui->Sat_min,
                       ui->Sat_max, ui->Val_min, ui->Val_max})
        s->blockSignals(false);
    updateSliderLabels();
}

// 슬라이더 현재값을 현재 색 범위에 반영
void MainWindow::writeRangeFromSliders()
{
    HsvRange& r = range_[current_];
    r.h_min = ui->Hue_min->value();  r.h_max = ui->Hue_max->value();
    r.s_min = ui->Sat_min->value();  r.s_max = ui->Sat_max->value();
    r.v_min = ui->Val_min->value();  r.v_max = ui->Val_max->value();
}
void MainWindow::updateSliderLabels()
{
    ui->print_Hx->setText(QString::number(ui->Hue_max->value()));
    ui->print_Hn->setText(QString::number(ui->Hue_min->value()));
    ui->print_Sx->setText(QString::number(ui->Sat_max->value()));
    ui->print_Sn->setText(QString::number(ui->Sat_min->value()));
    ui->print_Vx->setText(QString::number(ui->Val_max->value()));
    ui->print_Vn->setText(QString::number(ui->Val_min->value()));
}
void MainWindow::onSliderChanged()
{
    writeRangeFromSliders();
    updateSliderLabels();       // ← 추가
}

void MainWindow::onColorSelected()
{
    // toggled는 켜질 때/꺼질 때 둘 다 오므로, 켜진 것만 처리
    if      (ui->radio_white->isChecked())  current_ = WHITE;
    else if (ui->radio_blue->isChecked())   current_ = BLUE;
    else if (ui->radio_neon->isChecked())   current_ = NEON;
    else if (ui->radio_orange->isChecked()) current_ = ORANGE;  
    else return;

    loadSlidersFromRange();    // 선택된 색 값을 슬라이더에 로드
}
// 파란선 마스크에서 가로선의 대표 y좌표를 구함. 못 찾으면 -1.
int MainWindow::findLineY(const cv::Mat& mask)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 가장 큰 컨투어(=선) 찾기  ← detectAndDraw와 같은 로직
    double best_area = 0; int best = -1;
    for (size_t i = 0; i < contours.size(); ++i) {
        double a = cv::contourArea(contours[i]);
        if (a < 100) continue;
        if (a > best_area) { best_area = a; best = (int)i; }
    }
    if (best < 0) return -1;

    cv::Rect box = cv::boundingRect(contours[best]);
    return box.y + box.height / 2;   // 가로선의 중심 y
}
// 파란선 마스크에서 가로선의 대표 y좌표를 구함. 못 찾으면 -1.
int MainWindow::findLineX(const cv::Mat& mask)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 가장 큰 컨투어(=선) 찾기  ← detectAndDraw와 같은 로직
    double best_area = 0; int best = -1;
    for (size_t i = 0; i < contours.size(); ++i) {
        double a = cv::contourArea(contours[i]);
        if (a < 100) continue;
        if (a > best_area) { best_area = a; best = (int)i; }
    }
    if (best < 0) return -1;

    cv::Rect box = cv::boundingRect(contours[best]);
    return box.x + box.width / 2;   // 가로선의 중심 y
}
void MainWindow::updateFrame()
{
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // 저장된 색별 범위로 마스크 생성
    auto maskOf = [&](ColorId id){
        const HsvRange& r = range_[id];
        cv::Mat m;
        cv::inRange(hsv, cv::Scalar(r.h_min, r.s_min, r.v_min),
                         cv::Scalar(r.h_max, r.s_max, r.v_max), m);
        return m;
    };

    showMat(ui->Usb_Cam,     frame);
    showMat(ui->White_line,  maskOf(WHITE));   // ⚠️ 라벨 이름 (라디오와 겹침 주의)
    showMat(ui->Blue_line,   maskOf(BLUE));
    showMat(ui->Neon_cone,   maskOf(NEON));
    showMat(ui->Orange_cone, maskOf(ORANGE));

    // 이미 mask_orange, mask_neon 등은 만들어져 있음
    cv::Mat result = frame.clone();   // 원본 복사 (여기에 박스를 그림)

        cv::Point neon_c   = detectAndDraw(maskOf(NEON),   result, cv::Scalar(0,255,0));
    cv::Point orange_c = detectAndDraw(maskOf(ORANGE), result, cv::Scalar(0,0,255));

    int blue_y  = findLineY(maskOf(BLUE));
    int white_x = findLineX(maskOf(WHITE));

    QString ud, rl;
    classify(neon_c, white_x, blue_y, ud, rl);
    ui->Up_Down_N->setText(ud);
    ui->R_L_N->setText(rl);

    classify(orange_c, white_x, blue_y, ud, rl);
    ui->Up_Down_O->setText(ud);
    ui->R_L_O->setText(rl);

    showMat(ui->Find_object, result);   // TODO: 검출 결과 그리기
    
}

// showMat 은 이전 답변 그대로