// Define the pin where the LED is connected
#include <Arduino.h>
const int ledPin = 11;

// Variable to store the last time the LED was toggled
unsigned long previousMillis = 0;

// Interval (delay time) for LED blink in milliseconds
const unsigned long interval = 1000; // 1000 ms = 1 second

// Variable to track LED state (ON or OFF)
bool ledState = LOW;

void setup() {
    pinMode(ledPin, OUTPUT); // Set pin 11 as an OUTPUT
}

void loop() {
    // Get the current time in milliseconds
    unsigned long currentMillis = millis();

    // Check if the interval time has passed
    if (currentMillis - previousMillis >= interval) {
        // Save the last time the LED was toggled
        previousMillis = currentMillis;

        // Toggle the LED state (ON becomes OFF, OFF becomes ON)
        ledState = !ledState;

        // Apply the new LED state to the pin
        digitalWrite(ledPin, ledState);
    }
}
