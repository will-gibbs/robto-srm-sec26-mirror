// C++-specific packages
#include <memory>
#include <stdexcept>

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
   // Interface type aliases
   using UpdateTask = sec_interfaces::srv::UpdateTask;
   using CompleteTask = sec_interfaces::action::CompleteTask;

   // Action server for the complete task action on the controller
   rclcpp_action::Client<CompleteTask>::SharedPtr controller_complete_task;

   // Service server for updating the task values of the controller
   Service<UpdateTask>::SharedPtr                 controller_update_task;

   // The name of the complete task action on the controller
   string                                         controller_name;

                                                  // Priority value of the controller's task
   float                                          task_priority,
                                                  // Point-value value of the controller's task
                                                  task_point_value,
                                                  // Time-to-complete value of the controller's task
                                                  task_time_to_complete,
                                                  // Likelihood-of-success value of the controller's task
                                                  task_likelihood_of_success;

public:
   // Constructor, create a controller reference
   ControllerReference(const string name)
   {
      // Initilize the 
      controller_name = name;
   }

   // Set the member variables
   void   set_task_priority             (float new_pr)  {task_priority = new_pr;};
   void   set_task_point_value          (float new_pv)  {task_point_value = new_pv;};
   void   set_task_time_to_complete     (float new_ttc) {task_time_to_complete = new_ttc;};
   void   set_task_likelihood_of_success(float new_los) {task_likelihood_of_success = new_los;};

   // Set the update task service server for the controller
   void   set_controller_update_task    (Service<UpdateTask>::SharedPtr srv)                 {controller_update_task = srv;};

   // Set the complete task action client for the controller
   void   set_controller_complete_task  (rclcpp_action::Client<CompleteTask>::SharedPtr cli) {controller_complete_task = cli;};

   // Get the member variables
   float  get_task_priority             ()              {return task_priority;};
   float  get_task_point_value          ()              {return task_point_value;};
   float  get_task_time_to_complete     ()              {return task_time_to_complete;};
   float  get_task_likelihood_of_success()              {return task_likelihood_of_success;};
   string get_controller_name           ()              {return controller_name;};

   // Update a controller's task values at the controller's request
   void controller_update_task_callback(const shared_ptr<UpdateTask::Request> request, shared_ptr<UpdateTask::Response> response)
   {
      try
      {
         if (request->new_task_likelihood_of_success > 1.00f)
         {
            response->request_status = response->RETRY;
         }
         else
         {
            // Change controller task values to new values
            set_task_point_value          ((float)request->new_task_point_value);
            set_task_time_to_complete     ((float)request->new_task_time_to_complete);
            set_task_likelihood_of_success((float)request->new_task_likelihood_of_success);

            // Reply with a status code of OK
            response->request_status = response->OK;
         }
      }
      catch (...)
      {
         // Reply with a status code of RETRY
         response->request_status = response->RETRY;
      }
   }
};