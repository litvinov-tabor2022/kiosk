#include "Kiosk.h"

#define DEFAULT_PAGE Page_BonusPoints

bool Kiosk::begin() {
    if (!display.begin([this](const u16 addr, const u8 *data, const u8 dataLen) {
        pagesManager.handleAsyncDisplayData(addr, data, dataLen);
    })) {
        Debug.println("Could not initialize the display!");
        return false;
    }

    if (!framework.begin()) {
        Debug.println("Could not initialize Portal framework!");
        return false;
    }

    if (!pagesManager.begin(DEFAULT_PAGE)) {
        Debug.println("Could not initialize pages manager!");
        return false;
    }

    // setup

    if (!display.setBrightness(255)) {
        Debug.println("Could not set brightness");
        return false;
    }

    if (!display.disableBeeping()) {
        Debug.println("Could not disable beeping");
        return false;
    }

    if (!display.beep(100)) {
        Debug.println("Could not beep");
        return false;
    }

    framework.addOnConnectCallback([](PlayerData playerData) {
        Debug.printf("Connected player: ID %d, strength %d\n", playerData.user_id, playerData.strength);
    });

    return true;
}
