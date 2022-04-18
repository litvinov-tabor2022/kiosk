#ifndef KIOSK_BONUSPOINTSPAGE_H
#define KIOSK_BONUSPOINTSPAGE_H

#include <utility>

#include "pages/DisplayPage.h"
#include "PortalFramework.h"

#define PageAddrs Addrs::User::BonusPoints

class BonusPointsPage : public DisplayPage {
public:
    explicit BonusPointsPage(Kiosk *kiosk, std::function<void(const PageId pageId)> switchPage) :
            DisplayPage(kiosk, std::move(switchPage)) {};

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {
        //TODO create transaction

        switch (addr) {
            case PageAddrs::IncStrength:
                playerData.strength++;
                playerData.bonus_points--;
                break;
            case PageAddrs::IncDexterity:
                playerData.dexterity++;
                playerData.bonus_points--;
                break;
            case PageAddrs::IncMagic:
                playerData.magic++;
                playerData.bonus_points--;
                break;
        }

        if (!kiosk->framework.writePlayerData(playerData)) {
            Debug.println("Could not update data on tag!");
            // TODO recover 😱
            return;
        }

        if (!kiosk->display.writeIntVar(PageAddrs::Strength, playerData.strength)) {
            Debug.println("Could not set display value!");
            return;
        }
        if (!kiosk->display.writeIntVar(PageAddrs::Dexterity, playerData.dexterity)) {
            Debug.println("Could not set display value!");
            return;
        }
        if (!kiosk->display.writeIntVar(PageAddrs::Magic, playerData.magic)) {
            Debug.println("Could not set display value!");
            return;
        }

        if (playerData.bonus_points <= 0) switchPage(Page_UserMain);
    }

    bool beforeLoad() override {
        Debug.println("Loading bonus page");

        playerData = kiosk->getLastPlayerData();

        if (!kiosk->display.writeTextVar(PageAddrs::Name, kiosk->getPlayerMetadata(playerData.user_id).name)) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!kiosk->display.writeIntVar(PageAddrs::Strength, playerData.strength)) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!kiosk->display.writeIntVar(PageAddrs::Dexterity, playerData.dexterity)) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!kiosk->display.writeIntVar(PageAddrs::Magic, playerData.magic)) {
            Debug.println("Could not set display value!");
            return false;
        }

        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading bonus page");
        return true;
    }

private:
    PlayerData playerData;

};

#endif //KIOSK_BONUSPOINTSPAGE_H

#pragma clang diagnostic pop
