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
   gpiod::chip chip; // The chip to use for GPIO communication

   string driver_name; // The name of the driver node to be used for line requests
public:
   // Constructor, create a hardware driver
   HardwareDriver(const string name) : Node(name), chip("gpiochip0")
   {
      // Log node creation
      RCLCPP_INFO(get_logger(), "Creating a hardware driver: '%s'.", name.c_str());
      driver_name = name;
      
      // Initialize GPIO pins
      if (init_gpios() == 0)
      {
        RCLCPP_INFO(get_logger(), "Successfully initialized GPIO pins.");
      }
      else
      {
         RCLCPP_ERROR(get_logger(), "Error: failed to initialize GPIO pins. Exiting.");
         exit(1);
      }
   }

   // Destructor, delete a hardware driver
   ~HardwareDriver()
   {
      // Log node deletion
      RCLCPP_INFO(get_logger(), "Deleting a hardware driver: '%s'.", driver_name.c_str());
   }

   // Set the mode of a specific GPIO pin
   gpiod::line set_pin_mode(int pin_number, int direction)
   {
      // Log pin initialization
      RCLCPP_INFO(get_logger(), "Setting pin mode: pin_number=%d, direction=%d", pin_number, direction);

      // Get a line for the GPIO pin number
      gpiod::line line = chip.get_line(pin_number);

      // Create the line request configuration
      gpiod::line_request config;
      config.consumer = driver_name;
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
            RCLCPP_ERROR(get_logger(), "Failed to initialize GPIO pin %d: unknown direction %d.",
               pin_number, direction);
            RCLCPP_ERROR(get_logger(), "Exiting.");
            exit(1);
            break;
      }

      // Request the line with the created configuration
      line.request(config, 0);

      return line;
   }

   // Set the modes of all GPIO pins the driver uses
   virtual int init_gpios()
   {
      RCLCPP_INFO(get_logger(), "Initializing GPIO pins...");

      return 0;
   }
};

#endif
