#ifndef KIOSK_KIOSK_H
#define KIOSK_KIOSK_H

#include <hw/DwinDisplay.h>
#include <PortalFramework.h>

class PagesManager;

class Kiosk {
public:
    Kiosk();

    bool begin();

    [[nodiscard]] bool isAdminMode() const { return adminMode; }

    void disableAdminMode();

    [[nodiscard]] PlayerData getLastPlayerData() const { return lastPlayerData; }

private:
    void handleConnectedTag(PlayerData playerData);

    void handleDisconnectedTag();

    void enableAdminMode();

    bool adminMode = false;

    PlayerData lastPlayerData;

    PortalFramework framework;
    DwinDisplay display;
    PagesManager *pagesManager;
};


#endif //KIOSK_KIOSK_H
