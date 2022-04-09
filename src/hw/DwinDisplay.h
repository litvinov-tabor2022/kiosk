#ifndef KIOSK_DWINDISPLAY_H
#define KIOSK_DWINDISPLAY_H

#include "types.h"
#include "HwLocks.h"
#include <vector>
#include <mutex>

class DwinDisplay {
public:
    bool begin(const std::function<void(u16 addr, u8 *dataDest, u8 dataLen)> &asyncDataCallback);

    bool readVar(u16 addr, u8 *dest, u8 len);

    bool writeIntVar(u16 addr, u16 value);

    bool writeRawVar(u16 addr, const u8* data, u8 len);

    bool writeTextVar(u16 addr, const std::string& text);

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
};


#endif //KIOSK_DWINDISPLAY_H
