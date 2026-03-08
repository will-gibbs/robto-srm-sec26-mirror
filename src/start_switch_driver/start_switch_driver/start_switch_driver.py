from gpiozero import DigitalInputDevice
from time import sleep

import rclpy
from rclpy.node import Node
from sec_interfaces.msg import StartSwitch

# from rpi_hardware_pwm import HardwarePWM

class StartSwitchDriver(Node):
   def __init__(self):
      super().__init__("start_switch")
      self.start_switch = DigitalInputDevice(10)
      self.start_switch_publisher = self.create_publisher(StartSwitch, "start_switch", 10)
      self.timer = self.create_timer(0.1, self.timer_callback)

   def timer_callback(self):
      msg = StartSwitch()
      if (self.start_switch.is_active):
         self.get_logger().info(f"Start switch pressed")
         msg.start_switch_pressed = True
      else:
         msg.start_switch_pressed = False
      self.start_switch_publisher.publish(msg)


def main(args=None):
   rclpy.init(args=args)
   
   start_switch_node = StartSwitchDriver()
   
   rclpy.spin(start_switch_node)

   start_switch_node.destroy_node()
   rclpy.shutdown()

if __name__ == '__main__':
   main()
