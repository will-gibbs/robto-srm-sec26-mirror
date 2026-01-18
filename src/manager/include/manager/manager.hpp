//******************************************************************************
//* Project: Robto SRM, IEEE SoutheastCon 2026                                 *
//* Package: Manager                                                           *
//* Name:    manager.hpp                                                       *
//******************************************************************************

//******************************************************************************
//* This is the Manager class definition. The Manager class controls Robto's   *
//* central processing and starts and stops Robto's other subsystems.          *
//******************************************************************************

#include "rclcpp/rclcpp.hpp"
// TODO: Future imports
// ControllerReference
// RegisterController
using namespace std;
using namespace rclcpp;

// Robto's central processing manager
class Manager : public Node
{
   ControllerReference *p_controllers;        // ?
   Service             register_controler;    // ?
   Subscription<builtin_interfaces::msg::Duration>::SharedPointer
                       round_time_subscriber; // ?

   // ?
   void register_controller_callback(
      const sec_interfaces::srv::RegisterController::Request  request,
            sec_interfaces::srv::RegisterController::Response response
   );
public:
   // Constructor
   Manager();
};