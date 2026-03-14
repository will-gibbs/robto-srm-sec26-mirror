import rclpy
from rclpy.node import Node
from sec_interfaces.msg import ColorRGBLT

import board
import adafruit_tcs34725
from time import sleep

class ArmColorSensorDriver(Node):
   def __init__(self):
      super().__init__("arm_color_sensor")
      self.color_publisher = self.create_publisher(ColorRGBLT, "arm_color", 10)
      self.i2c = board.I2C()
      self.color_sensor = adafruit_tcs34725.TCS34725(i2c)
      self.timer = self.create_timer(0.1, self.timer_callback)

   def timer_callback(self):
      msg = ColorRGBLT()
      msg.r, msg.g, msg.b = color_sensor.get_rgb_bytes
      msg.lux = color_sensor.lux
      msg.temp = color_sensor.color_temperature
      self.color_publisher.publish(msg)
      self.get_logger().info(f"Publishing: r={msg.r} g={msg.g} b={msg.b} lux={msg.lux} temp={msg.temp}")

def main(args=None):
   rclpy.init(args=args)
   
   arm_color_sensor_driver = ArmColorSensorDriver()
   
   rclpy.spin(arm_color_sensor_driver)

   arm_color_sensor_driver.destroy_node()
   rclpy.shutdown()


if __name__ == '__main__':
   main()
