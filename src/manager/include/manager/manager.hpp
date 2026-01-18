//******************************************************************************
//* Project: Robto SRM, IEEE SoutheastCon 2026                                 *
//* Package: Manager                                                           *
//* Name:    manager.hpp                                                       *
//******************************************************************************

//******************************************************************************
//* This is the Manager class definition. The Manager class controls Robto's   *
//* central processing and starts and stops Robto's other subsystems.          *
//******************************************************************************

#include <iostream>
#include "rclcpp/rclcpp.hpp"
// TODO: Future imports
// ControllerReference
// RegisterController
using namespace std;
using namespace rclcpp;

// Robto's central processing manager
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