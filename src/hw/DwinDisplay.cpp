#include "DwinDisplay.h"
#include "Tasker.h"

#define DWIN_DEBUG false

#define RX2 16
#define TX2 17

u8 inBuff[255];
u8 outBuff[255];
u8 inBuffSize = 0;
u8 outBuffSize = 0;

// some constants of the protocol

static const u8 Header[2] = {0x5A, 0xA5};
static const u8 ExpectedCheckResponse[4] = {0x83, 0x00, 0x31, 0x01};
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

bool DwinDisplay::begin(const std::function<void(u16 addr, u8 *dataDest, u8 dataLen)> &asyncDataCallback) {
    hwSerial.begin(115200, SERIAL_8N1, RX2, TX2);

    setPage(1);

    if (!hwSerial.availableForWrite() || !checkConnectivity()) {
        Serial.println("Could not communicate with the display!");
        return false;
    }

    // regular check for async data
    Core0.loopEvery("asyncDataCheck", 50, [this, asyncDataCallback] {
        u16 asyncDataAddr;
        u8 asyncDataLen;

        if ((asyncDataLen = readAsyncData(&asyncDataAddr, asyncDataBuffer)) > 0) {
            asyncDataCallback(asyncDataAddr, asyncDataBuffer, asyncDataLen
            );
        }
    });

    return true;
}

bool DwinDisplay::checkConnectivity() {
    u16 asyncDataAddr;
    u8 asyncDataLen;

    Serial.println("Checking display connectivity...");

    // check it works... this reads current display brightness which we give no shit about, but it proves the communication works ;-)
    const u8 buffer[7] = {0x5A, 0xA5, 0x04, 0x83, 0x00, 0x31, 0x01};
    {
        std::lock_guard<std::mutex> lg(HwLocks::DWIN_SERIAL);
        for (u8 i: buffer) {
            hwSerial.write(i);
        }

        if (DWIN_DEBUG) Serial.println("DWIN: Check data sent, waiting for response");

        const u64 now = millis();
        while (!hwSerial.available() && (millis() - now) < 500) {} // wait up to 500ms for the data
    }

    if (DWIN_DEBUG) Serial.println("DWIN: Ready");

    if ((asyncDataLen = readAsyncData(&asyncDataAddr, asyncDataBuffer)) > 0) {
        if (memcmp(ExpectedCheckResponse, inBuff, 4) != 0) {
            Serial.printf("DWIN: Invalid data returned (%d B), comm not working!\n", asyncDataLen);
            return false;
        }
    } else {
        Serial.println("DWIN: No data returned, comm not working!");
        return false;
    }

    return true;
}

bool DwinDisplay::readVar(u16 addr, u8 *dest, u8 len) {
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
    // TODO check for async data :-|

    /*
     * Example:
     *
     * Sent:      5AA50483100001
     * Received:        831000019999
     * */

    const u8 expLen = 4 + len * 2; // 4B = 1B op.confirm + 2B addr + 1B data length

    if (inBuffSize == expLen && memcmp(&outBuff[3], inBuff, 4) == 0) {
        // if ok, copy result into dest and quit
        memcpy(dest, &inBuff[4], len * 2);  // data are stored in words
        return true;
    }

    return false;
}

bool DwinDisplay::writeVar(u16 addr, u64 value) {
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
    // TODO check for async data :-|

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

u8 DwinDisplay::readAsyncData(u16 *addr, u8 *dest) {
    std::lock_guard<std::mutex> lg(HwLocks::DWIN_SERIAL);

    if (hwSerial.available()) {
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
                Serial.print("DWIN: Raw data received (async): 0x");
                dumpInBuff();
                Serial.println();
            }

            *addr = (inBuff[1] << 8) + inBuff[2];
            const u8 realLen = inBuff[3] * 2; // data are stored in words

            memcpy(dest, &inBuff[4], realLen);

            return realLen;
        }

        // fuck, invalid frame header!!
        Serial.println("DWIN: Invalid data received!!");
    }

    return 0;
}

