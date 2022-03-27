#ifndef KIOSK_DWINDISPLAY_H
#define KIOSK_DWINDISPLAY_H

#include "../types.h"
#include "HwLocks.h"

#define RX2 16
#define TX2 17

class DwinDisplay {
public:
    void begin();

    bool readVar(u64 addr, u8 *dest, u8 len);

    bool writeVar(u64 addr, u64 value);

    bool setPage(u8 no);

private:
    void sendAndWaitForResponse();

    HardwareSerial hwSerial = HardwareSerial(2);

};


#endif //KIOSK_DWINDISPLAY_H
