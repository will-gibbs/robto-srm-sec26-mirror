import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist

from gpiozero import DigitalOutputDevice, PWMOutputDevice
from time import sleep

# from rpi_hardware_pwm import HardwarePWM

RAMP_STEP = 5 / 255.0  # ~0.020 per tick
RAMP_INTERVAL = 0.020  # seconds  (20 ms)
BRAKE_PWM = 40 / 255.0  # ~0.157  hold current during brake ramp


class MotorDriver:
    def __init__(self, pin_pwm: int, pin_dir: int):
        self.pwm = PWMOutputDevice(pin_pwm, frequency=1000)
        self.dir_pin = DigitalOutputDevice(pin_dir)
        self.target_speed = 0.0  # 0.0 – 1.0
        self.current_speed = 0.0
        self.mode = "s"  # 'f' | 'r' | 's' | 'b'

    def apply(self):
        if self.mode == "f":  # forward drive
            self.dir_pin.on()
            self.pwm.value = self.current_speed
        elif self.mode == "r":  # reverse drive
            self.dir_pin.off()
            self.pwm.value = self.current_speed
        elif self.mode == "b":  # immediate brake (nice)
            self.pwm.value = (
                max(self.current_speed, BRAKE_PWM) if self.current_speed > 0 else 0.0
            )
        else:  # 's' — coast stop
            self.pwm.value = 0.0

    def set(self, mode: str, speed: float):
        speed = max(0.0, min(1.0, abs(speed)))
        if mode == "s":
            self.target_speed = 0.0
            self.current_speed = 0.0
            self.mode = "s"
        elif mode == "b":
            self.target_speed = 0.0
            self.mode = "b"
        else:
            self.target_speed = speed
            self.mode = mode
        self.apply()

    def ramp(self):
        if self.mode == "s":
            return
        if self.mode == "b":
            if self.current_speed > 0:
                self.current_speed = max(self.current_speed - RAMP_STEP, 0.0)
            self.apply()
            return
        # normal forward / reverse ramp toward target
        if self.current_speed < self.target_speed:
            self.current_speed = min(self.current_speed + RAMP_STEP, self.target_speed)
            self.apply()
        elif self.current_speed > self.target_speed:
            self.current_speed = max(self.current_speed - RAMP_STEP, self.target_speed)
            self.apply()

    def cleanup(self):
        self.pwm.value = 0.0
        self.dir_pin.off()


class DrivetrainAction:
    def __init__(self):
        self.left_motor = 0.0
        self.right_motor = 0.0

    def clamp_motor_value(self, motor, min_value, max_value):
        if motor == "left":
            return max(min_value, min(self.left_motor, max_value))
        elif motor == "right":
            return max(min_value, min(self.right_motor, max_value))
        return None


class WheelDriver(Node):
    def __init__(self):
        super().__init__("wheel_driver")
        self.vel_subscriber = self.create_subscription(
            Twist, "cmd_vel", self.twist_callback, 10
        )

        self.left = MotorDriver(pin_pwm=27, pin_dir=15)  # Left motor
        self.right = MotorDriver(pin_pwm=14, pin_dir=17)  # right motor

        self.ramp_timer = self.create_timer(
            RAMP_INTERVAL, self._ramp_cb
        )  # internal state timer

    def twist_callback(self, vel: Twist):
        self.get_logger().info(
            f"Linear: {vel.linear.x:.3f}  Angular: {vel.angular.z:.3f}"
        )

        dta = self.calc_drivetrain_action(vel.linear.x, vel.angular.z)

        self.get_logger().info(
            f"Left: {dta.left_motor:.3f}  Right: {dta.right_motor:.3f}"
        )

        self._apply_motor(self.left, dta.left_motor)
        self._apply_motor(self.right, dta.right_motor)

    def _apply_motor(self, motor: MotorDriver, value: float):
        if value > 0.0:
            motor.set("f", value)
        elif value < 0.0:
            motor.set("r", value)
        else:
            motor.set("s", 0.0)

    def _ramp_cb(self):
        self.left.ramp()
        self.right.ramp()

    def calc_drivetrain_action(self, linear, angular) -> DrivetrainAction:
        dta = DrivetrainAction()

        dta.left_motor = +linear + angular
        dta.right_motor = +linear - angular

        excession = (dta.clamp_motor_value("left", -1.0, 1.0) - dta.left_motor) + (
            dta.clamp_motor_value("right", -1.0, 1.0) - dta.right_motor
        )
        dta.left_motor += excession
        dta.right_motor += excession

        return dta

    def destroy_node(self):
        self.left.cleanup()
        self.right.cleanup()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    wheel_driver = WheelDriver()
    rclpy.spin(wheel_driver)
    wheel_driver.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
