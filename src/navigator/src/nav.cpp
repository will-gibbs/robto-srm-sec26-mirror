#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using NavigateToPose = nav2_msgs::action::NavigateToPose;

class CameraNavigator : public rclcpp::Node
{
public:
    CameraNavigator() : Node("camera_navigator")
    {
        this->client_ptr_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");
    }

    void goToPosition(double x, double y)
    {
        auto goal_msg = NavigateToPose::Goal();
        goal_msg.pose.header.frame_id    = "map";
        goal_msg.pose.header.stamp       = this->now();
        goal_msg.pose.pose.position.x    = x;
        goal_msg.pose.pose.position.y    = y;
        goal_msg.pose.pose.orientation.w = 1.0;

        // The OAK-D is now working in the background, 
        // updating the costmap so Nav2 avoids obstacles.
        this->client_ptr_->async_send_goal(goal_msg);
    }

private:
    rclcpp_action::Client<NavigateToPose>::SharedPtr client_ptr_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CameraNavigator>();
    
    // Example: Tell the robot to go to x=2.0, y=1.0
    node->goToPosition(2.0, 1.0);

    rclcpp::spin    (node);
    rclcpp::shutdown();
    return 0;
}