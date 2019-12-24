#include <DHT.h>
#include <DHT_U.h>
#include <time.h>
#include <WiFi.h>
#include <PubSubClient.h>

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

// declare our Wifi and MQTT connections and other constant settings for the network
const char* ssid     = "International_House_of_Corgi_24";            // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "ElwoodIsBigAndFat";                          // The password of the Wi-Fi network
const char* mqtt_server = "192.168.1.210";                           // The target mqtt server
String clientId = "GP1";                                             // The client ID of this collector - if you added more you would need to change this on each one

// declare our Wifi and MQTT connections
WiFiClient espClient;
PubSubClient client(espClient);

//
// Reconnects to the MQTT message-bus if the connection died, or we're
// not otherwise connected.
//
void reconnectMQ() {

  digitalWrite(net, LOW);  // turn the Blue NET LED Off

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

  digitalWrite(net, HIGH);  // turn the Blue NET LED on
  delay(5000);
}

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  WiFi.begin(ssid, password);             // Connect to the wifi network

  while (WiFi.status() != WL_CONNECTED) // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print('.');

  Serial.println('\n');
  Serial.println("Connected!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP32 to the serial port
  
}

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate as 9600 bps
  
  // These Options control the precision of the reading, and the voltage reduction capability of the ESP32
  // Attenuation number 3 is 11DB
  analogSetWidth(10);
  analogSetAttenuation((adc_attenuation_t)3);

  // Define the MQTT Server connection settings and then launch the MQTT Connection
  client.setServer(mqtt_server, 1883);
  
  // Set output and input pins to correct modes
  pinMode (power, OUTPUT);
  pinMode (exec, OUTPUT);
  pinMode (net, OUTPUT);
  dht.begin();
  
  //Turn on Green Power light
  digitalWrite (power, HIGH);
  Serial.println("Starting up");
  
  //Connect to the wireless network
  setup_wifi();
}

//the main loop is where all of the work gets done in a c/c++ program
void loop() {
  //Turn on RED EXEC light - this means a sample is being taken
  digitalWrite (exec, HIGH);

  //DHT11 read and collect variables
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();
  // Check if any reads failed and if so alert and set values to 0 so the process can continue
  if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      temperature = 0;
      humidity = 0;
      dtostrf(humidity, 4, 2, str_humidity);
      dtostrf(temperature, 4, 2, str_temperature);
  } else {
      dtostrf(temperature, 4, 2, str_temperature);
      dtostrf(humidity, 4, 2, str_humidity);
      // print the result to Terminal
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" *C ");
  }

  //Read value and print to serial for S1

    float soil_moisture = analogRead(probe);
    dtostrf(soil_moisture, 4, 2, s1);
    Serial.print("Soil moisture level (raw): ["); Serial.print(s1); Serial.println("]");
   
    int L1 = analogRead(light);
    Serial.print("Light level: ");
    Serial.print(L1);
    Serial.println();
  

  if (!client.connected()) {
    reconnectMQ();
  }
  
  placeholder_value=sprintf(data0, "{\"Message\":\"Sensor1\", \"Sensors\": {\"S1\":\"%s\"}}", s1);
  Serial.println("Publishing message 1:");
  while (!client.publish("garden1", data0)) {
    Serial.print(".");
  }

  placeholder_value=sprintf(data2, "{\"Message\":\"Temp_Hum_Light\", \"Sensors\": {\"Humidity\":\"%s\", \"Temperature\":\"%s\", \"Light\":\"%d\"}}", str_humidity, str_temperature, L1);
  Serial.println("Publishing message 3:");
  while (!client.publish("garden1", data2)){
    Serial.print(".");
  }

  //Turn off RED EXEC light - this means a sample is done
  digitalWrite (exec, LOW);
  digitalWrite (net, LOW);
  
  delay(timer_value);
  ESP.restart();
}
