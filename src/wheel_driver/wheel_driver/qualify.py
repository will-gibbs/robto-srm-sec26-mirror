import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sec_interfaces.msg import StartSwitch

from time import sleep

class QualifyNode(Node):
    def __init__(self):
        super().__init__("qualify_node")
        self.vel_publisher = self.create_publisher(Twist, "cmd_vel", 10)
        self.start_switch_subscriber = self.create_subscription(StartSwitch, "start_switch", self.start_switch_callback, 10)

    def start_switch_callback(self, message):
        if (message.start_switch_pressed == 1):
            vel_msg = Twist()
            vel_msg.linear.x = 0.5
            self.vel_publisher.publish(vel_msg)
            sleep(3.8)
            vel_msg.linear.x = 0.0
            self.vel_publisher.publish(vel_msg)
        
def main(args=None):
    rclpy.init(args=args)
    qualify_node = QualifyNode()
    rclpy.spin(qualify_node)
    qualify_node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
