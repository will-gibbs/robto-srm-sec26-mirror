import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist

from gpiozero import LED
from gpiozero import PWMOutputDevice
from time import sleep

# from rpi_hardware_pwm import HardwarePWM

class DrivetrainAction():
   def __init__(self):
      self.left_motor = 0.0
      self.right_motor = 0.0

   def clamp_motor_value(self, motor, min_value, max_value):
      if (motor == "left"):
         return max(min_value, min(self.left_motor, max_value))
      elif (motor == "right"):
         return max(min_value, min(self.right_motor, max_value))

class WheelDriver(Node):
   def __init__(self):
      super().__init__("wheel_driver")
      self.vel_subscriber = self.create_subscription(
         Twist,"cmd_vel", self.twist_callback, 10)
      self.left_forward = LED(27)
      self.left_backward = LED(17)
      self.left_pwm = PWMOutputDevice(14, frequency=50)
      self.right_forward = LED(1)
      self.right_backward = LED(0)
      self.right_pwm = PWMOutputDevice(15)

   def twist_callback(self, vel):
      dta = DrivetrainAction
      self.get_logger().info(f"Linear: {vel.linear} | Angular: {vel.angular}\r\n")
      dta = self.calc_drivetrain_action(vel.linear.x, vel.angular.z)
      self.get_logger().info(f"Left motor: {dta.left_motor} | Right motor: {dta.right_motor}")

      if (dta.left_motor > 0):
         self.left_forward.on()
         self.left_backward.off()
      elif (dta.left_motor < 0):
         self.left_forward.off()
         self.left_backward.on()
      else:
         self.left_forward.off()
         self.left_backward.off()
      self.left_pwm.value = abs(dta.left_motor)

      if (dta.right_motor > 0):
         self.right_forward.on()
         self.right_backward.off()
      elif (dta.right_motor < 0):
         self.right_forward.off()
         self.right_backward.on()
      else:
         self.right_forward.off()
         self.right_backward.off()
      self.right_pwm.value = abs(dta.right_motor)

   def calc_drivetrain_action(self, linear, angular) -> DrivetrainAction:
      dta = DrivetrainAction()

      dta.left_motor = +linear + angular
      dta.right_motor = +linear - angular

      excession = (dta.clamp_motor_value("left", -1.0, 1.0) - dta.left_motor) + \
                  (dta.clamp_motor_value("right", -1.0, 1.0) - dta.right_motor)
      dta.left_motor += excession
      dta.right_motor += excession

      return dta

def main(args=None):
   rclpy.init(args=args)
   
   wheel_driver = WheelDriver()
   
   rclpy.spin(wheel_driver)

   wheel_driver.destroy_node()
   rclpy.shutdown()


if __name__ == '__main__':
   main()
