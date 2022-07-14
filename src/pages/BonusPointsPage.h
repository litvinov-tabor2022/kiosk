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
        Transaction transaction = Transaction{
                .time = framework->getCurrentTime(),
                .device_id = framework->getDeviceConfig().deviceId,
                .user_id = (u16) playerData.user_id,
                .bonus_points = -1
        };

        switch (addr) {
            case PageAddrs::IncStrength:
                playerData.strength++;
                playerData.bonus_points--;
                transaction.strength = 1;
                break;
            case PageAddrs::IncDexterity:
                playerData.dexterity++;
                playerData.bonus_points--;
                transaction.dexterity = 1;
                break;
            case PageAddrs::IncMagic:
                playerData.magic++;
                playerData.bonus_points--;
                transaction.magic = 1;
                break;
            default:
                // this is a failsafe so the display doesn't generate invalid transactions
                Debug.println("Invalid action!! WTF?!");
                if (!kiosk->display.beep(2000)) { Debug.println("Could not beep"); }
                return;
        }

        if (!framework->writePlayerData(playerData)) {
            Debug.println("Could not update data on tag!");
            if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
            return;
        }

        if (!framework->storage.appendTransaction(transaction)) {
            Debug.println("Could not log transaction!");
            if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
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
        if (!kiosk->display.writeIntVar(PageAddrs::Points, playerData.bonus_points)) {
            Debug.println("Could not set display value!");
            return;
        }

        if (playerData.bonus_points <= 0) switchPage(Page_UserMain);

        if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }
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

        if (!kiosk->display.writeIntVar(PageAddrs::Points, playerData.bonus_points)) {
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
