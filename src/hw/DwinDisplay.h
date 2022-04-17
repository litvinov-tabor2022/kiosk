#ifndef KIOSK_DWINDISPLAY_H
#define KIOSK_DWINDISPLAY_H

#include "types.h"
#include "HwLocks.h"
#include <vector>
#include <mutex>

struct Color;

class DwinDisplay {
public:
    bool begin(const std::function<void(u16 addr, u8 *dataDest, u8 dataLen)> &asyncDataCallback);

    bool readVar(u16 addr, u8 *dest, u8 len);

    bool writeIntVar(u16 addr, u16 value);

    bool writeRawVar(u16 addr, const u8 *data, u8 len);

    bool writeTextVar(u16 addr, const std::string &text);

    bool setTextDisplayColor(u16 spAddr, Color color);

    bool setPage(u8 no);

    bool beep(u16 millis);

    bool setBrightness(u8 level);

    bool disableBeeping();

    bool reset();


private:
    void sendAndWaitForResponse();

    u8 readAsyncData(u16 *addr, u8 *dest);

    HardwareSerial hwSerial = HardwareSerial(2);
    std::mutex opMutex;

    u8 asyncDataBuffer[255]{};

    bool checkConnectivity();

    u16 toHighColor(u8 r, u8 g, u8 b);
};

struct Color {
    const u8 r;
    const u8 g;
    const u8 b;
};

class Colors {
public:
    constexpr static const Color Black = Color{.r = 0x00, .g = 0x00, .b = 0x00};
    constexpr static const Color White = Color{.r = 0xFF, .g = 0xFF, .b = 0xFF};
    constexpr static const Color Gray = Color{.r = 0x88, .g = 0x88, .b = 0x88};
    constexpr static const Color Red = Color{.r = 0xFF, .g = 0x00, .b = 0x00};
    constexpr static const Color Green = Color{.r = 0x00, .g = 0x80, .b = 0x00};
    constexpr static const Color Blue = Color{.r = 0x00, .g = 0x00, .b = 0xFF};
    constexpr static const Color LightRed = Color{.r = 0xFF, .g = 0x50, .b = 0x50};
    constexpr static const Color LightGreen = Color{.r = 0x00, .g = 0xC0, .b = 0x00};
    constexpr static const Color LightBlue = Color{.r = 0x00, .g = 0x00, .b = 0xC0};
    constexpr static const Color LightGray = Color{.r = 0xAA, .g = 0xAA, .b = 0xAA};
    constexpr static const Color DarkGray = Color{.r = 0x33, .g = 0x33, .b = 0x33};
};

#endif //KIOSK_DWINDISPLAY_H
