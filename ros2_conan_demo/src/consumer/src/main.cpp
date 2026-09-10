#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <nlohmann/json.hpp>

#include "str_printer/str_printer.hpp"

using namespace std::chrono_literals;

class Greeter : public rclcpp::Node {
 public:
  Greeter() : Node("greeter") {
    max_count_ = declare_parameter<int>("max_count", 0);  // 0 = run forever
    pub_ = create_publisher<std_msgs::msg::String>("greeting", 10);
    RCLCPP_INFO(get_logger(), "str_printer was built against fmt %s, consumer uses nlohmann_json %d.%d.%d",
                str_printer::fmt_version().c_str(), NLOHMANN_JSON_VERSION_MAJOR,
                NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
    timer_ = create_wall_timer(500ms, [this] { tick(); });
  }

 private:
  void tick() {
    std_msgs::msg::String msg;
    // Text comes from str_printer (fmt), envelope from nlohmann_json.
    const nlohmann::json payload = {
        {"seq", ++count_},
        {"text", str_printer::greet("Hello", "ROS 2 + Conan", count_)},
    };
    msg.data = payload.dump();
    RCLCPP_INFO(get_logger(), "publishing: '%s'", msg.data.c_str());
    pub_->publish(msg);
    if (max_count_ > 0 && count_ >= max_count_) {
      RCLCPP_INFO(get_logger(), "reached max_count=%d, shutting down", max_count_);
      rclcpp::shutdown();
    }
  }

  int count_ = 0;
  int max_count_ = 0;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Greeter>());
  rclcpp::shutdown();
  return EXIT_SUCCESS;
}
