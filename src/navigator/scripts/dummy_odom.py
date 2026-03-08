#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, TransformStamped
from nav_msgs.msg import Odometry
from tf2_ros import TransformBroadcaster
import math

class DummyOdom(Node):
    def __init__(self):
        super().__init__('dummy_odom')
        self.subscription = self.create_subscription(Twist, '/cmd_vel', self.cmd_callback, 10)
        self.odom_pub = self.create_publisher(Odometry, '/odom', 10)
        self.tf_broadcaster = TransformBroadcaster(self)

        # Robot State
        self.x = 0.0
        self.y = 0.0
        self.th = 0.0
        self.vx = 0.0
        self.vth = 0.0
        
        self.last_time = self.get_clock().now()
        self.timer = self.create_timer(0.1, self.update) # 10Hz update

    def cmd_callback(self, msg):
        self.vx = msg.linear.x
        self.vth = msg.angular.z

    def update(self):
        current_time = self.get_clock().now()
        dt = (current_time - self.last_time).nanoseconds / 1e9
        self.last_time = current_time

        # Update position based on velocity
        delta_x = self.vx * math.cos(self.th) * dt
        delta_y = self.vx * math.sin(self.th) * dt
        delta_th = self.vth * dt

        self.x += delta_x
        self.y += delta_y
        self.th += delta_th

        # Create Quaternion from Yaw (Z-axis rotation)
        q_z = math.sin(self.th / 2.0)
        q_w = math.cos(self.th / 2.0)

        # 1. Publish Transform (odom -> base_link)
        t = TransformStamped()
        t.header.stamp = current_time.to_msg()
        t.header.frame_id = 'odom'
        t.child_frame_id = 'base_link'
        t.transform.translation.x = self.x
        t.transform.translation.y = self.y
        t.transform.rotation.z = q_z
        t.transform.rotation.w = q_w
        self.tf_broadcaster.sendTransform(t)

        # 2. Publish Odometry Message
        odom = Odometry()
        odom.header = t.header
        odom.child_frame_id = 'base_link'
        odom.pose.pose.position.x = self.x
        odom.pose.pose.position.y = self.y
        odom.pose.pose.orientation.z = q_z
        odom.pose.pose.orientation.w = q_w
        self.odom_pub.publish(odom)

def main(args=None):
    rclpy.init(args=args)
    node = DummyOdom()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
    main()