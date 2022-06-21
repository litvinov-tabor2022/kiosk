#include "Kiosk.h"

#include <pages/PagesManager.h>

Kiosk::Kiosk() {
    pagesManager = new PagesManager(this);
}

bool Kiosk::begin() {
    if (!display.begin([this](const u16 addr, const u8 *data, const u8 dataLen) {
        pagesManager->handleAsyncDisplayData(addr, data, dataLen);
    })) {
        Debug.println("Could not initialize the display!");
        return false;
    }

    if (!pagesManager->begin()) {
        Debug.println("Could not initialize pages manager!");
        return false;
    }

    const std::optional<std::string> &frInit = framework.begin();
    if (frInit.has_value()) {
        Debug.println("Could not initialize Portal framework!");
        Debug.println(*frInit->c_str());
        pagesManager->showErrorPage(*frInit);
        return false;
    }

    if ((skillsList = framework.resources.loadSkillsList()) == nullptr) {
        Debug.println("Could not load skills list!");
        pagesManager->showErrorPage("Could not load skills list!");
        return false;
    }

    if ((playersMetadata = framework.resources.loadPlayersMeta()) == nullptr) {
        Debug.println("Could not load players metadata!");
        pagesManager->showErrorPage("Could not load players metadata!");
        return false;
    }

    // everything OK, let's setup

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

    framework.addOnConnectCallback([this](PlayerData playerData, const bool isReload) {
        // ignore the `isReload`, the code here counts with both options
        Debug.printf("Connected player: ID %d, strength %d\n", playerData.user_id, playerData.strength);
        handleConnectedTag(playerData);
    });

    framework.addOnDisconnectCallback([this] { handleDisconnectedTag(); });

    return true;
}

void Kiosk::handleConnectedTag(PlayerData playerData) {
    if (!display.beep(100)) { Debug.println("Could not beep"); }
    if (!display.setBrightness(255)) {
        Debug.println("Could not set brightness!");
    }

    if (playerData.user_id == ADMIN_USER_ID) {
        adminTagPresent = true;
        Debug.println("Admin tag inserted");

        if (!this->adminMode) enableAdminMode();
    } else {
        lastPlayerData = playerData;
        if (this->adminMode) {
            if (!pagesManager->reloadPage()) {
                Debug.println("Could not reload page");
            }
        } else {
            pagesManager->switchPage(playerData.bonus_points > 0 ? Page_BonusPoints : Page_UserMain);
        }
    }
}

void Kiosk::handleDisconnectedTag() {
    Debug.println("Tag disconnected");
    adminTagPresent = false; // definitely

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

PlayerMetadata Kiosk::getPlayerMetadata(const u8 userId) const {
    return playersMetadata->getPlayerMetadata(userId);
}

