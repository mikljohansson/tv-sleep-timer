#include <Arduino.h>
#include <Adafruit_SleepyDog.h>
#include "config.h"

extern void tvbgSetup();
extern void sendAllCodes();
extern void quickflashLEDx( uint8_t x );

// Count for button presses
volatile int buttonPressed = 0;

// When to start the TV shutdown
unsigned long shutdownAt = 0;

// Can't use millis() to count elapsed time since the deepsleep stops the CPU
unsigned long currentTime = 0;

void handleCalibrateButtonPress() {
    buttonPressed += 1;
}

void deepSleep(int ms) {
    int slept = Watchdog.sleep(ms);
    if (slept < ms) {
        delay(max(ms - slept, 1));
    }

    currentTime += ms;
}

void flashLed(int seconds, int ms = 500) {
    for (int i = 0; i < seconds * 1000 && !buttonPressed; i += ms * 2) {
        analogWrite(TVT_STATUS_LED_PIN, TVT_STATUS_WARNING_BRIGHTNESS);
        digitalWrite(TVT_ONBOARD_LED_PIN, HIGH);

        if (!buttonPressed) {
            deepSleep(ms);
        }
        
        analogWrite(TVT_STATUS_LED_PIN, 0);
        digitalWrite(TVT_ONBOARD_LED_PIN, LOW);

        if (!buttonPressed) {
            deepSleep(ms);
        }
    }
}

void resetTimer() {
    shutdownAt = currentTime + TVT_SLEEP_TIMER_SECONDS * 1000;
}

void shutdown() {
    delay(1000);
    buttonPressed = 0;
    flashLed(3, 150);

    analogWrite(TVT_STATUS_LED_PIN, TVT_STATUS_WARNING_BRIGHTNESS);
    digitalWrite(TVT_ONBOARD_LED_PIN, HIGH);

    digitalWrite(TVT_SHUTDOWN_PIN, LOW);
    pinMode(TVT_SHUTDOWN_PIN, OUTPUT);
    
    while (true) {
        delay(1000);
    }
}

void setup() {
    pinMode(TVT_ONBOARD_LED_PIN, OUTPUT);
    
    // Set the shutdown pin in high impedance mode to avoid wasting power
    digitalWrite(TVT_SHUTDOWN_PIN, LOW);
    pinMode(TVT_SHUTDOWN_PIN, INPUT);

    // Hook up interrupt to the button
    digitalWrite(TVT_BUTTON_PIN, LOW);
    pinMode(TVT_BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(TVT_BUTTON_PIN), handleCalibrateButtonPress, RISING);

    // Initialize the IR LED and status LED pins
    tvbgSetup();

    // Wait for button to be released after starting up
    delay(500);
    buttonPressed = 0;
    
    // Confirm startup by blinking
    flashLed(3);

    // Start the timer
    buttonPressed = 0;
    resetTimer();
}

void loop() {
    if (buttonPressed) {
        deepSleep(500);
        
        if (buttonPressed >= 2) {
            resetTimer();
            shutdown();
        }
        
        buttonPressed = 0;
        resetTimer();
    }
    else {
        deepSleep(500);
    }

    if (shutdownAt < currentTime) {
        flashLed(TVT_WARNING_BLINK_SECONDS);
        
        if (!buttonPressed) {
            sendAllCodes();
        }

        if (!buttonPressed) {
            resetTimer();
            shutdown();
        }
    }
}
