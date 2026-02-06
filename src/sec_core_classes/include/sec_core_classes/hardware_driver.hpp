#ifndef HARDWARE_DRIVER_HPP
#define HARDWARE_DRIVER_HPP

// Symbolic constants
#define AS_IS  1
#define INPUT  2
#define OUTPUT 3

// C++-specific packages
#include <memory>
#include <gpiod.hpp>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"

// Namespaces --- NOS
using namespace std;
using namespace rclcpp;

// Defintion of a general hardware driver node
class HardwareDriver : public Node
{
   
public:
   // Constructor, create a hardware driver
   HardwareDriver(const string name) : Node(name)
   {
      RCLCPP_INFO(get_logger(), "Creating a hardware driver: '%s'.", name.c_str());
      init_gpios();
   }

   // Destructor, delete a hardware driver
   ~HardwareDriver()
   {
      RCLCPP_INFO(get_logger(), "Deleting a hardware driver");
   }

   // Set the mode of a specific gpio pin
   int set_pin_mode(int pin_number, int direction)
   {
      RCLCPP_INFO(get_logger(), "Setting pin mode: pin_number=%d, direction=%d", pin_number, direction);

      switch (direction)
      {
         case (int)gpiod::line_request::DIRECTION_AS_IS:
            RCLCPP_INFO(get_logger(), "Pin mode=AS IS");
            break;
         case (int)gpiod::line_request::DIRECTION_INPUT:
            RCLCPP_INFO(get_logger(), "Pin mode=INPUT");
            break;
         case (int)gpiod::line_request::DIRECTION_OUTPUT:
            RCLCPP_INFO(get_logger(), "Pin mode=OUTPUT");
            break;
         default:
            RCLCPP_INFO(get_logger(), "Pin mode=UNKNOWN");
            break;
      }

      return 0;
   }

   // Set the modes of all gpio pins the driver uses
   virtual int init_gpios()
   {
      RCLCPP_INFO(get_logger(), "Initializing gpio pins...");
      set_pin_mode(17, (int)gpiod::line_request::DIRECTION_OUTPUT);

      return 0;
   }
};

#endif