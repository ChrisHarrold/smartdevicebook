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

try:
    # Define a listener on the sound sensor to wait for a signal - this will cycle the LED as sound comes in
    def callback(sensor_in):
            if GPIO.input(sensor_in):
                    print "Sound detected - turn on"
                    GPIO.output(red_led, GPIO.HIGH)
            else:
                    print "Sound detected - turn off"
                    GPIO.output(red_led, GPIO.LOW)

    GPIO.add_event_detect(sensor_in, GPIO.BOTH, bouncetime=300)  # let us know when the pin goes HIGH or LOW
    GPIO.add_event_callback(sensor_in, callback)  # assign function to GPIO PIN, Run function on change

    # infinite loop
    while True:
            time.sleep(1)

except (KeyboardInterrupt, SystemExit):
	
	#If the system is interrupted (ctrl-c) this will print the final values
	#so that you have at least some idea of what happened
	print "-------------------------------------------"
	print " "
	print "System Reset on Keyboard Command or SysExit"
	print " "
	print "-------------------------------------------"
	GPIO.cleanup()

else:
	
	GPIO.cleanup()

        # You can remove this entire block once you go to "production" mode
		# but these values are critical for the initial tuning phase.
        print "-------------------------------------------"
        print " "
        print "System Reset on Keyboard Command or SysExit"
        print " "
        print "-------------------------------------------"