//******************************************************************************
//* Project:      Robto SRM, IEEE SoutheastCon 2026                            *
//* Package:      sec_core_classes                                             *
//* Name:         ControllerReference                                          *
//* Written by:   Will Gibbs                                                   *
//* Written date: 2026-01-22                                                   *
//******************************************************************************

//******************************************************************************
//* This header file contains the definition of the controller reference       *
//* class. Each controller reference refers to one specific controller. It     *
//* contains the service server for the controller to update its task values,  *
//* the action client for the controller to complete its task, and the         *
//* controller's task values.                                                  *
//* The manager will create a new controller reference when a controller       *
//* registers, and will use the complete task action client to grant the       *
//* controller permission to complete its task (aka, pass the talking stick).  *
//* The controller reference handles updating the controller's task values     *
//* (at the controller's request).                                             *
//*                                                                            *
//* Services:                                                                  *
//* - controller_update_task (service name unique to each controller)          *
//*                                                                            *
//* Action Clients:                                                            *
//* - controller_complete_task (action name unique to each controller)         *
//*                                                                            *
//* Note: a ControllerReference object has as members a service server and an  *
//* action client, but it is not a Node. Therefore, the manager will create    *
//* the service server and action client when it registers a new controller    *
//* (refer to `controller_reference_test.cpp` for an example of this).         *
//******************************************************************************

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

//******************************************************************************
//*                              Class Definition                              *
//******************************************************************************
// The definition of a reference to a controller
class ControllerReference
{
   // Interface type aliases
   using UpdateTask = sec_interfaces::srv::UpdateTask;
   using CompleteTask = sec_interfaces::action::CompleteTask;
   using GoalHandle = rclcpp_action::ClientGoalHandle<CompleteTask>;
   using WrappedResult = rclcpp_action::Client<CompleteTask>::WrappedResult;

   // Action server for the complete task action on the controller
   rclcpp_action::Client<CompleteTask>::SharedPtr controller_complete_task;

   // Goal handle (point of reference and control), feedback, and result of the action request
   GoalHandle::SharedPtr                          current_goal;
   CompleteTask::Feedback::SharedPtr              current_feedback;
   WrappedResult::SharedPtr                       current_result;

   // Service server for updating the task values of the controller
   Service<UpdateTask>::SharedPtr                 controller_update_task;

   // The name of the complete task action on the controller
   string controller_name;

   float  task_priority,              // Priority value of the controller's task
          
          task_point_value,           // Point-value value of the controller's task
          
          task_time_to_complete,      // Time-to-complete value of the controller's task
          
          task_likelihood_of_success; // Likelihood-of-success value of the controller's task

   // Callback functions for the complete_task action call
   void on_goal_response(GoalHandle::SharedPtr goal_handle)
   {
      if (goal_handle)
      {
         // Complete task goal was accepted; save a reference to the active goal
         RCLCPP_INFO(get_logger(), "Complete task goal for %s was accepted.", controller_name.c_str());
         current_goal = goal_handle;
      }
      else
      {
         // Complete task goal was rejected
         RCLCPP_INFO(get_logger(), "Complete task goal for %s was rejected.", controller_name.c_str());
      }
   };
   void on_feedback(CompleteTask::Feedback::SharedPtr feedback)
   {
      current_feedback = feedback
      RCLCPP_INFO(get_logger(), "Feedback received from the task %s.", controller_name.c_str());
   };
   void on_result  (WrappedResult result)   
   {
      current_result   = result
      switch (result.code)
      {
         case rclcpp_action::ResultCode::SUCCEEDED:
            RCLCPP_INFO(get_logger(), "The task %s has been successfully completed!", controller_name.c_str());
            break;
         case rclcpp_action::ResultCode::ABORTED:
            RCLCPP_INFO(get_logger(), "The task %s has been aborted.",                controller_name.c_str());
            break;
         case rclcpp_action::ResultCode::CANCELED:
            RCLCPP_INFO(get_logger(), "The task %s has been canceled.",               controller_name.c_str());
            break;
      }
   };

public:
   // Constructor, create a controller reference
   ControllerReference(const string name)
   {
      // Initilize member variables
      controller_name            = name;
      task_priority              = 0.0f;
      task_point_value           = 0.0f;
      task_time_to_complete      = 0.0f;
      task_likelihood_of_success = 0.0f;
   }

   // Set the member variables
   void   set_task_priority             (float new_pr)  {task_priority = new_pr;};
   void   set_task_point_value          (float new_pv)  {task_point_value = new_pv;};
   void   set_task_time_to_complete     (float new_ttc) {task_time_to_complete = new_ttc;};
   void   set_task_likelihood_of_success(float new_los) {task_likelihood_of_success = new_los;};

   // Set the update task service server for the controller
   void   set_controller_update_task    (Service<UpdateTask>::SharedPtr srv) 
      {controller_update_task = srv;};

   // Set the complete task action client for the controller
   void   set_controller_complete_task  (rclcpp_action::Client<CompleteTask>::SharedPtr cli)
      {controller_complete_task = cli;};

   // Get the member variables
   float  get_task_priority             ()              {return task_priority;};
   float  get_task_point_value          ()              {return task_point_value;};
   float  get_task_time_to_complete     ()              {return task_time_to_complete;};
   float  get_task_likelihood_of_success()              {return task_likelihood_of_success;};
   string get_controller_name           ()              {return controller_name;};

   // Get the complete task action client for the controller
   rclcpp_action::Client<CompleteTask>::SharedPtr get_controller_complete_task()
      {return controller_complete_task;};

   // Update a controller's task values at the controller's request
   void controller_update_task_callback(const shared_ptr<UpdateTask::Request>  request,
                                              shared_ptr<UpdateTask::Response> response)
   {
      try // Attempt to update the controller's task values
      {
         // Validate new task values
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
      catch (...) // The controller reference fails to update the task values
      {
         // Reply with a status code of RETRY
         response->request_status = response->RETRY;
      }
   }

   // Send a task completion request to the controller
   void request_complete_task()
   {
      CompleteTask::Goal                                   goal;    // Goal for the CompleteTask action client
      rclcpp_action::Client<CompleteTask>::SendGoalOptions options; // Callbacks for responses from the action server

      // Set the callback functions
      options.goal_response_callback   = [this](auto goal_handle) { on_goal_response  (goal_handle); };
      options.feedback_callback        = [this](auto goal_handle) { on_feedback       (goal_handle); };
      options.cancel_response_callback = [this](auto goal_handle) { on_cancel_response(goal_handle); };
      options.result_callback          = [this](auto goal_handle) { on_result         (goal_handle); };

      // Set a goal and request the task to be completed
      goal.begin_task = CompleteTask::BEGIN_TASK;
      controller_complete_task->async_send_goal(goal, options);
   }

   // Cancel the controller task completion
   void cancel_complete_task()
   {
      if (active_goal)
      {
         controller_complete_task->async_cancel_goal(current_goal);
         RCLCPP_INFO(get_logger(), "The %s task was canceled.", controller_name.c_str());
      }
      else
      {
         RCLCPP_INFO(get_logger(), "Request to cancel the %s task, but it has not yet been started.", controller_name.c_str())
      }
   }
};