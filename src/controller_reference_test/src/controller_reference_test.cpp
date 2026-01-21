// C++-specific packages
#include <memory>
#include <chrono>
#include <string>
#include <functional>

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
   using UpdateTask   = sec_interfaces::srv::UpdateTask;
   using CompleteTask = sec_interfaces::action::CompleteTask;

   // Constructor, create a controller reference test node
   /* ControllerReferenceIDK() : Node("controller_reference_test")
   {
      RCLCPP_INFO(get_logger(), "Creating a controller reference test node...");

      controller_1("controller_1", shared_from_this());
      controller_2("controller_2", shared_from_this());
   }*/
   ControllerReferenceTest() : Node("controller_reference_test"),
      controller_1("controller_1"), controller_2("controller_2")
   {
      RCLCPP_INFO(get_logger(), "Creating a controller reference test node...");

      init_controller_reference(&controller_1);
      init_controller_reference(&controller_2);
   };

   void init_controller_reference(ControllerReference *controller)
   {
      auto service_callback = [controller](const shared_ptr<UpdateTask::Request> request, shared_ptr<UpdateTask::Response> response)
      {
         controller->controller_update_task_callback(request, response);
      };

      RCLCPP_INFO(get_logger(), "Initializing controller reference (%s)...", controller->get_controller_action_name().c_str());
      controller->set_controller_update_task(create_service<UpdateTask>(controller->get_controller_action_name().c_str() + (string)"_update_task", service_callback));
      RCLCPP_INFO(get_logger(), "Done.");
   };

   /*void set_controller_manager_node()
   {
      controller_1.set_manager_node(shared_from_this());
      controller_1.set_manager_node(shared_from_this());
   };*/
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   /*shared_ptr<ControllerReferenceTest> node = make_shared<ControllerReferenceTest>();
   node->set_controller_manager_node();
   spin(node);*/

   spin(make_shared<ControllerReferenceTest>());

   shutdown();

   return 0;
}