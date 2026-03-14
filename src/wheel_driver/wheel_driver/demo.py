import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sec_interfaces.msg import StartSwitch
from geometry_msgs.msg import Polygon, Point32

from gpiozero import Servo

from time import sleep

class Demo(Node):
    def __init__(self):
        super().__init__("demo")
        self.vel_publisher = self.create_publisher(Twist, "cmd_vel", 10)
        self.start_switch_subscriber = self.create_subscription(StartSwitch, "start_switch", self.start_switch_callback, 10)
        #self.ducktection_subscriber = self.create_subscription(Polygon, "ducktection", self.ducktection_callback, 10)
        # GPIO pin handles
        #self.duck_brush = Servo(12)
        #self.duck_airbag = Servo(13)

    def start_switch_callback(self, message):
        if (message.start_switch_pressed == 1):
            #vel_msg = Twist()
            #vel_msg.linear.x = 0.5
            #self.vel_publisher.publish(vel_msg)
            #sleep(3.8)
            #vel_msg.linear.x = 0.0
            #self.vel_publisher.publish(vel_msg)
            self.demo_robto()

    def demo_robto(self):
        vel_msg = Twist() # cmd_vel Twist message
    
        # Reset velocity to 0.0 and sleep before starting the demo
        vel_msg.linear.x = 0.0
        vel_msg.angular.z = 0.0
        sleep(1)

        for i in range(1, 5):
            # Move forward
            vel_msg.linear.x = 0.8
            self.vel_publisher.publish(vel_msg)
            sleep(5)
            vel_msg.linear.x = 0.0
            self.vel_publisher.publish(vel_msg)
            sleep(0.5)

            # Turn right
            vel_msg.angular.z = -0.5
            self.vel_publisher.publish(vel_msg)
            sleep(2.3)
            vel_msg.angular.z = 0.0
            self.vel_publisher.publish(vel_msg)
            sleep(0.5)

        
def main(args=None):
    rclpy.init(args=args)
    demo = Demo()
    rclpy.spin(demo)
    #main_sequence.ram_button()
    demo.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
