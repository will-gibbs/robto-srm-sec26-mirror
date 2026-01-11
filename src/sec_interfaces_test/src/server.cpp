// C++-specific packages
#include <memory>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "sec_interfaces/srv/register_controller.hpp"
#include "sec_interfaces/srv/update_task.hpp"

// Namespaces
using namespace std;
using namespace rclcpp;
using namespace sec_interfaces::srv;

// Definition of a test server node
class Server : public Node
{
   // Service servers
   Service<RegisterController>::SharedPtr register_controller;
   Service<UpdateTask>::SharedPtr         update_task;
public:
   // Constructor, create a server node
   Server() : Node("test_server")
   {
      // Callback for register controller service server
      auto register_controller_callback = [this](const shared_ptr<RegisterController::Request> request, shared_ptr<RegisterController::Response> response) -> void
      {
         // Modify the response to have a status code of OK
         // note: this response will be sent to the client when the callback function returns
         response->registration_status_code = response->OK;

         // Log the actions of the service
         RCLCPP_INFO(get_logger(), "Registration request received with a controller action name of '%s'.", (request->controller_action_name).c_str());
         RCLCPP_INFO(get_logger(), "Replying with status code %d (OK)...", response->OK);
      };

      // Callback for update task service server
      auto update_task_callback = [this](const shared_ptr<UpdateTask::Request> request, shared_ptr<UpdateTask::Response> response) -> void
      {
         // Modify the response to have a status code of OK
         // note: this response will be sent to the client when the callback function returns
         response->request_status = response->OK;

         // Log the actions of the service
         RCLCPP_INFO(get_logger(), "Update task request received with values: controller_action_name='%s', new_task_point_value=%f, new_task_time_to_complete=%f, new_task_likelihood_of_success=%f.",
                                    (request->controller_action_name).c_str(),
                                    request->new_task_point_value,
                                    request->new_task_time_to_complete,
                                    request->new_task_likelihood_of_success);
         RCLCPP_INFO(get_logger(), "Replying with status code %d (OK)...", response->OK);
      };

      // Create the services with the related interface types, service names, and callback functions
      // create_service(const std::string &service_name, CallbackT &&callback)
      register_controller = create_service<RegisterController>("register_controller", register_controller_callback);
      update_task = create_service<UpdateTask>("update_task", update_task_callback);

      // Log the actions of the node
      RCLCPP_INFO(get_logger(), "Creating a test server...");
      RCLCPP_INFO(get_logger(), "Done.");
      RCLCPP_INFO(get_logger(), "Waiting for a request...");
   }
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   // Execute the node
   spin(make_shared<Server>());

   // Clean up
   shutdown();

   return 0;
}