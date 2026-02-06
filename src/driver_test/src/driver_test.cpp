#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"

#include "sec_core_classes/hardware_driver.hpp"

using namespace std;
using namespace rclcpp;

class DriverTest : public HardwareDriver
{
   // GPIO line for the LED
   gpiod::line line;

   // Subscriber to blink the LED
   Subscription<std_msgs::msg::Bool>::SharedPtr blink_subscriber;
public:
   DriverTest() : HardwareDriver("driver_test")
   {
      // Callback for the blink subscriber
      auto blink_callback = [this](std_msgs::msg::Bool::UniquePtr message) -> void
      {
         if (message->data == 1)
         {
            // Turn the LED on
            RCLCPP_INFO(get_logger(), "LED on...");
	    line.set_value(1);
         }
         else
         {
            // Turn the LED off
            RCLCPP_INFO(get_logger(), "LED off...");
            line.set_value(0);
         }
      };

      if (init_gpios() == 0)
      {
      	RCLCPP_INFO(get_logger(), "GPIO pins initialization successful.");
      }
      else
      {
         RCLCPP_ERROR(get_logger(), "Error: failed to initialize GPIO pins. Exiting");
	 exit(1);
      }

      // Initialize the blink subscriber
      blink_subscriber = create_subscription<std_msgs::msg::Bool>
         ("blink", 10, blink_callback);
   }

   int init_gpios() override
   {
      line = set_pin_mode(4, (int)gpiod::line_request::DIRECTION_OUTPUT);

      return 0;
   }
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   spin(make_shared<DriverTest>());

   shutdown();

   return 0;
}
