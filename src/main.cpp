#include <Arduino.h>

#include <hw/DwinDisplay.h>
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

u8 freePoints = 5;
PlayerData stats = PlayerData{.user_id = 999, .strength = 10, .magic = 10, .dexterity = 10};

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
    if (addr == 0x2000) {
        loadSkillJson(map(data[1], 1, 100, 1, 5));
    }

    if (freePoints <= 0) display.setPage(0);
}

void setup() {
    Serial.begin(115200);
    SPIFFS.begin();
    delay(500);

    Serial.println("Initializing...");

    strcpy(stats.secret, TagSecret.c_str());

    if (!display.begin(receiveAsyncData)) {
        Serial.println("Could not initialize the display!");
        return;
    }

    if (!framework.begin()) {
        Serial.println("Could not initialize Portal framework!");
        return;
    }

    Serial.println("Starting...");

    if (!display.writeIntVar(0x1000, stats.strength)) {
        Serial.println("err! 1");
    }

    if (!display.writeIntVar(0x1001, stats.dexterity)) {
        Serial.println("err! 2");
    }

    if (!display.writeIntVar(0x1002, stats.magic)) {
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
