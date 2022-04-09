#include <Arduino.h>

#include <hw/DwinDisplay.h>
#include <pages/PagesManager.h>
#include <PortalFramework.h>
#include <ArduinoJson.h>

//byte buffer[255];
//
//void dumpBuffer(const u8 len = 255) {
//    for (int n = 0; n < len; n++) {
//        if (buffer[n] < 0x10) Serial.print("0");
//        Serial.print(buffer[n], HEX);
//    }
//}

DwinDisplay display;
PortalFramework framework;
PagesManager pagesManager(&display);

void loadSkillJson(u8 id) {
    File file = SPIFFS.open("/skills.json", FILE_READ);

    if (!file) {
        Debug.printf("Couldn't load requested file %s!\n", "/skills.json");
        return;
    }

    DynamicJsonDocument doc(file.size() * 2);

    const DeserializationError err = deserializeJson(doc, file);
    if (err) {
        Debug.printf("ERR loading JSON file %s: '%s'\n", "/skills.json", err.c_str());
        return;
    }

    auto skill = doc[String(id)];

    std::string name = skill["name"];
    std::string description = skill["description"];

    if (!display.writeTextVar(0x1100, name)) {
        Serial.println("Could not write text");
    }
    if (!display.writeTextVar(0x1200, description)) {
        Serial.println("Could not write text");
    }
}

// this is just a syntax-conveniency method...
void receiveAsyncData(const u16 addr, const u8 *data, const u8 dataLen) {
    pagesManager.handleAsyncDisplayData(addr, data, dataLen);
}

void setup() {
    Serial.begin(115200);
    SPIFFS.begin();
    delay(500);

    Serial.println("Initializing...");

    if (!display.begin(receiveAsyncData)) {
        Debug.println("Could not initialize the display!");
        return;
    }

    if (!framework.begin()) {
        Debug.println("Could not initialize Portal framework!");
        return;
    }

    if (!pagesManager.begin(Page_BonusPoints)) {
        Debug.println("Could not initialize pages manager!");
    }

    Debug.println("Starting...");

    if (!display.setBrightness(255)) {
        Debug.println("Could not set brightness");
    }

    if (!display.disableBeeping()) {
        Debug.println("Could not disable beeping");
    }

    if (!display.beep(100)) {
        Debug.println("Could not beep");
    }

    if (!display.writeTextVar(0x1005, u8"Žluťoučký kůň úpěl ďábelské ódy.")) {
        Serial.println("Could not write text");
    }

    loadSkillJson(1);

    framework.addOnConnectCallback([](PlayerData playerData) {
        Debug.printf("Connected player: ID %d, strength %d\n", playerData.user_id, playerData.strength);
    });

    Serial.println("Started!");
}

void loop() {}
