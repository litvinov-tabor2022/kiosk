#ifndef KIOSK_KIOSK_H
#define KIOSK_KIOSK_H

#include <hw/DwinDisplay.h>
#include <PortalFramework.h>
#include <Resources.h>

class PagesManager;

class Kiosk {
public:
    Kiosk();

    bool begin();

    [[nodiscard]] bool isAdminMode() const { return adminMode; }

    void disableAdminMode();

    [[nodiscard]] PlayerData getLastPlayerData() const { return lastPlayerData; }

    PortalFramework framework;
    DwinDisplay display;
private:
    void handleConnectedTag(PlayerData playerData);

    void handleDisconnectedTag();

    void enableAdminMode();

    bool adminMode = false;

    PlayerData lastPlayerData;

    PagesManager *pagesManager;
};


#endif //KIOSK_KIOSK_H
