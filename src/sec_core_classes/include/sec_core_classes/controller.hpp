//******************************************************************************
//* Project: Robto SRM, IEEE SoutheastCon 2026                                 *
//* Package: Controller                                                        *
//* Name:    controller.hpp                                                       *
//******************************************************************************

//******************************************************************************
//* This program is the implementation of Robto's Controller node. The         *
//* Controller will act as an interface for individual controllers focused on  *
//* completing specific tasks. Any controller inheriting from this will need   *
//* to implement the handle_goal, handle_cancel, handle_accepted, and execute  *
//* functions.                                                                 *
//*                                                                            *
//* Services:                                                                  *
//* - action_server                                                            *
//* -                                                                          *
//******************************************************************************

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

//******************************************************************************
//*                              Class Definition                              *
//******************************************************************************
// Robto's controller interface node
class Controller : public Node
{
public:
    using CompleteTask = sec_interfaces::action::CompleteTask;
    using GoalHandleCompleteTask = rclcpp_action::ServerGoalHandle<CompleteTask>;

    // Receives the objective to complete
    virtual rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID & uuid, shared_ptr<const CompleteTask::Goal> goal)
    {
        RCLCPP_INFO(get_logger(), "Received goal with begin task value of %d", goal->begin_task);
        (void)uuid;
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    };

    // Handles canceling the goal
    virtual rclcpp_action::CancelResponse handle_cancel(const shared_ptr<GoalHandleCompleteTask> goal_handle)
    {
        RCLCPP_INFO(get_logger(), "Received cancel request.");
        (void)goal_handle;
        return rclcpp_action::CancelResponse::ACCEPT;
    };

    // Determines whether or not the goal is acceptable
    virtual void handle_accepted(const shared_ptr<GoalHandleCompleteTask> goal_handle)
    {
        RCLCPP_INFO(get_logger(), "Accepted a goal.");
        auto execute_in_thread = [this, goal_handle](){return this->execute(goal_handle);};
        thread{execute_in_thread}.detach();
    };

    // Executes the goal
    virtual void execute(const shared_ptr<GoalHandleCompleteTask> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Executing goal");
        const auto goal = goal_handle->get_goal();
    }

    // Constructs controller objects
    Controller(const string name, std::chrono::milliseconds tick_rate) : Node(name)
    {
        controller_name = name;
        timer_tick_rate = tick_rate;

        this->complete_task = rclcpp_action::create_server<CompleteTask>(
            this,
            name + "/complete_task",
            [this](const auto & uuid, const auto & goal) {return handle_goal(uuid, goal);},
            [this](const auto & goal_handle) {return handle_cancel(goal_handle);},
            [this](const auto & goal_handle) {handle_accepted(goal_handle);});
        
        update_task = create_client<UpdateTask>(name + "/update_task");

        timer = create_wall_timer(timer_tick_rate, [this]() {return timer_callback();});
    };
private:
    rclcpp_action::Server<CompleteTask>::SharedPtr    complete_task;
    Client<RegisterController>::SharedPtr             register_controller;
    Client<UpdateTask>::SharedPtr                     update_task;
    string                                            controller_name;
    TimerBase::SharedPtr                              timer;
    std::chrono::milliseconds                         timer_tick_rate;

    // Loops to perform a task
    virtual void timer_callback() {};
};