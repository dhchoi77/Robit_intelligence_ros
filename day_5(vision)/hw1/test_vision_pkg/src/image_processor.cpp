#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

class ImageProcessor : public rclcpp::Node
{
public:
  ImageProcessor() : Node("image_processor")
  {
    sub_ = this->create_subscription<sensor_msgs::msg::Image>(
      "/camera/image_raw", 10,
      std::bind(&ImageProcessor::callback, this, std::placeholders::_1));

    // 블러 없이 바로 inRange 한 마스크
    pub_red_   = this->create_publisher<sensor_msgs::msg::Image>("/camera/mask_red", 10);
    pub_green_ = this->create_publisher<sensor_msgs::msg::Image>("/camera/mask_green", 10);
    pub_blue_  = this->create_publisher<sensor_msgs::msg::Image>("/camera/mask_blue", 10);

    // 가우시안 블러 후 inRange 한 마스크
    pub_red_blur_   = this->create_publisher<sensor_msgs::msg::Image>("/camera/mask_red_blur", 10);
    pub_green_blur_ = this->create_publisher<sensor_msgs::msg::Image>("/camera/mask_green_blur", 10);
    pub_blue_blur_  = this->create_publisher<sensor_msgs::msg::Image>("/camera/mask_blue_blur", 10);
  }

private:
  // 색 이름에 맞는 HSV 마스크 생성 (빨강은 0/180 양쪽이라 범위 2개)
  cv::Mat makeMask(const cv::Mat & hsv, const std::string & color)
  {
    cv::Mat mask;
    if (color == "red") {
      cv::Mat m1, m2;
      cv::inRange(hsv, cv::Scalar(0, 100, 100),   cv::Scalar(10, 255, 255),  m1);
      cv::inRange(hsv, cv::Scalar(170, 100, 100), cv::Scalar(179, 255, 255), m2);
      mask = m1 | m2;   // 두 범위 합치기
    } else if (color == "green") {
      cv::inRange(hsv, cv::Scalar(40, 100, 100), cv::Scalar(80, 255, 255), mask);
    } else {  // blue
      cv::inRange(hsv, cv::Scalar(100, 100, 100), cv::Scalar(130, 255, 255), mask);
    }
    return mask;
  }

  void publishMask(rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub,
                   const std_msgs::msg::Header & header, const cv::Mat & mask)
  {
    auto out = cv_bridge::CvImage(header, "mono8", mask).toImageMsg();
    pub->publish(*out);
  }

  void callback(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    cv_bridge::CvImagePtr cv_ptr;
    try {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    } catch (const cv_bridge::Exception & e) {
      RCLCPP_ERROR(this->get_logger(), "cv_bridge 예외: %s", e.what());
      return;
    }

    // ===== 1) 블러 없이: BGR → HSV → inRange =====
    cv::Mat hsv;
    cv::cvtColor(cv_ptr->image, hsv, cv::COLOR_BGR2HSV);
    publishMask(pub_red_,   msg->header, makeMask(hsv, "red"));
    publishMask(pub_green_, msg->header, makeMask(hsv, "green"));
    publishMask(pub_blue_,  msg->header, makeMask(hsv, "blue"));

    // ===== 2) 블러 후: 가우시안 → HSV → inRange =====
    cv::Mat blurred, hsv_blur;
    cv::GaussianBlur(cv_ptr->image, blurred, cv::Size(5, 5), 0);
    cv::cvtColor(blurred, hsv_blur, cv::COLOR_BGR2HSV);
    publishMask(pub_red_blur_,   msg->header, makeMask(hsv_blur, "red"));
    publishMask(pub_green_blur_, msg->header, makeMask(hsv_blur, "green"));
    publishMask(pub_blue_blur_,  msg->header, makeMask(hsv_blur, "blue"));
  }

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_red_, pub_green_, pub_blue_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_red_blur_, pub_green_blur_, pub_blue_blur_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ImageProcessor>());
  rclcpp::shutdown();
  return 0;
}