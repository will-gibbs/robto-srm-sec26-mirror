// This is bad code; this is not complete

// C++-specific packages
#include <functional>
#include <memory>
#include <chrono>
// #include <string>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "sec_interfaces/srv/register_controller.hpp"
#include "sec_interfaces/srv/update_task.hpp"
#include "sec_interfaces/action/complete_task.hpp"

// Namespaces
using namespace std;
using namespace std::chrono_literals;
using namespace std::placeholders;
using namespace rclcpp;
using namespace sec_interfaces::srv;

class Controller : public Node
{
public:
    using CompleteTask = sec_interfaces::action::CompleteTask;
    using GoalHandleCompleteTask = rclcpp_action::ServerGoalHandle<CompleteTask>;

    virtual rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID & uuid, shared_ptr<const CompleteTask::Goal> goal)
    {
        RCLCPP_INFO(get_logger(), "Received goal with begin task value of %d", goal->begin_task);
        (void)uuid;
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    };

    virtual rclcpp_action::CancelResponse handle_cancel(const shared_ptr<GoalHandleCompleteTask> goal_handle)
    {
        RCLCPP_INFO(get_logger(), "Received cancel request.");
        (void)goal_handle;
        return rclcpp_action::CancelResponse::ACCEPT;
    };

    virtual void handle_accepted(const shared_ptr<GoalHandleCompleteTask> goal_handle)
    {
        RCLCPP_INFO(get_logger(), "Accepted a goal.");
        auto execute_in_thread = [this, goal_handle](){return this->execute(goal_handle);};
        thread{execute_in_thread}.detach();
    };

    virtual void execute(const shared_ptr<GoalHandleCompleteTask> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Executing goal");
        const auto goal = goal_handle->get_goal();
    }

    Controller(const string name, std::chrono::milliseconds tick_rate) : Node(name)
    {
        action_name = name;
        timer_tick_rate = tick_rate;

        this->complete_task = rclcpp_action::create_server<CompleteTask>(
            this,
            name,
            [this](const auto & uuid, const auto & goal) {return handle_goal(uuid, goal);},
            [this](const auto & goal_handle) {return handle_cancel(goal_handle);},
            [this](const auto & goal_handle) {handle_accepted(goal_handle);});
        
        timer = create_wall_timer(timer_tick_rate, [this]() {return timer_callback();});
    };
private:
    rclcpp_action::Server<CompleteTask>::SharedPtr    complete_task;
    Client<RegisterController>::SharedPtr             register_controller;
    Client<UpdateTask>::SharedPtr                     update_task;
    string                                            action_name;
    TimerBase::SharedPtr                              timer;
    std::chrono::milliseconds                         timer_tick_rate;

    virtual void timer_callback() {};
};