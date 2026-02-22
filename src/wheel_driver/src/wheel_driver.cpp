#include <memory>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
//#include "std_msgs/msg/string.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std;
using namespace rclcpp;

class WheelDriver : public Node
{
   Subscription<geometry_msgs::msg::Twist>::SharedPtr subscr;
public:
   // Node(const std::string &node_name)
   WheelDriver() : Node("wheel_driver")
   {
      // Callback function to be called every time the subscriber receives a message
      auto topic_callback = [this](std_msgs::msg::String::UniquePtr message) -> void
      {
         // Log an event
         // RCLCPP_INFO(rclcpp::Logger logger, std::string log_message)
         RCLCPP_INFO(get_logger(), "I have heard the next number in the sequence, and that number is %s.", message->data.c_str());
      };
      
      // Create a subscriber on the node
      const rclcpp::QoS qos = topic_qos();
      // create_subscriber(const std::string &topic_name, rclcpp::QoS QoS, CallbackT && callback)
      subscr = create_subscription<geometry_msgs::msg::Twist>(/*`real_name "/" + `?*/"cmd_vel", /*`10`?*/qos, topic_callback);
      /*velocity_sub_ = nh_->create_subscription<geometry_msgs::msg::Twist>(
       real_name + "/cmd_vel", qos, std::bind(
         &Turtle::velocityCallback, this,
         std::placeholders::_1))*/
   }
};

int main(int argc, char * argv[])
{
   init(argc, argv);
   
   // Start up the node
   spin(make_shared<WheelDriver>());

   // Clean up
   shutdown();
   
   return 0;
}
