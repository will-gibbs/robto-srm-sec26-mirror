// C++-specific packages
#include <memory>
#include <chrono>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "sec_interfaces/srv/register_controller.hpp"
#include "sec_interfaces/srv/update_task.hpp"

// Namespaces
using namespace std;
using namespace std::chrono_literals;
using namespace rclcpp;
using namespace sec_interfaces::srv;

// Definition of a test client node
class TestClient : public Node
{
   Client<RegisterController>::SharedPtr register_controller;

   Client<UpdateTask>::SharedPtr         update_task;

   TimerBase::SharedPtr                  timer;
public:
   TestClient() : Node("test_client")
   {
      auto register_controller_callback = [this](Client<RegisterController>::SharedFuture future_response) -> void
      {
         RCLCPP_INFO(get_logger(), "Received a response with a status code of %d.", future_response.get()->registration_status_code);
      };

      auto update_task_callback = [this](Client<UpdateTask>::SharedFuture future_response) -> void
      {
         RCLCPP_INFO(get_logger(), "Received a response with a status code of %d.", future_response.get()->request_status);
      };

      auto timer_callback = [this, register_controller_callback, update_task_callback]() -> void
      {
         // Create service requests
         auto register_controller_request = std::make_shared<RegisterController::Request>();
         auto update_task_request         = std::make_shared<UpdateTask::Request>();

         // Set register controller service request values
         register_controller_request->controller_action_name = "duck_collection";

         // Set update tsk service request values
         update_task_request->controller_action_name = "duck_collection";
         update_task_request->new_task_point_value = 2.5f;
         update_task_request->new_task_time_to_complete = 9.0f;
         update_task_request->new_task_likelihood_of_success = 0.95f;

         RCLCPP_INFO(get_logger(), "Sending register controller request...");
         while(!register_controller->wait_for_service(500ms))
         {
            if(ok())
            {
               RCLCPP_INFO(get_logger(), "Waiting for service server...");
            }
            else
            {
               RCLCPP_ERROR(get_logger(), "Interrupted while waiting for service server. Exiting.");
               exit(1);
            }
         }

         auto register_controller_response = register_controller->async_send_request(register_controller_request, register_controller_callback);

         RCLCPP_INFO(get_logger(), "Sending update task request...");
         while(!update_task->wait_for_service(500ms))
         {
            if(ok())
            {
               RCLCPP_INFO(get_logger(), "Waiting for service server...");
            }
            else
            {
               RCLCPP_ERROR(get_logger(), "Interrupted while waiting for service server. Exiting.");
               exit(1);
            }
         }

         auto update_task_response = update_task->async_send_request(update_task_request, update_task_callback);
      };

      register_controller = create_client<RegisterController>("register_controller");
      update_task         = create_client<UpdateTask>("update_task");

      timer = create_wall_timer(2s, timer_callback);
   }
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   spin(make_shared<TestClient>());

   shutdown();

   return 0;
}