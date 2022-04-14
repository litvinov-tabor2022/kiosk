#include <Arduino.h>

#include <Kiosk.h>

Kiosk kiosk;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("Initializing...");

    if (!kiosk.begin()) {
        Debug.println("Could not initialize the kiosk!");
        return;
    }

    Serial.println("Started!");
}

void loop() {}
