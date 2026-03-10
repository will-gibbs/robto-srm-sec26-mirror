#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, TransformStamped
from nav_msgs.msg import Odometry
from tf2_ros import TransformBroadcaster
import math

# A ROS 2 node that simulates robot odometry by integrating velocity commands.
# It translates '/cmd_vel' inputs into a position in the 'odom' coordinate frame.
class DummyOdom(Node):
    def __init__(self):
        super().__init__('dummy_odom')
        
        # Subscribe to velocity commands (what the robot is TOLD to do)
        self.subscription   = self.create_subscription(Twist, '/cmd_vel', self.cmd_callback, 10)
        
        # Publisher for the Odometry message (where the robot THINKS it is)
        self.odom_pub       = self.create_publisher(Odometry, '/odom', 10)
        
        # Broadcaster to link the 'odom' frame to the 'base_link' frame in TF2
        self.tf_broadcaster = TransformBroadcaster(self)

        # Internal State: [x, y] coordinates and [th] (theta/yaw) heading in radians
        self.x              = 0.0
        self.y              = 0.0
        self.th             = 0.0
        
        # Linear and Angular velocities derived from cmd_vel
        self.vx             = 0.0
        self.vth            = 0.0
        
        # Track time for delta calculations (dt)
        self.last_time      = self.get_clock().now()
        
        # Run the 'update' loop at 20Hz (every 0.05 seconds)
        self.timer          = self.create_timer(0.05, self.update)

    # Stores the incoming velocity commands to be used in the next integration step
    def cmd_callback(self, msg):
        self.vx  = msg.linear.x
        self.vth = msg.angular.z

    # Calculates the new position and broadcasts it to the ROS system
    def update(self):
        current_time = self.get_clock().now()
        
        # Calculate time elapsed since last update in seconds
        dt           = (current_time - self.last_time).nanoseconds / 1e9
        self.last_time = current_time

        # Calculate displacement based on current heading (theta)
        delta_x  = self.vx * math.cos(self.th) * dt
        delta_y  = self.vx * math.sin(self.th) * dt
        delta_th = self.vth * dt

        # Update global pose state
        self.x   += delta_x
        self.y   += delta_y
        self.th  += delta_th

        # Convert Euler yaw (theta) to a Quaternion (required by ROS for 3D rotations)
        q_z      = math.sin(self.th / 2.0)
        q_w      = math.cos(self.th / 2.0)

        # 1. BROADCAST TRANSFORM: Defines where base_link is relative to the odom frame
        t = TransformStamped()
        t.header.stamp            = current_time.to_msg()
        t.header.frame_id         = 'odom'
        t.child_frame_id          = 'base_link'
        
        t.transform.translation.x = self.x
        t.transform.translation.y = self.y
        t.transform.rotation.z    = q_z
        t.transform.rotation.w    = q_w
        self.tf_broadcaster.sendTransform(t)

        # 2. PUBLISH ODOMETRY MESSAGE: Provides pose and velocity data for navigation
        odom = Odometry()
        odom.header               = t.header
        odom.child_frame_id       = 'base_link'
        
        # Set position and orientation
        odom.pose.pose.position.x  = self.x
        odom.pose.pose.position.y  = self.y
        odom.pose.pose.orientation.z = q_z
        odom.pose.pose.orientation.w = q_w
        
        # Set velocity (twist)
        odom.twist.twist.linear.x  = self.vx
        odom.twist.twist.angular.z = self.vth
        
        self.odom_pub.publish(odom)

def main(args=None):
    rclpy.init(args=args)
    node = DummyOdom()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()