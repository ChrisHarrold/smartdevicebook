#define soundSensor 6
#define REDLED 7
#define GREENLED 8
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
        Serial.println("Detected Sound");
        digitalWrite(REDLED,HIGH); // this turns on the LED when sound it detected
        //delay(2000); // wait 2 seconds
        //digitalWrite(LED, LOW); // Re-set the LED to being off again
    }
    else{
        LEDStatus=false;
        digitalWrite(REDLED,LOW); //when the sound stops the LED will turn off
    }
  }
 } 
