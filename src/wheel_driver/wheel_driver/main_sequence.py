import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sec_interfaces.msg import StartSwitch
from geometry_msgs.msg import Polygon, Point32

from gpiozero import Servo

from time import sleep

class MainSequence(Node):
    def __init__(self):
        super().__init__("main_sequence")
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
            self.ram_button()

    #def ducktection_callback(self, message):
    #    for duck in message:
    #        if (duck.z <= 36):
    #            while (duck.x < 540 or duck.x > 740):
    #                if (duck.x < 640):
    #                    # Left
    #                    pass
    #                else:
    #                    # Right
    #                    pass

    def ram_button(self):
        vel_msg = Twist()
        vel_msg.linear.x = 0.0
        self.vel_publisher.publish(vel_msg)
        sleep(1)
        # Ram 1
        vel_msg.linear.x = 0.5
        self.vel_publisher.publish(vel_msg)
        sleep(3.8)
        vel_msg.linear.x = 0.0
        self.vel_publisher.publish(vel_msg)
        sleep(0.5)
        vel_msg.linear.x = -0.5
        self.vel_publisher.publish(vel_msg)
        sleep(1)
        # Ram 2
        vel_msg.linear.x = 0.5
        self.vel_publisher.publish(vel_msg)
        sleep(1.5)
        vel_msg.linear.x = 0.0
        self.vel_publisher.publish(vel_msg)
        sleep(0.5)
        vel_msg.linear.x = -0.5
        self.vel_publisher.publish(vel_msg)
        sleep(1)
        # Ram 3
        vel_msg.linear.x = 0.5
        self.vel_publisher.publish(vel_msg)
        sleep(1.5)
        vel_msg.linear.x = 0.0
        self.vel_publisher.publish(vel_msg)
        sleep(0.5)
        # Reverse
        vel_msg.linear.x = -0.5
        self.vel_publisher.publish(vel_msg)
        sleep(1.5)
        vel_msg.linear.x = 0.0
        self.vel_publisher.publish(vel_msg)
        sleep(1)
        # Turn and push duck
        vel_msg.angular.z = -0.5
        vel_msg.linear.x = 0.5
        self.vel_publisher.publish(vel_msg)
        sleep(2.5)
        vel_msg.linear.x = 0.5
        vel_msg.angular.z = 0.0
        self.vel_publisher.publish(vel_msg)
        sleep(2)
        vel_msg.linear.x = 0.0
        self.vel_publisher.publish(vel_msg)
        sleep(1)
        # Reverse and rotate to detect second duck
        vel_msg.linear.x = -0.5
        vel_msg.angular.z = 0.5
        self.vel_publisher.publish(vel_msg)
        sleep(1.5)
        vel_msg.linear.x = -0.5
        vel_msg.angular.z = 0.0
        self.vel_publisher.publish(vel_msg)
        sleep(4)
        vel_msg.linear.x = 0.0
        vel_msg.angular.z = 0.0
        sleep(1)

def main(args=None):
    rclpy.init(args=args)
    main_sequence = MainSequence()
    rclpy.spin(main_sequence)
    #main_sequence.ram_button()
    main_sequence.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
