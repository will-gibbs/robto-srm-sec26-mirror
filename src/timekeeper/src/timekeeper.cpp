// C++-specific packages
#include <chrono>
#include <memory>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "builtin_interfaces/msg/duration.hpp"
#include "sec_interfaces/srv/start_round.hpp"
#include "sec_interfaces/msg/start_switch.hpp"

// Namespaces
using namespace std;
using namespace std::chrono_literals;
using namespace rclcpp;

// The definition of a timekeeper node
class Timekeeper : public Node
{
   // Wall timer
   TimerBase::SharedPtr                                    timer;

   // Round start client
   Client<sec_interfaces::srv::StartRound>::SharedPtr      start_round;
   
   // Round time publisher
   Publisher<builtin_interfaces::msg::Duration>::SharedPtr publisher;

   // Start switch subscriber
   Subscription<sec_interfaces::msg::StartSwitch>::SharedPtr start_switch_subscriber;

   // Time remaining in the round in seconds and nanoseconds
   int32_t                                                 round_time_sec;
   uint32_t                                                round_time_nanosec;

   // Flag to indicate if the round has started
   bool round_has_started = 0;
public:
   // Constructor, create a timekeeper
   Timekeeper() : Node("timekeeper"), round_time_sec(180), round_time_nanosec(0)
   {
      // NOTE: right now timekeeper is configured to start the round when it comes into existence. This may or may not need to be changed in the future.
      // Create the round start client
      start_round = create_client<sec_interfaces::srv::StartRound>("start_round");

      // Wait for start_round to be available
      while (!start_round->wait_for_service(1s))
      {
         if (!rclcpp::ok())
         {
            RCLCPP_ERROR(get_logger(), "Interrupted while waiting for start_round.");
            exit(1);
         }
         RCLCPP_INFO(get_logger(), "Waiting for start_round...");
      }

      // Create the request to start the round
      // auto request = std::make_shared<sec_interfaces::srv::StartRound::Request>();

      // Populate request field
      // request->start_round = request->START_ROUND;

      // Call the round start service
      // auto future = start_round->async_send_request(request);

      // Confirm the call was sent
      // RCLCPP_INFO(get_logger(), "start_round called.");

      // Create the round time publisher
      publisher = create_publisher<builtin_interfaces::msg::Duration>("round_time", 10);

      // Callback function to be called on every tick of the wall timer
      auto timer_callback = [this]() -> void
      {
         // Publish the time remaining until it hits zero
         if (round_time_sec >= 0)
         {
            // Create the message to be published
            auto message = builtin_interfaces::msg::Duration();

            // Write the time remaining to the message
            message.sec = round_time_sec;
            message.nanosec = round_time_nanosec;

            // Log the time remaining
            RCLCPP_INFO(get_logger(), "Time remaining: %d.%02.0f seconds.",
               message.sec, (float)message.nanosec * 0.0000001f);
            
            // Publish the message
            publisher->publish(message);

            // Subtract time elapsed from the time remaining
            if (round_time_nanosec == (uint32_t)0)
            {
               round_time_sec -= 1;
               round_time_nanosec = 990000000;
            }
            else
            {
               round_time_nanosec -= 10000000;
            }
         }
         else 
         {
            RCLCPP_INFO(get_logger(), "Times up!");
            exit(0);
         }
      };

      auto start_switch_callback = [this](sec_interfaces::msg::StartSwitch::UniquePtr message) -> void
      {
         if (message->start_switch_pressed == 1)
         {
            if (round_has_started == 0)
            {
               round_has_started = 1;

               // Create the request to start the round
               auto request = std::make_shared<sec_interfaces::srv::StartRound::Request>();

               // Populate request field
               request->start_round = request->START_ROUND;

               // Call the round start service
               auto future = start_round->async_send_request(request);

               // Confirm the call was sent
               RCLCPP_INFO(get_logger(), "start_round called.");
            }
         }
      };

      // Create the start switch subscriber
      start_switch_subscriber = create_subscription<sec_interfaces::msg::StartSwitch>
      ("start_switch", 10, start_switch_callback);

      // Create the wall timer
      timer = create_wall_timer(10ms, timer_callback);
   }
};

int main(int argc, char *argv[])
{
   init(argc, argv);

   // Execute the node
   spin(make_shared<Timekeeper>());

   // Clean up
   shutdown();

   return 0;
}