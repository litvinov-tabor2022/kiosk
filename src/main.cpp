#include <Arduino.h>

#include <hw/DwinDisplay.h>
#include <PortalFramework.h>

//byte buffer[255];
//
//void dumpBuffer(const u8 len = 255) {
//    for (int n = 0; n < len; n++) {
//        if (buffer[n] < 0x10) Serial.print("0");
//        Serial.print(buffer[n], HEX);
//    }
//}

DwinDisplay display;

u8 freePoints = 5;
PlayerData stats = PlayerData{.strength = 10, .magic = 10, .dexterity = 10};

void receiveAsyncData(const u16 addr, const u8 *data, const u8 dataLen) {
    if (addr == 0x1000) {
        const u8 value = data[1];
        if (value - stats.strength != 1) {
            Serial.println("ERROR - diff != 1 for strength");
            return;
        }
        stats.strength = value;
        freePoints--;
    }
    if (addr == 0x1001) {
        const u8 value = data[1];
        if (value - stats.dexterity != 1) {
            Serial.println("ERROR - diff != 1 for dexterity");
            return;
        }
        stats.dexterity = value;
        freePoints--;
    }
    if (addr == 0x1002) {
        const u8 value = data[1];
        if (value - stats.magic != 1) {
            Serial.println("ERROR - diff != 1 for magic");
            return;
        }
        stats.magic = value;
        freePoints--;
    }

    if (freePoints <= 0) display.setPage(0);
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("Initializing...");

    if (!display.begin(receiveAsyncData)) {
        Serial.println("Could not initialize the display!");
        return;
    }

    Serial.println("Starting...");

    if (!display.writeVar(0x1000, stats.strength)) {
        Serial.println("err! 1");
    }

    if (!display.writeVar(0x1001, stats.dexterity)) {
        Serial.println("err! 2");
    }

    if (!display.writeVar(0x1002, stats.magic)) {
        Serial.println("err! 3");
    }

    if (!display.setPage(1)) {
        Serial.println("Could not switch page!");
    }

    if (!display.setBrightness(255)) {
        Serial.println("Could not set brightness");
    }

    if (!display.disableBeeping()) {
        Serial.println("Could not disable beeping");
    }

    if (!display.beep(100)) {
        Serial.println("Could not beep");
    }

    Serial.println("Started!");
}

void loop() {}
