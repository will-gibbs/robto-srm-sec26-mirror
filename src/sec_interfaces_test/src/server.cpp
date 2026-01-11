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
   Service<RegisterController>::SharedPtr register_controller;

   Service<UpdateTask>::SharedPtr         update_task;

   // TimerBase::SharedPtr timer;
public:
   Server() : Node("test_server")
   {
      auto register_controller_callback = [this](const shared_ptr<RegisterController::Request> request, shared_ptr<RegisterController::Response> response) -> void
      {
         string action_name = request->controller_action_name;

         response->registration_status_code = response->OK;

         RCLCPP_INFO(get_logger(), "Registration request received with a controller action name of '%s'.", action_name.c_str());
         RCLCPP_INFO(get_logger(), "Replying with status code %d (OK)...", response->OK);
      };

      auto update_task_callback = [this](const shared_ptr<UpdateTask::Request> request, shared_ptr<UpdateTask::Response> response) -> void
      {
         string action_name = request->controller_action_name;

         response->request_status = response->OK;

         RCLCPP_INFO(get_logger(), "Update task request received with values: controller_action_name='%s', new_task_point_value=%f, new_task_time_to_complete=%f, new_task_likelihood_of_success=%f.",
                                    action_name.c_str(),
                                    request->new_task_point_value,
                                    request->new_task_time_to_complete,
                                    request->new_task_likelihood_of_success);
         RCLCPP_INFO(get_logger(), "Replying with status code %d (OK)...", response->OK);
      };

      register_controller = create_service<RegisterController>("register_controller", register_controller_callback);
      update_task = create_service<UpdateTask>("update_task", update_task_callback);

      RCLCPP_INFO(get_logger(), "Creating a test server...");
      RCLCPP_INFO(get_logger(), "Done.");
      RCLCPP_INFO(get_logger(), "Waiting for a request...");
   }

   /*void register_controller_callback(const shared_ptr<RegisterController::Request> request, shared_ptr<RegisterController::Response> response)
   {
      response->registration_status_code = response->OK;

      RCLCPP_INFO(get_logger(), "Registration request recieved...");
      RCLCPP_INFO(get_logger(), "Replying with status code %d (OK)...", response->OK);
   }*/
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   spin(make_shared<Server>());

   shutdown();

   return 0;
}