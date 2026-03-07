from rpi_hardware_pwm import HardwarePWM
from gpiozero import PWMLED
import time

# 1. Initialize Hardware PWM on channel 0, 50Hz (e.g., servo)
# On Pi 5, it is recommended to specify the chip, often chip=2
pwm = HardwarePWM(pwm_channel=0, hz=50, chip=2)

# 2. Start PWM with a duty cycle (0.0 to 100.0)
pwm.start(0) 

# Example: Controlling a servo (5% to 10% duty cycle for 50Hz)
try:
    while True:
        pwm.change_duty_cycle(5) # 0 degrees
        time.sleep(1)
        pwm.change_duty_cycle(10) # 180 degrees
        time.sleep(1)
except KeyboardInterrupt:
    pwm.stop()

