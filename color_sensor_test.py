import time
import board
import adafruit_tcs34725

# Create I2C bus and sensor instance
i2c = board.I2C()  
sensor = adafruit_tcs34725.TCS34725(i2c)

# Optional: Enable the LED
# sensor.led = True

while True:
    # Read color components (R, G, B, Clear)
    color = sensor.color_rgb_bytes
    lux = sensor.lux
    temp = sensor.color_temperature
    
    print(f"RGB: {color} | Temp: {temp}K | Lux: {lux}")
    time.sleep(1)
