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
//* Services:                                                                  *
//* - start_round                                                              *
//* - register_controller                                                      *
//******************************************************************************

#include <iostream>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "sec_interfaces/srv/register_controller.hpp"
#include "sec_interfaces/srv/start_round.hpp"
#include "sec_core_classes/controller_reference.hpp"
// TODO: Future imports
// ControllerReference
// RegisterController
using namespace std;
using namespace rclcpp;

// Aliases
using RegisterController = sec_interfaces::srv::RegisterController;
using StartRound = sec_interfaces::srv::StartRound;

//******************************************************************************
//*                              Class Definition                              *
//******************************************************************************
// Robto's central processing manager node
class Manager : public Node
{
   int                 round_has_started = 0; // Indicates whether the round has begun
   // ? For now, p_controllers will be an array of integers
   // ? All instances should be changed to ControllerReference object pointers once that class can be implemented
   vector<ControllerReference>         controllers;           // List of Robto's controllers
   Subscription<builtin_interfaces::msg::Duration>::SharedPointer
                       round_time_subscriber; // Subscription to the time remaining in the round

   Service<RegisterController>::SharedPointer register_controller;
   Service<StartRound>::SharedPointer         start_round;

   // ?
   void register_controller_callback(
      const RegisterController::Request  request,
            RegisterController::Response response
   );
   // ? start_round_callback();
   void start_round_callback(
      const StartRound::Request  request,
            StartRound::Response response
   );
   // Add a controller to the list
   void add_controller();
   // Sort the controllers into descending order of priority
   void sort_controllers();
public:
   // Constructor
   Manager();
};

//******************************************************************************
//* Manager Class Constructor *
//******************************************************************************
Manager::Manager() : Node("manager")
{
   // Create services for registering controllers and starting the round
   register_controller = 
      node->create_service<RegisterController>
      ("register_controller",
       [this](const RegisterController::Request  request,
                    RegisterController::Response response) {Manager::register_controller_callback(request, response);});
   start_round =
      node->create_service<StartRound>
         ("start_round",
         [this](const StartRound::Request  request,
                      StartRound::Response response) {Manager::start_round_callback(request, response);});

   // Create round time subscriber
   round_time_subscriber = create_subscription<builtin_interfaces::msg::Duration>("round_time", 10, void);
}

//******************************************************************************
//* ? *
//******************************************************************************
void Manager::register_controller_callback(
   const RegisterController::Request  request,
         RegisterController::Response response)
{
   // float priority; // Priority of the controller's task

   // ? Assign an arbitrary priority value
   // priority = (int)request->controller_action_name[0];

   // Create a new controller reference
   ControllerReference new_controller_reference = new ControllerReference(request->controller_action_name);

   // Add the new controller to the list
   add_controller(new_controller_reference); //? will be replaced with the ControllerReference object
   // ? TODO: Add error handling
   response->regisration_status_code == response->OK;
   RCLCPP_INFO(get_logger("rclcpp"), "Added the controller %s", request->controller_action_name);
   return;
}

//******************************************************************************
//*                           Start Robto's mission                            *
//******************************************************************************
void start_round_callback(
   const StartRound::Request  request,
         StartRound::Response response)
{
   if (request->start_round == request->START_ROUND)
   {
      round_has_started        = 1;
      response->manager_status = response->OK;
      RCLCPP_INFO(get_logger("rclcpp"), "Starting Robto's mission. Hang in there, Astroducks!");
      // ? Start round logic
   }
   else
   {
      response->manager_status = response->ERROR;
      RCLCPP_INFO(get_logger("rclcpp"), "Something went wrong with starting the round.");
   }
   return;
}

//******************************************************************************
//* Add a new controller to the list *
//******************************************************************************
void Manager::add_controller(ControllerReference new_controller)
{
   controllers.push_back(new_controller);
   sort_controllers();
   return;
}

//******************************************************************************
//* Sort controllers into descending order of priority *
//******************************************************************************
void Manager::sort_controllers()
{
   sort(controllers.begin(), controllers.end(), 
      [](auto& first_controller, auto& last_controller)
      {
         return first_controller->tast_priority > last_controller->task_priority;
      });
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
   RCLCPP_INFO(get_logger("rclcpp"), "Robto's Manager is on board and ready to go.");

   // Spin up the node
   spin(make_shared<Manager>());
   shutdown();
   return 0;
}