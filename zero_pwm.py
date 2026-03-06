from gpiozero import Servo
from gpiozero import Motor
from gpiozero import LED
from time import sleep

forward = LED(0)
backward = LED(1)
servo = Servo(15)
left_motor = Motor(17, 27)

#while True:
#    servo.min()
#    sleep(1)
#    servo.mid()
#    sleep(1)
#    servo.max()
#    sleep(1)

#forward.on()
#backward.off()
left_motor.forward()
while True:
#    servo.value = 0.1
    left_motor.value = 1
#servo.mid()
#sleep(0.5)
#servo.value = 0
