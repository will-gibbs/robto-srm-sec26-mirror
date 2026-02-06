#include <memory>

#include "rclcpp/rclcpp.hpp"

#include "sec_core_classes/hardware_driver.hpp"

using namespace std;
using namespace rclcpp;

class DriverTest : public HardwareDriver
{
   gpiod::line line;
public:
   DriverTest() : HardwareDriver("driver_test")
   {
      if (init_gpios() == 0)
      {
      	RCLCPP_INFO(get_logger(), "GPIO pins initialization successful.");
      }
      else
      {
         RCLCPP_ERROR(get_logger(), "Error: failed to initialize GPIO pins. Exiting");
	 exit(1);
      }
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
