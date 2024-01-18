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
#include "std_msgs/msg/byte_multi_array.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;

class MinimalSubscriber : public rclcpp::Node
{
public:
  MinimalSubscriber()
  : Node("minimal_subscriber")
  {
    const auto effort = rclcpp::QoS(100).best_effort();

    subscription_options_.event_callbacks.message_lost_callback = [this](rclcpp::QOSMessageLostInfo & msg)
    {
        RCLCPP_INFO(
          get_logger(),
          "%li got got lost, with total of %li",msg.total_count_change, msg.total_count);
    };

    subscription_ = this->create_subscription<std_msgs::msg::ByteMultiArray>(
      "topic", effort, std::bind(&MinimalSubscriber::topic_callback, this, _1),subscription_options_);

    window_start_time = get_clock()->now();

    stats_timer_ = create_wall_timer(1s, [this]() {
      const rclcpp::Time t(this->get_clock()->now());
      const double elapsed = (t - window_start_time).seconds();
      const double msg_per_sec = this->window_num_msg / elapsed;

      RCLCPP_INFO(
          get_logger(),
          "rate: %.6f msgs/sec \n messages received: %d \n elapsed seconds: %.6f",
          msg_per_sec,
          window_num_msg,
          elapsed);
      this->window_num_msg = 0;
      window_start_time = t;
    });
  }

private:
  void topic_callback(std_msgs::msg::ByteMultiArray::ConstSharedPtr msg) 
  {

    (void)msg;
    this->window_num_msg++;
  }

  int window_num_msg = 0;
  rclcpp::Time window_start_time;
  rclcpp::Subscription<std_msgs::msg::ByteMultiArray>::SharedPtr subscription_;
  rclcpp::TimerBase::SharedPtr stats_timer_;
  rclcpp::SubscriptionOptions subscription_options_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv); 
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}
