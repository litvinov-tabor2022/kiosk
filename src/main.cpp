#include <Arduino.h>

#include <Kiosk.h>
//#include <ArduinoJson.h>

//byte buffer[255];
//
//void dumpBuffer(const u8 len = 255) {
//    for (int n = 0; n < len; n++) {
//        if (buffer[n] < 0x10) Serial.print("0");
//        Serial.print(buffer[n], HEX);
//    }
//}

Kiosk kiosk;

//void loadSkillJson(u8 id) {
//    File file = SPIFFS.open("/skills.json", FILE_READ);
//
//    if (!file) {
//        Debug.printf("Couldn't load requested file %s!\n", "/skills.json");
//        return;
//    }
//
//    DynamicJsonDocument doc(file.size() * 2);
//
//    const DeserializationError err = deserializeJson(doc, file);
//    if (err) {
//        Debug.printf("ERR loading JSON file %s: '%s'\n", "/skills.json", err.c_str());
//        return;
//    }
//
//    auto skill = doc[String(id)];
//
//    std::string name = skill["name"];
//    std::string description = skill["description"];
//
//    if (!display.writeTextVar(0x1100, name)) {
//        Serial.println("Could not write text");
//    }
//    if (!display.writeTextVar(0x1200, description)) {
//        Serial.println("Could not write text");
//    }
//}

void setup() {
    Serial.begin(115200);
    SPIFFS.begin();
    delay(500);

    Serial.println("Initializing...");

    if (!kiosk.begin()) {
        Debug.println("Could not initialize pages manager!");
        return;
    }

    Serial.println("Started!");
}

void loop() {}
