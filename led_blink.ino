#define LED 22  //instead of remembering which pin this makes it simpler to reference later in the code
const int loops = 0;

void setup() {
    pinMode(LED, OUTPUT);    // LED pin as output.
}

void loop() {
    do {
        digitalWrite(LED, HIGH);    // turn the LED on
        delay(500);                 // wait for 1/2 second.
        digitalWrite(LED, LOW);     // turn the LED off
        delay(500);                 // wait for 1/2 second.
        loops = loops + 1;
    } while(loops <10);
    
}