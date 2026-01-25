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
#include "sec_interfaces/srv/RegisterController.hpp" // ? Check this file name
#inlcude "sec_interfaces/srv/StartRound.hpp" // ? Same here
// TODO: Future imports
// ControllerReference
// RegisterController
using namespace std;
using namespace rclcpp;

//******************************************************************************
//*                              Class Definition                              *
//******************************************************************************
// Robto's central processing manager node
class Manager : public Node
{
   int                 round_has_started = 0; // Indicates whether the round has begun
   // ? For now, p_controllers will be an array of integers
   // ? All instances should be changed to ControllerReference object pointers once that class can be implemented
   vector<int>         controllers;           // List of Robto's controllers
   Subscription<builtin_interfaces::msg::Duration>::SharedPointer
                       round_time_subscriber; // Subscription to the time remaining in the round

   // ?
   void register_controller_callback(
      const sec_interfaces::srv::RegisterController::Request  request,
            sec_interfaces::srv::RegisterController::Response response
   );
   // ? start_round_callback();
   void start_round_callback(
      const sec_interfaces::srv::StartRound::Request  request,
            sec_interfaces::srv::StartRound::Response response
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
   Service<sec_interfaces::srv::RegisterController>::SharedPointer register_controller = 
      node->create_service<sec_interfaces::srv::RegisterController>("register_controller", &register_controller_callback);
   Service<sec_interfaces::srv::StartRound>::        SharedPointer start_round =
      node->create_service<sec_interfaces::srv::StartRound>        ("start_round",         &start_round_callback);
}

//******************************************************************************
//* ? *
//******************************************************************************
void Manager::register_controller_callback(
   const sec_interfaces::srv::RegisterController::Request  request,
         sec_interfaces::srv::RegisterController::Response response)
{
   int priority; // Priority of the controller's task

   // ? Assign an arbitrary priority value
   priority = (int)request->controller_action_name[0];

   // Add the new controller to the list
   add_controller(priority); //? will be replaced with the ControllerReference object
   // ? TODO: Add error handling
   response->regisration_status_code == OK;
   RCLCPP_INFO(get_logger("rclcpp"), "Added the controller %s", request->controller_action_name);
   return;
}

//******************************************************************************
//*                           Start Robto's mission                            *
//******************************************************************************
void start_round_callback(
   const sec_interfaces::srv::StartRound::Request  request,
         sec_interfaces::srv::StartRound::Response response)
{
   if (request->start_round == START_ROUND)
   {
      round_has_started        = 1;
      response->manager_status = OK;
      RCLCPP_INFO(get_logger("rclcpp"), "Starting Robto's mission. Hang in there, Astroducks!");
      // ? Start round logic
   }
   else
   {
      response->manager_status = ERROR;
      RCLCPP_INFO(get_logger("rclcpp"), "Something went wrong with starting the round.");
   }
   return;
}

//******************************************************************************
//* Add a new controller to the list *
//******************************************************************************
void Manager::add_controller(int new_controller)
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
         return first_controller > last_controller;
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