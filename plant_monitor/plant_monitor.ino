#include <DHT.h>
#include <DHT_U.h>
#include <time.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ======YOU NEED TO CHANGE THIS SECTION TO MATCH YOUR NETWORK!!======================
// The SSID (name) of the Wi-Fi network you want to connect to
const char* ssid     = "International_House_of_Corgi_24";            
// The password of the Wi-Fi network
const char* password = "ElwoodIsBigAndFat";                          
// The target mqtt server - in theory this is your Raspberry Pi IP address, but can be whatever you are using
const char* mqtt_server = "192.168.1.210";                
// The client ID of this collector - unique to this collector but just an ID for MQTT to use
String clientId = "GP1";                                             
// ======================================================================================

//our sensor is DHT11 type
//creates an instance of DHT sensor
#define DHTPIN 17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Define the pins for the LED indicators and the light sensor
const int power = 22;
const int exec = 21;
const int net = 16;
const int light = 36;
const int probe = 34;
// define the wait time beteen readings (miliseconds) - the max value for this is 2147483647 which is roughly 25 days
// realistically a reading every 4 hours (14400000) for an outdoor garden 
// or 8 hours (28800000) for a potted plant is probably fine - by default the interval is 10 seconds. This is far too short
// and once you are satisfied with the readings you should for sure change it.
const int timer_value = 10000;

// everything text-related in C++ uses character arrays 
// <sarcasm>because strings weren't challenging enough...</sarcasm>
char data0[50]; //Will hold the soil moisture data
char data1[50]; //will hold "control" data updates for the MQTT system
char data2[50]; //will hold the temp, humidity and light data
char str_temperature[10]; //holds the temperature value
char str_humidity[10];  //holds the humidity value
char s1[8];  //holds the converted value from the soil moisture probe to be published via MQTT
float sm_value[8];  // holds the raw voltage reading off the soil moisture probe
int placeholder_value; // exactly what it looks like :-) just a plasceholder to get replaced with other data later

// declare our Wifi and MQTT connections
WiFiClient espClient;
PubSubClient client(espClient);

//
// Reconnects to the MQTT message-bus if the connection died, or we're
// not otherwise connected.
//
void reconnectMQ() {

  digitalWrite(net, LOW);  // turn the Blue NET LED Off if connection not established

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
  String payload = "{\"Unit\":\"G1\", \"MQTT\":\"Reconnected\"}";
  payload.toCharArray(data1, (payload.length() + 1));
  client.publish("control", data1); //the "control" topic is just for notifications - change to fit your needs

  digitalWrite(net, HIGH);  // turn the Blue NET LED on when reconnected
  delay(5000);  //sleep for 5 seconds to allow the connection to stabilize - not explicitly required but a good idea
}

//connect to the wifi network and obtain an IP address
void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  WiFi.begin(ssid, password);             // Connect to the wifi network

  while (WiFi.status() != WL_CONNECTED) // Wait for the Wi-Fi to connect and print a status while waiting
    delay(1000);
    Serial.print('.');

  Serial.println('\n');
  Serial.println("Connected!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Print the IP address of the ESP32 to the serial port
  
}

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate as 9600 bps
  
  // These Options control the precision of the analog reading, and the voltage reduction capability of the ESP32
  // Attenuation number 3 is 11DB and is the widely recommended setting 
  analogSetWidth(10);
  analogSetAttenuation((adc_attenuation_t)3);

  // Define the MQTT Server connection settings and then launch the MQTT Connection
  client.setServer(mqtt_server, 1883);
  
  // Set output and input pins to correct modes and start the dht sensor pin 
  pinMode (power, OUTPUT);
  pinMode (exec, OUTPUT);
  pinMode (net, OUTPUT);
  dht.begin();
  
  //Turn on Green "Power" light
  digitalWrite (power, HIGH);
  Serial.println("Starting up");
  
  //Connect to the wireless network
  setup_wifi();
}

//the main loop is where all of the work gets done in a c/c++ program
void loop() {
  //Turn on RED EXEC light - this means a sample is being taken
  digitalWrite (exec, HIGH);

  //DHT11 read and collect variables - float is a numeric type with better precision than int
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default - can be change to Farenheit if desired)
  float temperature = dht.readTemperature();
  // Check if any reads failed and if so alert to the console, 
  // and set values to 0 so the process can continue - DHT11 sensors are notoriously flaky
  if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      temperature = 0;
      humidity = 0;
      dtostrf(humidity, 4, 2, str_humidity);
      dtostrf(temperature, 4, 2, str_temperature);
  //actually got a clean reading so go down the happy path and format the reading for the MQTT message    
  } else {
      // dtostrf is a C++ function that formats strings with complex values
      dtostrf(temperature, 4, 2, str_temperature);
      dtostrf(humidity, 4, 2, str_humidity);
      // print the result sto Terminal for a sanity check - you can use this type of format for long strings
      // instead of single lines, although this might be a little less readable
      Serial.print("Humidity: "); Serial.print(humidity); Serial.print(" %\t"); 
      Serial.print("Temperature: "); Serial.print(temperature); 
      Serial.println(" *C ");
  }

  //Read value, convert to a string value, and print to serial for S1
  float soil_moisture = analogRead(probe);
  dtostrf(soil_moisture, 4, 2, s1);
  Serial.print("Soil moisture level (raw): ["); Serial.print(s1); Serial.println("]");
   
  // read the light sensor input and get the analog voltage level (1023 is full light, and anything less is, well, less) 
  int L1 = analogRead(light);
  Serial.print("Light level: "); Serial.print(L1); Serial.println();

  //defore I go ahead and publish messages to the MQTT server, always a good idea to check the connection and reconnect if needed
  if (!client.connected()) {
    reconnectMQ();
  }
  
  // publish the sensor value. the sprintf is also a string formatting tool in C++ - in this case allowing you to insert
  // a variable into a string. The string looks complex because it has to be JSON at the other end for NodeRed
  // there are other (perhaps more elegant) ways to skin this cat as it were, but this one worls solidly
  placeholder_value=sprintf(data0, "{\"Message\":\"Sensor1\", \"Sensors\": {\"S1\":\"%s\"}}", s1);
  Serial.println("Publishing message 1:");
  while (!client.publish("garden1", data0)) {
    Serial.print(".");
  }
// second message to MQTT with the temp/humidity/light values - MQTT payloads are fixed length, so breaking them up like this
// might not be optional if you collect a lot of data from the sensor pack
  placeholder_value=sprintf(data2, "{\"Message\":\"Temp_Hum_Light\", \"Sensors\": {\"Humidity\":\"%s\", \"Temperature\":\"%s\", \"Light\":\"%d\"}}", str_humidity, str_temperature, L1);
  Serial.println("Publishing message 3:");
  while (!client.publish("garden1", data2)){
    Serial.print(".");
  }

  //Turn off RED EXEC light - this means a sample is done
  digitalWrite (exec, LOW);
  digitalWrite (net, LOW);
  
  // sleep for the "timer_value"
  delay(timer_value);

  // reboot - I do not like that this has to happen, but there are serious conflicts in the MQTT library and the DHT11 
  // library and if you try and just loop back after the delay it will hang forever. This is a terrible "fix"
  // but I traded elegance for functionality. Also, you might want to use something like esp.deepSleep here
  // to save power so really, it will probably "reboot" every time anyway.
  ESP.restart();
}
