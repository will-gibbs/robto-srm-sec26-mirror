#include <memory>

// ROS2-specific packages
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std;
using namespace rclcpp;
using namespace geometry_msgs;

// A drivetrain action; describes an action specifying how the left and right motors should rotate.
struct drivetrain_action {
	// Interval: [-1.0, 1.0]. Positive means forward.
	double left_motor;
	// Interval: [-1.0, 1.0]. Positive means forward.
	double right_motor;
};

class WheelDriver : public Node
{
	Subscription<msg::Twist>::SharedPtr subscr;
	
public:
	
	WheelDriver() : Node("wheel_driver")
	{
		// Twist message callback
		auto twist_callback = [this](const msg::Twist::ConstSharedPtr vel) -> void
		{
			RCLCPP_INFO(get_logger(), "Linear: %f | Angular: %f\r\n", vel->linear.x, vel->angular.z);
			drivetrain_action dta = calc_drivetrain_action(vel->linear.x, vel->angular.z);
			RCLCPP_INFO(get_logger(), "Left motor: %f | Right motor: %f\r\n", dta.left_motor, dta.right_motor);
			
			// TODO: apply calc_drivetrain_action to the drivetrain.
			// I doubt you need to do the PID loop; the node which sends Twist messages and receives Odometry data should be doing it.
		};
		
		const rclcpp::QoS qos = rclcpp::QoS(rclcpp::KeepLast(7)).reliable(); // From Turtlesim
		// create_subscriber(const std::string &topic_name, rclcpp::QoS QoS, CallbackT && callback)
		subscr = create_subscription<msg::Twist>(/*`real_name "/" + `?*/"cmd_vel", /*`10`?*/qos, twist_callback);
	}
	
private:
	
	// Calculates the driving of the motors.
	// Both values must be in the interval [-1.0, 1.0].
	// (A positive linear value means moving forwards, and a positive angular value means rotating leftwards.)
	drivetrain_action calc_drivetrain_action(double linear, double angular) {
		drivetrain_action dta; // The drivetrain action to return
		
		{ // Sanity checks
			const auto error_OOB_fmtstr = "Error: %s parameter OOB in calc_drivetrain_action: %f\r\n";
			
			if (linear <  -1.0 || linear  > 1.0)
				RCLCPP_ERROR(get_logger(), error_OOB_fmtstr, "linear",  linear);
			if (angular < -1.0 || angular > 1.0)
				RCLCPP_ERROR(get_logger(), error_OOB_fmtstr, "angular", angular);
		}
		
		{ // Create the initial drivetrain action calculation.
		  // This is merely the *initial* calculation, because it may specify
		  // one (exactly one) motor speed beyond +/- 100%.
			dta.left_motor  = + linear - angular;
			dta.right_motor = + linear + angular;
		}
		
		{ // Move both motor speeds towards zero so that any which are beyond +/- 100%
		  // become 100%.
			double excession; // The amount over (negative) or under (positive)
			
			excession =
				// The amount the left  motor value is under
				+ (clamp(dta.left_motor,  -1.0, 1.0) - dta.left_motor)
				// The amount the right motor value is under
				+ (clamp(dta.right_motor, -1.0, 1.0) - dta.right_motor);
			dta.left_motor  += excession;
			dta.right_motor += excession;
		}
		
		return dta;
	}
};

int main(int argc, char *argv[])
{
	init(argc, argv);
	
	// Start up the node
	spin(make_shared<WheelDriver>());
	
	// Clean up
	shutdown();
	
	return 0;
}
