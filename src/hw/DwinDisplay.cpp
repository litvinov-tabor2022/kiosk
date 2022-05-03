#include <string>
#include <locale>
#include <codecvt>

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
static const u8 SetPage[4] = {0x00, 0x84, 0x5A, 0X01};
static const u8 WriteOp = 0x82;
static const u8 ReadOp = 0x83;
static const u8 TextColorSpOffset = 0x0003;

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
        std::lock_guard<std::mutex> lg_(HwLocks::DWIN_SERIAL);
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
    std::lock_guard<std::mutex> lg(opMutex);

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

bool DwinDisplay::writeIntVar(u16 addr, u16 value) {
    std::lock_guard<std::mutex> lg(opMutex);

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

bool DwinDisplay::writeRawVar(u16 addr, const u8 *data, const u8 len) {
    std::lock_guard<std::mutex> lg(opMutex);

    u8 bAdrL, bAdrH;

    bAdrL = addr & 0xFF;
    bAdrH = (addr >> 8) & 0xFF;

    memcpy(outBuff, Header, 2);
    outBuff[2] = 3 + len;
    outBuff[3] = WriteOp;
    outBuff[4] = bAdrH;
    outBuff[5] = bAdrL;
    memcpy(&outBuff[6], data, len);
    outBuffSize = 6 + len;

    sendAndWaitForResponse();

    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0);
}

bool DwinDisplay::writeTextVar(u16 addr, const std::string &text) {
    std::lock_guard<std::mutex> lg(opMutex);

    u8 bAdrL, bAdrH;

    bAdrL = addr & 0xFF;
    bAdrH = (addr >> 8) & 0xFF;

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
    std::u16string utf16 = utf16conv.from_bytes(text);

    memcpy(outBuff, Header, 2);
    // outBuff[2] is filled later
    outBuff[3] = WriteOp;
    outBuff[4] = bAdrH;
    outBuff[5] = bAdrL;

    outBuffSize = 6;
    for (char16_t c: utf16) {
        outBuff[outBuffSize++] = (c >> 8) & 0xFF;
        outBuff[outBuffSize++] = c & 0xFF;
    }

    // terminating zero!!!
    outBuff[outBuffSize++] = 0x00;
    outBuff[outBuffSize++] = 0x00;

    outBuff[2] = outBuffSize - 3; // 3 == header + op

    sendAndWaitForResponse();

    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0);
}

bool DwinDisplay::setTextDisplayColor(u16 spAddr, const Color color) {
    return writeIntVar(spAddr + TextColorSpOffset, toHighColor(color.r, color.g, color.b));
}

bool DwinDisplay::setPage(u8 no) {
    std::lock_guard<std::mutex> lg(opMutex);

    memcpy(outBuff, Header, 2);
    outBuff[2] = 0x07;
    outBuff[3] = WriteOp;
    memcpy(&outBuff[4], SetPage, 4);
    outBuff[8] = (no >> 8) & 0xFF;
    outBuff[9] = no & 0XFF;
    outBuffSize = 10;

    sendAndWaitForResponse();

    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0);
}

bool DwinDisplay::reset() {
    std::lock_guard<std::mutex> lg(opMutex);

    memcpy(outBuff, Header, 2);
    outBuff[2] = 0x07; // length
    outBuff[3] = WriteOp;
    outBuff[4] = 0x00;
    outBuff[5] = 0x04;
    outBuff[6] = 0x55;
    outBuff[7] = 0xAA;
    outBuff[8] = 0x5A;
    outBuff[9] = 0xA5;
    outBuffSize = 10;

    sendAndWaitForResponse();

    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0) && checkConnectivity();
}

bool DwinDisplay::beep(const u16 millis) {
    return true;
    // TODO use ;-)
//    std::lock_guard<std::mutex> lg(opMutex);
//
//    memcpy(outBuff, Header, 2);
//    outBuff[2] = 0x05; // length
//    outBuff[3] = WriteOp;
//    outBuff[4] = 0x00;
//    outBuff[5] = 0xA0;
//    outBuff[6] = 0x00;
//    outBuff[7] = millis / 8;
//    outBuffSize = 8;
//
//    sendAndWaitForResponse();
//
//    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0);
}

bool DwinDisplay::setBrightness(u8 level) {
    std::lock_guard<std::mutex> lg(opMutex);

    level = map(level, 0, 255, 0, 0x64);

    memcpy(outBuff, Header, 2);
    outBuff[2] = 0x05; // length
    outBuff[3] = WriteOp;
    outBuff[4] = 0x00;
    outBuff[5] = 0x82;
    outBuff[6] = level;
    outBuff[7] = level;
    outBuffSize = 8;

    sendAndWaitForResponse();

    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0);
}

bool DwinDisplay::disableBeeping() {
    std::lock_guard<std::mutex> lg(opMutex);

    memcpy(outBuff, Header, 2);
    outBuff[2] = 0x07; // length
    outBuff[3] = WriteOp;
    outBuff[4] = 0x00;
    outBuff[5] = 0x80;
    outBuff[6] = 0x5A;
    outBuff[7] = 0x00;
    outBuff[8] = 0x00;
    outBuff[9] = 0x30;
    outBuffSize = 10;

    sendAndWaitForResponse();

    return (inBuffSize == 3 && memcmp(SuccessfulWrite, inBuff, 3) == 0);
}

void DwinDisplay::sendAndWaitForResponse() {
    std::lock_guard<std::mutex> lg_(HwLocks::DWIN_SERIAL);

    if (DWIN_DEBUG) {
        Serial.printf("DWIN: Raw data sent (%d B): 0x", outBuffSize);
        dumpOutBuff();
        Serial.println();
    }

    hwSerial.write(outBuff, outBuffSize);

    // TODO implement timeout
    // TODO check for async data :-|

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
            Serial.printf("DWIN: Raw data received (%d B): 0x", inBuffSize);
            dumpInBuff();
            Serial.println();
        }

        return;
    }

    // fuck, invalid frame header!!
    Serial.println("DWIN: Invalid data received!!");
}

u8 DwinDisplay::readAsyncData(u16 *addr, u8 *dest) {
    std::lock_guard<std::mutex> lg(opMutex);
    std::lock_guard<std::mutex> lg_(HwLocks::DWIN_SERIAL);

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

u16 DwinDisplay::toHighColor(const u8 r, const u8 g, const u8 b) {
    // FFFFFF -> FFFF = 11111 111111 11111
    // C00000 -> C000 = 11000 000000 00000 ( 1100 0000 -> 1 1000 = x >> 3 )
    // 0000FF -> 001F = 00000 000000 11111
    // FF0000 -> F800 = 11111 000000 00000
    // 00C000 -> 0600 = 00000 110000 00000 ( 1100 0000 -> 11 0000 = x >> 2 )

    // I REALLY don't want to study operators precedence here... brackets to the rescue.
    return (((r >> 3) & 0x1F) << 11) + (((g >> 2) & 0x3F) << 5) + (((b >> 3) & 0x1F));
}
