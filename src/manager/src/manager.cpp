//******************************************************************************
//* Project: Robto SRM, IEEE SoutheastCon 2026                                 *
//* Package: Manager                                                           *
//* Name:    manager.cpp                                                       *
//******************************************************************************

//******************************************************************************
//* This program is the implementation of Robto's Manager node. The manager is *
//* responsible for starting the mission, registering Robto's controllers      *
//* (subsystems responsible for completing tasks), sorting them according to   *
//* priority, and granting them the almighty Talking Stck.                     *
//*                                                                            *
//* Since ControllerReference objects contain action clients and service       *
//* servers but are not nodes, the Manager will create the                     *
//* controller_complete_task action client and the controller_update_task      *
//* service server when the controller is registered with the manager.         *
//*                                                                            *
//* Services:                                                                  *
//* - start_round                                                              *
//* - register_controller                                                      *
//*                                                                            *
//* Subscriptions:                                                             *
//* - round_time                                                               *
//******************************************************************************

// C++-specific packages
#include <iostream>
#include <memory>
#include <chrono>
#include <vector>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "builtin_interfaces/msg/duration.hpp"

// Custom packages
#include "sec_interfaces/srv/register_controller.hpp"
#include "sec_interfaces/srv/start_round.hpp"
#include "sec_interfaces/action/complete_task.hpp"
#include "sec_core_classes/controller_reference.hpp"

// Namespaces
using namespace std;
using namespace rclcpp;
using namespace std::chrono_literals;

// Aliases
using CompleteTask         = sec_interfaces::action::CompleteTask;
using RegisterController   = sec_interfaces::srv::RegisterController;
using StartRound           = sec_interfaces::srv::StartRound;
using UpdateTask           = sec_interfaces::srv::UpdateTask;

// Symbolic constants
#define TICK_RATE 100ms

//******************************************************************************
//*                              Class Definition                              *
//******************************************************************************
// Robto's central processing manager node
class Manager : public Node
{
   int                                                        round_has_started = 0; // Indicates whether the round has begun
   vector<shared_ptr<ControllerReference>>                    controllers;           // List of pointers to Robto's controllers
   Subscription<builtin_interfaces::msg::Duration>::SharedPtr round_time_subscriber; // Subscription to the time remaining in the round
   Service<RegisterController>::SharedPtr                     register_controller;   // Service for registering new controllers
   Service<StartRound>::SharedPtr                             start_round;           // Service for starting Robto's mission
   TimerBase::SharedPtr                                       timer;                 // Timer

   // Callback function for registering a new controller
   void register_controller_callback(
      const shared_ptr<RegisterController::Request>  request,
            shared_ptr<RegisterController::Response> response
   );
   // Callback function for starting Robto's mission
   void start_round_callback(
      const shared_ptr<StartRound::Request>  request,
            shared_ptr<StartRound::Response> response
   );
   // Callback funtion for the timer
   void timer_callback();
   // Add a new controller reference to the list
   void add_controller(shared_ptr<ControllerReference> new_controller);
   // Sort the controllers into descending order of priority
   void sort_controllers();
public:
   // Constructor
        Manager();
};

//******************************************************************************
//*                         Manager Class Constructor                          *
//******************************************************************************
Manager::Manager() : Node("manager")
{
   // Create services for registering controllers and starting the round
   register_controller = create_service<RegisterController>
   (
      "register_controller",
      [this](const shared_ptr<RegisterController::Request>  request,
                   shared_ptr<RegisterController::Response> response) 
      { Manager::register_controller_callback(request, response); }
   );
   start_round = create_service<StartRound>
   (
      "start_round",
      [this](const shared_ptr<StartRound::Request>  request,
                   shared_ptr<StartRound::Response> response) 
      { Manager::start_round_callback(request, response); }
   );

   // Create a subscriber for getting the time remaining in the mission
   round_time_subscriber = create_subscription<builtin_interfaces::msg::Duration>
   (
      "round_time", 10, [this](builtin_interfaces::msg::Duration::UniquePtr message) 
      { RCLCPP_INFO(get_logger(), "Message: %d, %d", message->sec, message->nanosec); }
   );

   // Create a timer that will run the given callback function after every designated interval
   timer = create_wall_timer(TICK_RATE, [this]() {timer_callback();});
}

//******************************************************************************
//*                   Register a new controller upon request                   *
//******************************************************************************
void Manager::register_controller_callback(
   const shared_ptr<RegisterController::Request>  request,
         shared_ptr<RegisterController::Response> response)
{
   // TODO: Add error handling

   shared_ptr<ControllerReference>                new_controller_reference  = std::make_shared<ControllerReference>(request->controller_action_name);
                                                                                                        // New controller reference being added to the list
   weak_ptr<ControllerReference>                  weak_controller_reference = new_controller_reference; // Weak pointer to the new controller reference to avoid memory leaks
   Service<UpdateTask>::SharedPtr                 new_controller_service;                               // Update task service server for the new controller
   rclcpp_action::Client<CompleteTask>::SharedPtr new_controller_action_client;                         // Complete task action client for the new controller
   
   // Add the new controller to the list
   add_controller(new_controller_reference);
   RCLCPP_INFO(get_logger(), "Controller %s is online.", request->controller_action_name.c_str());

   // Calculate the new controller's priority

   // Create the controller_update_task service server for the new controller reference
   new_controller_service = create_service<UpdateTask>
   (
      "controller_update_task/" + new_controller_reference->get_controller_name(),
      [this, weak_controller_reference](const shared_ptr<UpdateTask::Request>  request,
                                              shared_ptr<UpdateTask::Response> response)
      { 
         if (auto strong_controller_reference = weak_controller_reference.lock())
         {
            strong_controller_reference->controller_update_task_callback(request, response);
         }
      }
   );
   new_controller_reference->set_controller_update_task(new_controller_service);

   // Create the controller_complete_task action client
        new_controller_action_client   = rclcpp::create_client<CompleteTask>(this, "complete_task/" + new_controller_reference->get_controller_name()); // ? What are these parameters?
   auto complete_task_goal             = CompleteTask::Goal();
        complete_task_goal->begin_task = complete_task_goal->BEGIN_TASK;
   auto complete_task_callbacks        = rclcpp_action::Client<CompleteTask>::SendGoalOptions;

   // Create callbacks for the action client
   complete_task_callbacks.goal_response_callback   = [this]() { RCLCPP_INFO(get_logger(), "The %s complete task action received a goal.",                       new_controller_reference->get_controller_name()); };
   complete_task_callbacks.cancel_response_callback = [this]() { RCLCPP_INFO(get_logger(), "The %s complete task action responded to a cancel results request.", new_controller_reference->get_controller_name()); };
   complete_task_callbacks.feedback_callback        = [this]() { RCLCPP_INFO(get_logger(), "The %s complete task action delivered feedback.",                    new_controller_reference->get_controller_name()); };
   complete_task_callbacks.result_callback          = [this]() { RCLCPP_INFO(get_logger(), "The %s complete task action published results.",                     new_controller_reference->get_controller_name()); };

   new_controller_reference->set_controller_complete_task(new_controller_action_client);

   // Set the status of the registration in the service response for the new controller reference
   response->registration_status_code = response->OK;
   return;
}

//******************************************************************************
//*                     Start Robto's mission upon request                     *
//******************************************************************************
void Manager::start_round_callback(
   const shared_ptr<StartRound::Request>  request,
         shared_ptr<StartRound::Response> response)
{
   if (request->start_round == request->START_ROUND)
   {
      round_has_started        = 1;
      response->manager_status = response->OK;
      RCLCPP_INFO(get_logger(), "Starting Robto's mission. Hang in there, Astroducks!");
      // ? Start round logic
   }
   else
   {
      response->manager_status = response->ERROR;
      RCLCPP_INFO(get_logger(), "Something went wrong with starting the round.");
   }
   return;
}

//******************************************************************************
//*                      Add a new controller to the list                      *
//******************************************************************************
void Manager::add_controller(shared_ptr<ControllerReference> new_controller)
{
   controllers.push_back(new_controller);
   sort_controllers();
   return;
}

//******************************************************************************
//*             Sort controllers into descending order of priority             *
//******************************************************************************
void Manager::sort_controllers()
{
   sort(controllers.begin(), controllers.end(), 
      [](auto& first_controller, auto& last_controller)
      {
         return first_controller->get_task_priority() > last_controller->get_task_priority();
      });
   return;
}

//******************************************************************************
//*               Callback function for every tick of the timer                *
//******************************************************************************
void Manager::timer_callback()
{
   RCLCPP_INFO(get_logger(), "AAAAAAAAHHHHHHHH");

   /*if (round_has_started)
   {

   }*/

   return;
}

//******************************************************************************
//*                               Main Function                                *
//******************************************************************************
int main(int argc, char **argv)
{
   // Initialize ROS2 C++ client library
   init(argc, argv);

   // Print a message indicating that the maager is ready
   RCLCPP_INFO(get_logger("rclcpp"), "Robto's Manager is online and ready to go.");

   // Spin up the node
   spin(make_shared<Manager>());
   shutdown();
   return 0;
}

// TODO:
// - Finish action client and adjust controller_reference.hpp
// - Action client CMake and package.xml file updates if necessary
// - Add logging
// - Polish documentation