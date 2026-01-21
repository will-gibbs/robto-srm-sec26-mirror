// C++-specific packages
#include <memory>
#include <chrono>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"

// Custom packages
#include "sec_core_classes/controller_reference.hpp"

// Namespaces
using namespace std;
using namespace rclcpp;
using namespace std::chrono_literals;

// The definition of a controller reference test class
class ControllerReferenceTest : public Node
{
   ControllerReference controller_1,
                       controller_2;

   TimerBase::SharedPtr timer;
public:
   // Constructor, create a controller reference test node
   /* ControllerReferenceIDK() : Node("controller_reference_test")
   {
      RCLCPP_INFO(get_logger(), "Creating a controller reference test node...");

      controller_1("controller_1", shared_from_this());
      controller_2("controller_2", shared_from_this());
   }*/
   ControllerReferenceTest() : Node("controller_reference_idk"),
      controller_1("controller_1"), controller_2("controller_2")
   {
      RCLCPP_INFO(get_logger(), "Creating a controller reference IDK node...");

      timer = create_wall_timer(1s, [this](){});
   }
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   spin(make_shared<ControllerReferenceTest>());

   shutdown();

   return 0;
}