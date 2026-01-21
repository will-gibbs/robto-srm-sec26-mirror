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
   int                 round_has_started;     // Indicates whether the round has begun
   // ? For now, p_controllers will be an array of integers
   //   All instances should be changed to ControllerReference object pointers once that class can be implemented
   int                 *p_controllers;        // List of Robto's controllers
   Service             register_controler;    // ?
   Service             start_round;           // Begins Robto's mission
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
         sec_interfaces::srv::RegisterController::Response response)
{
   
   return;
}

//******************************************************************************
//*                           Start Robto's mission                            *
//******************************************************************************
// ? start_round_callback
void start_round_callback(
   const sec_interfaces::srv::StartRound::Request  request,
         sec_interfaces::srv::StartRound::Response response)
{
   if (request->start_round == START_ROUND)
   {
      this->round_has_started = 1;
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
//*                               Main Function                                *
//******************************************************************************
int main()
{

   return 0;
}