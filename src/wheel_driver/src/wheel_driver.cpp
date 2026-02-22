// C++-specific packages
#include <memory>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std;
using namespace rclcpp;

// The definition of a node that subscribes to a topic of odd and even integers
class OddEvenSubscriber : public Node
{
   Subscription<std_msgs::msg::String>::SharedPtr subscription;
public:
   // Constructor, create an odd-even subscriber node; call the Node constructor
   // Node(const std::string &node_name)
   OddEvenSubscriber() : Node("ethan_black_subscriber")
   {
      // Callback function to be called every time the subscriber receives a message
      auto topic_callback = [this](std_msgs::msg::String::UniquePtr message) -> void
      {
         // Log an event
         // RCLCPP_INFO(rclcpp::Logger logger, std::string log_message)
         RCLCPP_INFO(get_logger(), "I have heard the next number in the sequence, and that number is %s.", message->data.c_str());
      };

      // Create a subscriber on the node
      // create_subscriber(const std::string &topic_name, rclcpp::QoS QoS, CallbackT && callback)
      subscription = create_subscription<std_msgs::msg::String>("ethan_black_topic", 10, topic_callback);
   }
};

int main(int argc, char * argv[])
{
   init(argc, argv);

   // Start up the node
   spin(make_shared<OddEvenSubscriber>());

   // Clean up
   shutdown();
   
   return 0;
}
