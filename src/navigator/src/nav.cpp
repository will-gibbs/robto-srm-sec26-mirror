// Work in Progress, slop detected

// C++-specific packages
#include <memory>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"

// Custom packages


using namespace std;
using namespace rclcpp;

using NavigateToPose = nav2_msgs::action::NavigateToPose;
using GoalHandleNav  = rclcpp_action::ClientGoalHandle<NavigateToPose>;

class Navigator : public Node
{
public:
    Navigator() : Node("Navigator")
    {
        this->client_ptr_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");
    }

    void send_goal(double x, double y, double theta_w)
    {
        if (!this->client_ptr_->wait_for_action_server(chrono::seconds(10)))
        {
            RCLCPP_ERROR(this->get_logger(), "Action server timed out");
            return;
        }

        auto goal_msg                 = NavigateToPose::Goal();
        goal_msg.pose.header.frame_id = "map";
        goal_msg.pose.header.stamp    = this->now();

        // Set target position
        goal_msg.pose.pose.position.x = x;
        goal_msg.pose.pose.position.y = y;
        
        // Set target orientation (quaternion)
        goal_msg.pose.pose.orientation.w = theta_w;

        RCLCPP_INFO(this->get_logger(), "Sending goal...");
        auto send_goal_options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
        
        send_goal_options.result_callback = bind(&Navigator::result_callback, this, placeholders::_1);

        this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
    }

private:
    rclcpp_action::Client<NavigateToPose>::SharedPtr client_ptr_;

    void result_callback(const GoalHandleNav::WrappedResult & result)
    {
        switch (result.code)
        {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "Location reached successfully!");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Navigation was aborted");
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_ERROR(this->get_logger(), "Navigation was canceled");
                break;
            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown navigation error");
        }
    }
};

int main(int argc, char ** argv)
{
    init(argc, argv);
    auto node = make_shared<Navigator>();
    
    // Move 2.0 meters forward in X, 1.0 meters in Y
    node->send_goal(2.0, 1.0, 1.0); 
    
    spin(node);
    shutdown();
    return 0;
}