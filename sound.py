# Import libraries used in this program
import RPi.GPIO as GPIO
import time


# Startup message
print "Preparing to monitor sound levels"

# Set our pin assignments
sensor_in = 18
red_led = 21
green_led = 20

# Setup the pin modes for IO
GPIO.setmode(GPIO.BCM)
GPIO.setup(sensor_in, GPIO.IN)
GPIO.setup(red_led, GPIO.OUT)
GPIO.setup(green_led, GPIO.OUT)

#turn on the green LED so you know we are working
GPIO.output(green_led, GPIO.HIGH)

# Define a listener on the sound sensor to wait for a signal
def callback(sensor_in):
        if GPIO.input(sensor_in):
                print "Sound detected!"
                GPIO.output(red_led, GPIO.HIGH)
        else:
                print "No sound at this time"
                GPIO.output(red_led, GPIO.LOW)

GPIO.add_event_detect(sensor_in, GPIO.BOTH, bouncetime=300)  # let us know when the pin goes HIGH or LOW
GPIO.add_event_callback(sensor_in, callback)  # assign function to GPIO PIN, Run function on change

# infinite loop
while True:
        time.sleep(1)