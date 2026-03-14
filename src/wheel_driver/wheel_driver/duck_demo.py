import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sec_interfaces.msg import StartSwitch
from geometry_msgs.msg import Polygon, Point32

from gpiozero import Servo

from time import sleep

class DuckDemo(Node):
    def __init__(self):
        super().__init__("duck_demo")
        self.vel_publisher = self.create_publisher(Twist, "cmd_vel", 10)
        self.start_switch_subscriber = self.create_subscription(StartSwitch, "start_switch", self.start_switch_callback, 10)
        self.ducktection_subscriber = self.create_subscription(Polygon, "ducktection", self.ducktection_callback, 10)
        # GPIO pin handles
        #self.duck_brush = Servo(12)
        #self.duck_airbag = Servo(13)

        self.demo_started = False
        self.duck_detected = False

    def start_switch_callback(self, message):
        if (message.start_switch_pressed == 1):
            #vel_msg = Twist()
            #vel_msg.linear.x = 0.5
            #self.vel_publisher.publish(vel_msg)
            #sleep(3.8)
            #vel_msg.linear.x = 0.0
            #self.vel_publisher.publish(vel_msg)
            #self.duck_demo_robto()
            self.demo_started = True

    def ducktection_callback(self, message):
        vel_msg = Twist() # cmd_vel Twist message

        if (self.duck_detected == True and message.points[0].x == message.points[0].y == message.points[0].z == -1.0):
            self.duck_detected = False
            vel_msg.linear.x = 0.0
            self.vel_publisher.publish(vel_msg)
            sleep(1)
            vel_msg.angular.z = 0.5
            self.vel_publisher.publish(vel_msg)
        elif (self.duck_detected == False and (message.points[0].x != -1.0 or message.points[0].y != -1.0 or message.points[0].z != -1.0)):
            self.duck_detected = True
            vel_msg.angular.z = 0.0
            self.vel_publisher.publish(vel_msg)
            sleep(1)
            vel_msg.linear.x = -0.5
            self.vel_publisher.publish(vel_msg)
            if (message.points[0].z < 25.0):
                sleep(3)
    

    def duck_demo_robto(self):
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
    duck_demo = DuckDemo()
    rclpy.spin(duck_demo)
    #main_sequence.ram_button()
    duck_demo.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
