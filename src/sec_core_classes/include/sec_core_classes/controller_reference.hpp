// C++-specific packages
#include <memory>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

// Custom packages
#include "sec_interfaces/srv/update_task.hpp"
#include "sec_interfaces/action/complete_task.hpp"

// Namespaces
using namespace std;
using namespace rclcpp;

// The definition of a controller reference
class ControllerReference
{
   using UpdateTask = sec_interfaces::srv::UpdateTask;
   using CompleteTask = sec_interfaces::action::CompleteTask;

   rclcpp_action::Client<CompleteTask>::SharedPtr controller_complete_task;

   Service<UpdateTask>::SharedPtr controller_update_task;

   string controller_action_name;

   float task_priority,
         task_point_value,
         task_time_to_complete,
         task_likelihood_of_success;

   // Update a controller's task values at the controller's request
   void controller_update_task_callback(const UpdateTask::Request  request,
                                              UpdateTask::Response response)
   {
      
   }
public:
   // Constructor, create a controller reference
   ControllerReference(const string action_name)
   {
      controller_action_name = action_name;

      RCLCPP_INFO(get_logger(), "Creating a controller with an action name of '%s'...",
         controller_action_name.c_str());
   }

   // Get the complete task action client for the controller
   rclcpp_action::Client<CompleteTask>::SharedPtr get_controller_complete_task() {return controller_complete_task;};

   // Get the update task service server for the controller
   Service<UpdateTask>::SharedPtr get_controller_update_task() {return controller_update_task;};

   // Set the member variables
   void  set_task_priority             (float new_pr)  {task_priority = new_pr;};
   void  set_task_point_value          (float new_pv)  {task_point_value = new_pv;};
   void  set_task_time_to_complete     (float new_ttc) {task_time_to_complete = new_ttc;};
   void  set_task_likelihood_of_success(float new_los) {task_likelihood_of_success = new_los;};

   // Get the member variables
   float get_task_priority             ()              {return task_priority;};
   float get_task_point_value          ()              {return task_point_value;};
   float get_task_time_to_complete     ()              {return task_time_to_complete;};
   float get_task_likelihood_of_success()              {return task_likelihood_of_success;};
};