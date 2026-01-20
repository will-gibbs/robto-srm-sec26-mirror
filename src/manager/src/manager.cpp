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
#include "rclcpp/rclcpp.hpp"
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
   int                 round_has_started;     // Indicates whether the round has begun
   ControllerReference *p_controllers;        // List of Robto's controllers
   Service             register_controler;    // ?
   Service             start_round;           // Begins Robto's mission
   Subscription<builtin_interfaces::msg::Duration>::SharedPointer
                       round_time_subscriber; // Subscription to the time remaining in the round

   // ?
   void register_controller_callback(
      const sec_interfaces::srv::RegisterController::Request  request,
            sec_interfaces::srv::RegisterController::Response response
   );
public:
   // Constructor
   Manager();
};

//******************************************************************************
//* Manager Class Constructor *
//******************************************************************************
Manager::Manager()
{

}

//******************************************************************************
//* ? *
//******************************************************************************
void Manager::register_controller_callback(
   const sec_interfaces::srv::RegisterController::Request  request,
         sec_interfaces::srv::RegisterController::Response response
)
{
   
   return;
}