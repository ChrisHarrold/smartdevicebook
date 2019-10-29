#define LED 22  //instead of remembering which pin this makes it simpler to reference later in the code
int loops = 0;

void setup() {
    pinMode(LED, OUTPUT);    // LED pin as output.
    Serial.begin(9600);       //turn on the serial output - not explicitly required, but nice to have for debugging
}

void loop() {

    digitalWrite(LED, HIGH);    // turn the LED on
    delay(500);                 // wait for 1/2 second.
    digitalWrite(LED, LOW);     // turn the LED off
    delay(500);                 // wait for 1/2 second.
    loops = loops + 1;          // count the loops
    Serial.print(loops);        // print the loop counter to the serial output
    Serial.println(" completed."); 

}
