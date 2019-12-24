// due to the nature of real-time monitoring, this code executes slightly differently from the RPi code
// this is due to the fact that the NODEMCU esentially listens "all the time" to the digital pinMode
// whereas the Pi listens as part of a loop and it runs slower meaning there is a "gap" in the listening
// This will essentially make the red LED come on with sound just like the onboard LED on the sensor.

#define soundSensor 23
#define REDLED 21
#define GREENLED 22
boolean LEDStatus=false;

void setup() {
 pinMode(soundSensor,INPUT);
 pinMode(REDLED,OUTPUT);
 pinMode(GREENLED,OUTPUT);
 digitalWrite(GREENLED, HIGH); // Green LED on to show program is running
 Serial.begin(9600);
 Serial.println("preparing to listen for sounds");

}

void loop() {

  int SensorData=digitalRead(soundSensor); 
  if(SensorData==1){

    if(LEDStatus==false){
        LEDStatus=true;
        digitalWrite(REDLED,HIGH); // this turns on the LED when sound it detected
    }
    else{
        LEDStatus=false;
        digitalWrite(REDLED,LOW); //when the sound stops the LED will turn off
    }
  }
 } 
