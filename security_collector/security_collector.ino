#include <WiFi.h>
#include <PubSubClient.h>

// Set GPIOs for LEDs and sensors
#define PIRSensor = 19
#define soundSensor 23
#define REDLED 21
#define GREENLED 22
#define timeSeconds 10
boolean LEDStatus=false;

// declare our Wifi and MQTT connections and other constant settings for the network
const char* ssid     = "YOUR_SSID_HERE";            // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "YOUR_PASSWORD_HERE";        // The password of the Wi-Fi network
const char* mqtt_server = "YOUR_MQTT_IP_HERE";      // The target mqtt server
String clientId = "Collector_1";

// declare our Wifi and MQTT connections
WiFiClient espClient;
PubSubClient client(espClient);

//Arrays to hold the data to send to our central station
char data0[50];
char data1[50];

// variables for timekeeping
unsigned long now = millis();
unsigned long motion_Last_Triggered = 0;
boolean Timer_Start = false;
boolean Sound_Alert = false;
boolean Alert_1 = false;

void reconnectMQ() {

    // If for some reason the connection to MQTT is lost, the Green LED turns off
    digitalWrite(GREENLED, LOW);

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi not connected - configuring");
        setup_wifi();
    } else {
        Serial.println("Wifi Connected - connecting MQTT");
    }

    // Attempt to connect
    Serial.println("Attempting to connect to MQTT Server...");
    while (!client.connected()) {
        if (client.connect(clientId.c_str())) {
        Serial.println("connected to MQTT server");
        } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
        }
    }

    // Once connected, publish an announcement...
    String payload = "{\"Unit\":\"Collector_1\", \"MQTT\":\"Reconnected\"}";
    payload.toCharArray(data0, (payload.length() + 1));
    client.publish("control", data0); //the "control" topic is just for notifications - change to fit your needs
    
    // Green LED on to alert that connection restablished
    digitalWrite(GREENLED, HIGH);
    
    delay(5000);
}

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.print(ssid); 
  Serial.println(" ...");

  WiFi.begin(ssid, password);             // Connect to the wifi network

  while (WiFi.status() != WL_CONNECTED) // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print('.');

  Serial.println('\n');
  Serial.println("Connected!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP32 to the serial port
  
}

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR MotionAlert() {
    Serial.println("Motion Detected!")
    Timer_Start = true;
    Alert_1 = true;
    motion_Last_Triggered = millis();
}

void setup() {
    Serial.begin(9600);
    Serial.println("preparing to listen for sounds and watch for motion");

    pinMode(soundSensor,INPUT);
    pinMode(REDLED,OUTPUT);
    pinMode(GREENLED,OUTPUT);
    
    pinMode(motionSensor, INPUT_PULLUP);
    
    // Interrupts allow the sensor to trigger even if the application is doing something else
    // this way the loop can just run and listen instead of stopping all the time - this is the real-time advantage
    attachInterrupt(digitalPinToInterrupt(PIRSensor), MotionAlert, RISING);

    // Define the MQTT Server connection settings
    client.setServer(mqtt_server, 1883);  
    //Connect to the wireless network and MQTT server (this also publishes an MQTT message on completion)
    setup_wifi();
    if (!client.connected()) {
        reconnectMQ();
    }

    // Green LED on to alert program running and all systems ready
    digitalWrite(GREENLED, HIGH);
}

void loop() {
  // Current time
  now = millis();

    int SensorData=digitalRead(soundSensor); 
    
    if(SensorData==1){
        if(LEDStatus==false){
            LEDStatus=true;
            digitalWrite(REDLED,HIGH); // this turns on the LED when sound is detected
            Serial.println("Sound Detected!")
            Sound_Alert = true;
        }
        else{
            LEDStatus=false;
            Sound_Alert = false;
            digitalWrite(REDLED,LOW); //when the sound stops the LED will turn off
        }
    }

    if(Timer_Start && (now - motion_Last_Triggered > (timeSeconds*1000))) {
        Timer_Start = false;
        Alert_1 = false;
        Serial.println("Motion Stopped")
    }

    // check the connection status to MQTT before publishing messages - reconnect if needed
    if (!client.connected()) {
        reconnectMQ();
    }

    if(Alert_1) {
        Serial.println("Sending Alert!");
        while (!client.publish("Security", \"Unit\":\"Collector_1\", \"Alert\":\"ACTIVE\""){
            Serial.print(".");
        } 
    } else {
        Serial.println("Alert Cleared");
        while (!client.publish("Security", \"Unit\":\"Collector_1\", \"Alert\":\"ENDED\""){
            Serial.print(".");
        }
    
    } 


}