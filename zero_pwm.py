from gpiozero import Servo
from gpiozero import Motor
from gpiozero import LED
from gpiozero import PWMOutputDevice
from time import sleep

forward = LED(27)
backward = LED(17)
#servo = Servo(20)
#left_motor = Motor(17, 27)
#left_wheel_pwm = Servo(14)
pwm = PWMOutputDevice(14, frequency=50)

#while True:
#    servo.min()
#    sleep(1)
#    servo.mid()
#    sleep(1)
#    servo.max()
#    sleep(1)

#forward.on()
#backward.off()
#left_motor.forward()
forward.on()
backward.off()
#pwm.value = 1
#off = 0.0
#on = 1.0
#step = 0.1
pwm.value = 1
#while (pwm.value < on):
#    pwm.value += step
#    sleep(0.1)
sleep(5)
#while True:
#    left_wheel_pwm.value = 1
#    left_motor.forward()
#    servo.value = 0.1
#    left_motor.value = 1
#    servo.mid()
#    sleep(1)
#    servo.max()
#    sleep(1)
#servo.value = 0
pwm.value = 0
sleep(1)
