#include <Arduino.h>
#include <Adafruit_SleepyDog.h>
#include "config.h"

extern void tvbgSetup();
extern void sendAllCodes();
extern void quickflashLEDx( uint8_t x );

// Count for button presses
volatile int buttonPressed = 0;

// When to start the TV shutdown
int64_t shutdownAt = 0;

// Can't use millis() to count elapsed time since the deepsleep stops the CPU
int64_t currentTime = 0;

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

void flashLed(int64_t seconds, int ms = 500) {
    for (int64_t i = 0; i < seconds * 1000 && !buttonPressed; i += ms * 2) {
        analogWrite(TVT_STATUS_LED_PIN, TVT_STATUS_WARNING_BRIGHTNESS);
        digitalWrite(TVT_ONBOARD_LED_PIN, HIGH);

        if (!buttonPressed) {
            delay(ms);
        }
        
        analogWrite(TVT_STATUS_LED_PIN, 0);
        digitalWrite(TVT_ONBOARD_LED_PIN, LOW);

        if (!buttonPressed) {
            delay(ms);
        }
    }
}

void pulseLed(int64_t seconds, int ms = 500) {
    int step = max(ms / TVT_STATUS_WARNING_BRIGHTNESS, 1);
    
    for (int64_t i = 0; i < seconds * 1000 && !buttonPressed; i += ms * 2) {
        for (int j = 0; j <= TVT_STATUS_WARNING_BRIGHTNESS * 2 && !buttonPressed; j++) {
            double x = (sin((j / (TVT_STATUS_WARNING_BRIGHTNESS * 2.)) * (PI * 2.) - (PI / 2.)) + 1.) / 2.;
            analogWrite(TVT_STATUS_LED_PIN, (int)(x * TVT_STATUS_WARNING_BRIGHTNESS));
            delay(step);
        }
    }
}

void resetTimer() {
    shutdownAt = currentTime + TVT_SLEEP_TIMER_SECONDS * 1000;
}

void shutdown(bool finished = false) {
    delay(1000);
    buttonPressed = 0;
    
    if (!finished) {
        flashLed(3, 150);
    }

    analogWrite(TVT_STATUS_LED_PIN, TVT_STATUS_WARNING_BRIGHTNESS);
    digitalWrite(TVT_ONBOARD_LED_PIN, HIGH);

    digitalWrite(TVT_SHUTDOWN_PIN, LOW);
    pinMode(TVT_SHUTDOWN_PIN, OUTPUT);
    
    delay(10000);
    
    while (true) {
        pulseLed(10, 250);
    }
}

void setup() {
    pinMode(TVT_ONBOARD_LED_PIN, OUTPUT);
    pinMode(TVT_STATUS_LED_PIN, OUTPUT);
    
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
        pulseLed(TVT_WARNING_BLINK_SECONDS, TVT_STATUS_WARNING_INTERVAL);
        
        if (!buttonPressed) {
            sendAllCodes();
        }

        if (!buttonPressed) {
            resetTimer();
            shutdown(true);
        }
    }
}
