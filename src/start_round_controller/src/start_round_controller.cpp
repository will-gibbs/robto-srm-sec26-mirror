// C++-specific packages
#include <functional>
#include <memory>
#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sec_core_classes/controller.hpp"

using namespace std;
using namespace std::chrono_literals;
using namespace rclcpp;

class StartRoundController : public Controller
{
public:
   using CompleteTask = sec_interfaces::action::CompleteTask;
   using GoalHandleCompleteTask = rclcpp_action::ServerGoalHandle<CompleteTask>;
   using Twist = geometry_msgs::msg::Twist;

   StartRoundController() : Controller("start_round_controller", 100ms)
   {
      RCLCPP_INFO(get_logger(), "Creating a start round controller...");
   };

   ~StartRoundController() {};

   rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID & uuid, shared_ptr<const CompleteTask::Goal> goal)
   {
      RCLCPP_INFO(get_logger(), "Goal received with begin task value of %d", goal->begin_task);
      (void)uuid;
      return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
   };

   rclcpp_action::CancelResponse handle_cancel(const shared_ptr<GoalHandleCompleteTask> goal_handle)
   {
      RCLCPP_INFO(get_logger(), "Goal cancelled.");
      (void)goal_handle;
      return rclcpp_action::CancelResponse::ACCEPT;
   };

   void handle_accepted(const shared_ptr<GoalHandleCompleteTask> goal_handle)
   {
      RCLCPP_INFO(get_logger(), "Goal accepted.");
      auto execute_in_thread = [this, goal_handle](){return this->execute(goal_handle);};
      thread{execute_in_thread}.detach();
   };

   void execute(const shared_ptr<GoalHandleCompleteTask> goal_handle)
   {
      RCLCPP_INFO(get_logger(), "Executing goal...");
      Rate loop_rate(1);
      const auto goal = goal_handle->get_goal();

      auto feedback = std::make_shared<CompleteTask::Feedback>();
      auto result = std::make_shared<CompleteTask::Result>();

      for (int i = 10; i >= 1; i--)
      {
         if (goal_handle->is_canceling())
         {
            result->task_complete = 0;
            goal_handle->canceled(result);
            RCLCPP_INFO(get_logger(), "Goal cancelled.");
            return;
         }

         feedback->time_remaining.sec = i;
         feedback->time_remaining.nanosec = 0;
         goal_handle->publish_feedback(feedback);
         RCLCPP_INFO(get_logger(), "Publishing feedback: %d sec., %d nanosec. remaining.",
            feedback->time_remaining.sec, feedback->time_remaining.nanosec);
         
         loop_rate.sleep();
      }
      if (ok())
      {
         result->task_complete = result->TASK_COMPLETE;
         RCLCPP_INFO(get_logger(), "Goal succeeded!");
      }
   };
private:
   void timer_callback()
   {
      RCLCPP_INFO(get_logger(), "Timer tick");
   };
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   spin(make_shared<StartRoundController>());

   shutdown();

   return 0;
}