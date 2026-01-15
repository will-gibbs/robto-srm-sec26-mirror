// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "sec_interfaces/srv/register_controller.hpp"
#include "sec_interfaces/srv/update_task.hpp"

// Namespaces
using namespace std;
using namespace std::chrono_literals;
using namespace rclcpp;
using namespace sec_interfaces::srv;

class Controller : public Node
{
    
};