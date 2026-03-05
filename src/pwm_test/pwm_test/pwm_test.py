import rclpy
from rclpy.node import Node
from std_msgs.msg import Int8

# from rpi_hardware_pwm import HardwarePWM

class PWMTest(Node):
   def __init__(self):
      super().__init__("pwm_test")
      self.counter = 0
      self.publisher = self.create_publisher(Int8, "test_topic", 10)
      self.timer = self.create_timer(1, self.timer_callback)

   def timer_callback(self):
      msg = Int8()
      msg.data = self.counter
      self.publisher.publish(msg)
      self.get_logger().info(f"Publishing: {self.counter}...")
      self.counter += 1


def main(args=None):
   rclpy.init(args=args)
   
   pwm_test_node = PWMTest()
   
   rclpy.spin(pwm_test_node)

   pwm_test_node.destroy_node()
   rclpy.shutdown()


if __name__ == '__main__':
   main()