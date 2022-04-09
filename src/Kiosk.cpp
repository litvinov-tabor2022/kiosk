#include "Kiosk.h"

#include <pages/PagesManager.h>

Kiosk::Kiosk() {
    pagesManager = new PagesManager(this, &display, &framework);
}

bool Kiosk::begin() {
    if (!display.begin([this](const u16 addr, const u8 *data, const u8 dataLen) {
        pagesManager->handleAsyncDisplayData(addr, data, dataLen);
    })) {
        Debug.println("Could not initialize the display!");
        return false;
    }

    if (!framework.begin()) {
        Debug.println("Could not initialize Portal framework!");
        return false;
    }

    if (!pagesManager->begin()) {
        Debug.println("Could not initialize pages manager!");
        return false;
    }

    // setup

    if (!display.setBrightness(0)) {
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

    framework.addOnConnectCallback([this](PlayerData playerData) {
        Debug.printf("Connected player: ID %d, strength %d\n", playerData.user_id, playerData.strength);
        handleConnectedTag(playerData);
    });

    framework.addOnDisconnectCallback([this] { handleDisconnectedTag(); });

    return true;
}

void Kiosk::handleConnectedTag(PlayerData playerData) {
    if (!display.beep(100)) { Debug.println("Could not beep"); }
    display.setBrightness(255);

    if (playerData.user_id == ADMIN_USER_ID) {
        Debug.println("Admin tag inserted");

        if (!this->adminMode) enableAdminMode();
    } else {
        lastPlayerData = playerData;
        if (this->adminMode) {
            if (!pagesManager->reloadPage()) {
                Debug.println("Could not reload page");
            }
        } else {
            //TODO this will probably fail when having more than one page...
            pagesManager->switchPage(playerData.bonus_points > 0 ? Page_BonusPoints : Page_UserMain);
        }
    }
}

void Kiosk::handleDisconnectedTag() {
    Debug.println("Tag disconnected");

    if (this->adminMode) {
        if (!pagesManager->reloadPage()) {
            Debug.println("Could not reload page");
        }
    } else {
        pagesManager->switchPage(Page_Home);
        if (!display.setBrightness(0)) {
            Debug.println("Could not set brightness!");
        }
        if (!display.beep(100)) { Debug.println("Could not beep"); }
    }
}

void Kiosk::enableAdminMode() {
    this->adminMode = true;
    Debug.println("Switching on ADMIN mode");
    if (!pagesManager->switchPage(Page_Admin_Main)) {
        Debug.println("Could not show admin main page!");
        if (!display.beep(1000)) { Debug.println("Could not beep"); }
        // recovery :-(
        disableAdminMode();
    }
}

void Kiosk::disableAdminMode() {
    this->adminMode = false;
    if (!display.setBrightness(0)) {
        Debug.println("Could not set brightness!");
    }
    Debug.println("Switching off ADMIN mode");
    if (!pagesManager->switchPage(Page_Home)) {
        Debug.println("Could not show home page!");
        if (!display.beep(1000)) { Debug.println("Could not beep"); }
        return;
    }

    if (!display.beep(100)) { Debug.println("Could not beep"); }
}

