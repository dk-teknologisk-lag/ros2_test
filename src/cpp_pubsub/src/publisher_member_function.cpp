// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses std::bind() to register a
 * member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node
{
public:
  MinimalPublisher()
  : Node("minimal_publisher")
  {
    this->declare_parameter("freq", 2);
    this->declare_parameter("bytesize", 10);
    
    const auto freq = this->get_parameter("freq").as_int();
    bytesize = this->get_parameter("bytesize").as_int();

    RCLCPP_INFO(this->get_logger(),"Frequency is set to: %li",freq);
    RCLCPP_INFO(this->get_logger(),"Byte size is set to: %li",bytesize);

    const auto effort = rclcpp::QoS(100).best_effort();

    publisher_ = this->create_publisher<std_msgs::msg::ByteMultiArray>("topic", effort);
    timer_ = this->create_wall_timer(
      std::chrono::duration<double>(1.0 / freq), std::bind(&MinimalPublisher::timer_callback, this));

    window_start_time = get_clock()->now();

    stats_timer_ = create_wall_timer(1s, [this]() {
      const rclcpp::Time t(get_clock()->now());
      const double elapsed = (t - window_start_time).seconds();
      const double msg_per_sec = msg_cnt / elapsed;

      RCLCPP_INFO(
          get_logger(),
          "\nrate: %.6f msgs/sec\npublished: %li\nelapsed seconds: %.6f",
          msg_per_sec,
          msg_cnt,
          elapsed);
      msg_cnt = 0;
      window_start_time = t;
    });
  }

private:
  void timer_callback()
  {
    std_msgs::msg::ByteMultiArray::UniquePtr msg{new std_msgs::msg::ByteMultiArray};
    msg->data = std::vector<unsigned char>(bytesize);
    publisher_->publish(std::move(msg));
    msg_cnt++;
  }

  rclcpp::TimerBase::SharedPtr stats_timer_;
  rclcpp::Time window_start_time;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::ByteMultiArray>::SharedPtr publisher_;
  int64_t bytesize;
  size_t msg_cnt{0};
  
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalPublisher>());
  rclcpp::shutdown();
  return 0;
}
