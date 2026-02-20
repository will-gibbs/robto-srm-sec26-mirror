// C++-specific packages
#include <functional>
#include <memory>
#include <chrono>
// #include <string>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

// Custom packages
#include "sec_interfaces/srv/register_controller.hpp"
#include "sec_interfaces/srv/update_task.hpp"
#include "sec_interfaces/action/complete_task.hpp"
#include "sec_core_classes/controller.hpp"

// Namespaces
using namespace std;
using namespace std::chrono_literals;
using namespace std::placeholders;
using namespace rclcpp;
using namespace sec_interfaces::srv;

//******************************************************************************
//*                              Class Definition                              *
//******************************************************************************
// The definition of a test controller
class ControllerTest : public Controller
{
public:
   ControllerTest() : Controller("duck_collection", 500ms)
   {
      register_controller = create_client<sec_interfaces::srv::RegisterController>("register_controller");

      while (!register_controller->wait_for_service(1s))
      {
         if (!rclcpp::ok())
         {
            RCLCPP_ERROR(get_logger(), "Interrupted while waiting for register controller service. Exiting.");
            exit(1);
         }
         RCLCPP_INFO(get_logger(), "Waiting for register_controller...");
      }

      auto request = std::make_shared<sec_interfaces::srv::RegisterController::Request>();

      request->controller_action_name = controller_name;

      auto future = register_controller->async_send_request(request);

      RCLCPP_INFO(get_logger(), "%s controller successfully registered.", controller_name.c_str());
   }
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   spin(make_shared<ControllerTest>());

   shutdown();

   return 0;
}
