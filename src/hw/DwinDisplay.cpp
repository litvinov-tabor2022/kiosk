#include "DwinDisplay.h"

#define DWIN_DEBUG true

u8 inBuff[255];
u8 outBuff[255];
u8 inBuffSize = 0;
u8 outBuffSize = 0;

// some constants of the protocol

static const u8 Header[2] = {0x5A, 0xA5};
static const u8 SuccessfulWrite[3] = {0x82, 0x4F, 0x4B};
static const u8 SetPage[2] = {0x5A, 0X01};
static const u8 WriteOp = 0x82;
static const u8 ReadOp = 0x83;

// ------------------------------

void dumpInBuff() {
    for (int n = 0; n < inBuffSize; n++) {
        if (inBuff[n] < 0x10) Serial.print("0");
        Serial.print(inBuff[n], HEX);
    }
}

void dumpOutBuff() {
    for (int n = 0; n < outBuffSize; n++) {
        if (outBuff[n] < 0x10) Serial.print("0");
        Serial.print(outBuff[n], HEX);
    }
}

void DwinDisplay::begin() {
    hwSerial.begin(115200, SERIAL_8N1, RX2, TX2);
}

bool DwinDisplay::readVar(u64 addr, u8 *dest, u8 len) {
    u8 bAdrL, bAdrH;
    bAdrL = addr & 0xFF;
    bAdrH = (addr >> 8) & 0xFF;

    memcpy(outBuff, Header, 2);
    outBuff[2] = 0x04;
    outBuff[3] = ReadOp;
    outBuff[4] = bAdrH;
    outBuff[5] = bAdrL;
    outBuff[6] = len;
    outBuffSize = 7;

    sendAndWaitForResponse();

    const u8 expLen = 4 + len * 2;

    /*
     * Example:
     *
     * Sent:      5AA50483100001
     * Received:        831000019999
     * */
    return (inBuffSize == expLen && memcmp(&outBuff[3], inBuff, 4) == 0);
}

bool DwinDisplay::writeVar(u64 addr, u64 value) {
    u8 bAdrL, bAdrH, bValL, bValH;

    bAdrL = addr & 0xFF;
    bAdrH = (addr >> 8) & 0xFF;
    bValL = value & 0xFF;
    bValH = (value >> 8) & 0xFF;

    memcpy(outBuff, Header, 2);
    outBuff[2] = 0x05;
    outBuff[3] = WriteOp;
    outBuff[4] = bAdrH;
    outBuff[5] = bAdrL;
    outBuff[6] = bValH;
    outBuff[7] = bValL;
    outBuffSize = 8;

    sendAndWaitForResponse();

    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0);
}

bool DwinDisplay::setPage(u8 no) {
    memcpy(outBuff, Header, 2);
    outBuff[2] = 0x07;
    outBuff[3] = WriteOp;
    outBuff[4] = 0x00;
    outBuff[5] = 0x84;
    memcpy(&outBuff[6], SetPage, 2);
    outBuff[8] = (no >> 8) & 0xFF;
    outBuff[9] = no & 0XFF;
    outBuffSize = 10;

    sendAndWaitForResponse();

    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0);
}

void DwinDisplay::sendAndWaitForResponse() {
    std::lock_guard<std::mutex> lg(HwLocks::DWIN_SERIAL);

    if (DWIN_DEBUG) {
        Serial.print("DWIN: Raw data sent: 0x");
        dumpOutBuff();
        Serial.println();
    }

    hwSerial.write(outBuff, outBuffSize);

    // TODO implement timeout

    inBuffSize = 0;
    u8 i = 0;
    while (hwSerial.available() < 3) {} // wait until header arrives

    if (hwSerial.read() == 0x5A && hwSerial.read() == 0xA5) {
        const u8 iLen = hwSerial.read();

        while (hwSerial.available() < iLen); //Wait for the whole frame

        while (i < iLen) {  //Compile all frame
            inBuff[i] = hwSerial.read();
            inBuffSize++;
            i++;
        }

        if (DWIN_DEBUG) {
            Serial.print("DWIN: Raw data received: 0x");
            dumpInBuff();
            Serial.println();
        }

        return;
    }

    // fuck, invalid frame header!!
    Serial.println("DWIN: Invalid data received!!");
}

