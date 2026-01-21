// C++-specific packages
#include <memory>
#include <chrono>
#include <string>
#include <functional>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

// Custom packages
#include "sec_core_classes/controller_reference.hpp"

// Namespaces
using namespace std;
using namespace rclcpp;
using namespace std::chrono_literals;

// The definition of a controller reference test class
class ControllerReferenceTest : public Node
{
   // Test ControllerReference objects
   ControllerReference controller_1,
                       controller_2;

public:
   // Interface type aliases
   using UpdateTask   = sec_interfaces::srv::UpdateTask;
   using CompleteTask = sec_interfaces::action::CompleteTask;

   // Constructor, create a controller reference test node
   ControllerReferenceTest() : Node("controller_reference_test"),
      controller_1("controller_1"), controller_2("controller_2")
   {
      RCLCPP_INFO(get_logger(), "Creating a controller reference test node...");

      // Initialize the test ControllerReference objects
      init_controller_reference(&controller_1);
      init_controller_reference(&controller_2);
   };

   // Create the service server and action client on a controller reference object
   void init_controller_reference(ControllerReference *controller)
   {
      // Callback for the update task service on the controller reference object
      auto service_callback = [this, controller](const shared_ptr<UpdateTask::Request> request, shared_ptr<UpdateTask::Response> response)
      {
         // Log the update task request
         RCLCPP_INFO(get_logger(), "Received request to update task values from '%s'.", controller->get_controller_name().c_str());

         // Call the callback function that is a member of the ControllerReference object
         // to update the controller's task values
         controller->controller_update_task_callback(request, response);

         RCLCPP_INFO(get_logger(), "Request handled. request_status = %d", response->request_status);

         if (response->request_status == response->OK)
         {
            // Calculate and log the new task priority
            float calculated_priority = (request->new_task_point_value / request->new_task_time_to_complete) * request->new_task_likelihood_of_success;
            RCLCPP_INFO(get_logger(), "New calculated priority = %f", calculated_priority);
         }
         else if (response->request_status == response->RETRY)
         {
            RCLCPP_ERROR(get_logger(), "Error updating task values. Retry update.");
         }
      };

      RCLCPP_INFO(get_logger(), "Initializing controller reference (%s)...", controller->get_controller_name().c_str());

      // Crete the service server and action client
      //
      // Note: a ControllerReference object cannot do these because it is not a Node object.
      // Thus, the service server and action client are members of the ControllerReference object,
      // but are created and handled by a Node object
      // (in this case, a ControllerReferenceTest object; in actual implementation, the Manager).
      controller->set_controller_update_task  (create_service<UpdateTask>(controller->get_controller_name().c_str() + (string)"/update_task", service_callback));
      controller->set_controller_complete_task(rclcpp_action::create_client<CompleteTask>(this, controller->get_controller_name() + "/complete_task"));

      RCLCPP_INFO(get_logger(), "Done.");
   };
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   // Execute the node
   spin(make_shared<ControllerReferenceTest>());

   // Clean up
   shutdown();

   return 0;
}