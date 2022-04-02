#ifndef KIOSK_DWINDISPLAY_H
#define KIOSK_DWINDISPLAY_H

#include "types.h"
#include "HwLocks.h"
#include <vector>

class DwinDisplay {
public:
    bool begin(const std::function<void(u16 addr, u8 *dataDest, u8 dataLen)> &asyncDataCallback);

    bool readVar(u16 addr, u8 *dest, u8 len);

    bool writeVar(u16 addr, u64 value);

    bool setPage(u8 no);

    bool beep(u16 millis);

    bool setBrightness(u8 level);

    bool reset();


private:
    void sendAndWaitForResponse();

    u8 readAsyncData(u16 *addr, u8 *dest);

    HardwareSerial hwSerial = HardwareSerial(2);

    u8 asyncDataBuffer[255]{};

    bool checkConnectivity();
};


#endif //KIOSK_DWINDISPLAY_H
