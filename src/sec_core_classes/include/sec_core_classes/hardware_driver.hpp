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
   gpiod::chip chip;
public:
   // Constructor, create a hardware driver
   HardwareDriver(const string name) : Node(name), chip("gpiochip0")
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
   gpiod::line set_pin_mode(int pin_number, int direction)
   {
      RCLCPP_INFO(get_logger(), "Setting pin mode: pin_number=%d, direction=%d", pin_number, direction);

      gpiod::line line = chip.get_line(pin_number);

      gpiod::line_request config;
      config.consumer = "driver";
      switch (direction)
      {
         case (int)gpiod::line_request::DIRECTION_AS_IS:
            RCLCPP_INFO(get_logger(), "Pin mode=AS IS");
            config.request_type = gpiod::line_request::DIRECTION_AS_IS;
            break;
         case (int)gpiod::line_request::DIRECTION_INPUT:
            RCLCPP_INFO(get_logger(), "Pin mode=INPUT");
            config.request_type = gpiod::line_request::DIRECTION_INPUT;
            break;
         case (int)gpiod::line_request::DIRECTION_OUTPUT:
            RCLCPP_INFO(get_logger(), "Pin mode=OUTPUT");
            config.request_type = gpiod::line_request::DIRECTION_OUTPUT;
            break;
         default:
            RCLCPP_INFO(get_logger(), "Pin mode=UNKNOWN");
            break;
      }

      line.request(config, 0);

      return line;
   }

   // Set the modes of all gpio pins the driver uses
   virtual int init_gpios()
   {
      RCLCPP_INFO(get_logger(), "Initializing gpio pins...");

      return 0;
   }
};

#endif
