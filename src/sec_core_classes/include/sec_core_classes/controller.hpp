// This is bad code; this is not complete

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "sec_interfaces/srv/register_controller.hpp"
#include "sec_interfaces/srv/update_task.hpp"
#include <string>

// Namespaces
using namespace std;
using namespace std::chrono_literals;
using namespace rclcpp;
using namespace sec_interfaces::srv;

class Controller : public Node
{
    Server    complete_task;
    Client    register_controller;
    Client    update_task;
    string    action_name;
    TimerBase timer;

    virtual void timer_callback();

public:

    Controller(string name) : Node(name)
    {
        action_name = name;
        
        this->action_server_ = rclcpp_action::create_server<CompleteTask>(
            this,
            action_name,
            this->handle_goal,
            this->handle_cancel,
            this->handle_accepted);
    };

    virtual void handle_goal();

    virtual void handle_cancel();

    virtual void handle_accepted();
};