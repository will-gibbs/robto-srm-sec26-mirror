#include <memory>

#include "rclcpp/rclcpp.hpp"

#include "sec_core_classes/hardware_driver.hpp"

using namespace std;
using namespace rclcpp;

class DriverTest : public HardwareDriver
{
public:
   DriverTest() : HardwareDriver("driver_test") {}
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   spin(make_shared<DriverTest>());

   shutdown();

   return 0;
}