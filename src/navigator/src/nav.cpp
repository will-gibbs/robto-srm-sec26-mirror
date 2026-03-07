#include <memory>
#include <chrono>
#include <limits>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

using NavigateToPose = nav2_msgs::action::NavigateToPose;
using GoalHandleNav = rclcpp_action::ClientGoalHandle<NavigateToPose>;

class CameraNavigator : public rclcpp::Node {
public:
    CameraNavigator() : Node("camera_navigator") {
        // 1. PointCloud Subscription (OAK-D default topic)
        pc_sub_ = this->create_subscription<sensor_msgs::msg::PointCloud2>(
            "/oak/points", 10, std::bind(&CameraNavigator::pc_callback, this, std::placeholders::_1));
        
        // 2. LaserScan Publisher (Standard SLAM input)
        scan_pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>("/scan", 10);

        // 3. Nav2 Action Client
        this->client_ptr_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");

        RCLCPP_INFO(this->get_logger(), "Navigation Node Started. Waiting for data...");
    }

    void goToPosition(double x, double y) {
        if (!this->client_ptr_->wait_for_action_server(std::chrono::seconds(5))) {
            RCLCPP_ERROR(this->get_logger(), "Nav2 Action server not available!");
            return;
        }

        auto goal_msg = NavigateToPose::Goal();
        goal_msg.pose.header.frame_id = "map";
        goal_msg.pose.header.stamp = this->now();
        goal_msg.pose.pose.position.x = x;
        goal_msg.pose.pose.position.y = y;
        goal_msg.pose.pose.orientation.w = 1.0;

        RCLCPP_INFO(this->get_logger(), "Sending goal to x: %.2f, y: %.2f", x, y);
        this->client_ptr_->async_send_goal(goal_msg);
    }

private:
    void pc_callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
        pcl::fromROSMsg(*msg, *cloud);

        auto scan = std::make_unique<sensor_msgs::msg::LaserScan>();
        scan->header = msg->header;
        // IMPORTANT: frame_id must match your robot's camera frame (usually 'oak_rgb_camera_optical_frame')
        // but for SLAM we project it to a horizontal plane.
        scan->header.frame_id = msg->header.frame_id; 

        // LaserScan config
        double angle_min = -0.78, angle_max = 0.78, angle_inc = 0.0087;
        int nr_steps = std::ceil((angle_max - angle_min) / angle_inc);

        scan->angle_min = angle_min;
        scan->angle_max = angle_max;
        scan->angle_increment = angle_inc;
        scan->range_min = 0.1;
        scan->range_max = 10.0;
        scan->ranges.assign(nr_steps, std::numeric_limits<float>::infinity());

        for (const auto& point : cloud->points) {
            if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) continue;

            // In Optical Frames: 
            // point.z is Depth (Forward)
            // point.x is Horizontal (Left/Right)
            // point.y is Vertical (Up/Down)

            // 1. Filter Height: Use point.y (Vertical) 
            // Since Y is 'Down', -0.1 to 0.1 usually captures the middle of the camera view
            if (point.y < -0.1 || point.y > 0.1) continue; 

            // 2. Calculate Range and Angle using X and Z
            float range = std::sqrt(point.x * point.x + point.z * point.z);
            float angle = std::atan2(point.x, point.z); // Note: x and z swap to make Z 'forward'

            if (angle >= angle_min && angle <= angle_max) {
                int index = (angle - angle_min) / angle_inc;
                if (index >= 0 && index < nr_steps && range < scan->ranges[index]) {
                    scan->ranges[index] = range;
                }
            }
        }
        scan_pub_->publish(std::move(scan));
    }

    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr pc_sub_;
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
    rclcpp_action::Client<NavigateToPose>::SharedPtr client_ptr_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CameraNavigator>();
    
    // Example: Navigate to arena center after 5s
    rclcpp::TimerBase::SharedPtr timer = node->create_wall_timer(
        std::chrono::seconds(5), [&]() { node->goToPosition(1.0, 0.0); timer->cancel(); });

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}