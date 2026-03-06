from rpi_hardware_pwm import HardwarePWM
from time import sleep

pwm = HardwarePWM(pwm_channel=0, hz=100, chip=0)
pwm.start(20) # full duty cycle

sleep(20)

#pwm.change_duty_cycle(50)
#pwm.change_frequency(25_000)

pwm.stop()
