#include <Arduino.h>

#include <hw/DwinDisplay.h>

#define RX2 16
#define TX2 17

static HardwareSerial hwSerial(2);

//void DGUS_Beep(byte bTime) {
//    hwSerial.write(0x5A);
//    hwSerial.write(0xA5);
//    hwSerial.write(0x03);
//    hwSerial.write(0x80);
//    hwSerial.write(0x02);
//    hwSerial.write(bTime);
//}
//
//void DGUS_LED_Bright(byte bVal) {
//    if (bVal > 0x40) bVal = 0x40;
//    hwSerial.write(0x5A);
//    hwSerial.write(0xA5);
//    hwSerial.write(0x03);
//    hwSerial.write(0x80);
//    hwSerial.write(0x01);
//    hwSerial.write(bVal);
//}

DwinDisplay display;

void setup() {
    Serial.begin(115200);

    display.begin();

    delay(500);

    Serial.println("start");

    byte buffer[255];

    if (!display.readVar(0x1000, buffer, 1)) {
        Serial.println("err! 1");
    }

    if (!display.writeVar(0x1000, 0x9999)) {
        Serial.println("err! 2");
    }

    if (!display.readVar(0x1000, buffer, 1)) {
        Serial.println("err! 3");
    }

}

void loop() {
    if (!display.setPage(0)) {
        Serial.println("err! 4");
    }
    delay(500);

    if (!display.setPage(1)) {
        Serial.println("err! 5");
    }
    delay(500);
}

