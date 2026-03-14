import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sec_interfaces.

from time import sleep

class QualifyNode(Node):
    def __init__(self):
        super().__init__("qualify_node")
        self.vel_publisher = self.create_publisher(Twist, "cmd_vel", 10)
        self.timer = create

    def timer_callback(self):

        
def main(args=None):
    rclpy.init(args=args)
    wheel_driver = WheelDriver()
    rclpy.spin(wheel_driver)
    wheel_driver.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
