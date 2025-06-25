#include <Arduino.h>

uint32_t lastMilis = 0;
uint32_t currentMillis = 0;
int counter = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the built-in LED pin
  Serial.println("Starting the counter...");
}

void loop() {
  currentMillis = millis();
  if (currentMillis - lastMilis >= 1000) { // Check if 1 second has passed
    lastMilis = currentMillis; // Update lastMilis to current time
    Serial.print("Counter: ");
    Serial.println(counter); // Print the counter value
    counter++; // Increment the counter
    ledState = !ledState; // Toggle the LED state
    digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW); // Update the
  }
}

