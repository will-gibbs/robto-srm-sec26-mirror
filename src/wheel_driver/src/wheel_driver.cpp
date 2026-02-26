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
      auto velocity_callback = [this](const geometry_msgs::msg::Twist::ConstSharedPtr vel) -> void
      {
         // Log an event
         // RCLCPP_INFO(rclcpp::Logger logger, std::string log_message)
         RCLCPP_INFO(get_logger(), "Event received.");
      };
      
      // Create a subscriber on the node
      const rclcpp::QoS qos = rclcpp::QoS(rclcpp::KeepLast(7)).reliable(); // From Turtlesim
      // create_subscriber(const std::string &topic_name, rclcpp::QoS QoS, CallbackT && callback)
      subscr = create_subscription<geometry_msgs::msg::Twist>(/*`real_name "/" + `?*/"cmd_vel", /*`10`?*/qos, velocity_callback);
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
