#ifndef KIOSK_ADMINMAINPAGE_H
#define KIOSK_ADMINMAINPAGE_H

#include <utility>

#include "pages/DisplayPage.h"
#include "debugging.h"

#define PageAddrs Addrs::Admin::Main

class AdminMainPage : public DisplayPage {

public:
    explicit AdminMainPage(Kiosk *kiosk, std::function<void(const PageId pageId)> switchPage) :
            DisplayPage(kiosk, std::move(switchPage)) {
        strcpy(adminTagData.secret, TagSecret.c_str());
    };

    // TODO replicate the last tag
    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {
        if (addr >= PageAddrs::IncStrength && addr <= PageAddrs::DecMagic) {
            if (!adjustPlayerData(addr)) {
                Debug.println("Could not write the data!");
                if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
                return;
            }
            if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }

            return;
        }
        // else

        switch (addr) {
            case PageAddrs::InitAdmin:
                if (!framework->writePlayerData(adminTagData)) {
                    Debug.println("Could not initialize admin tag!");
                    if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
                    return;
                }
                if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }
                Debug.println("Admin tag initialized!");
                break;

            case PageAddrs::InitRandom:
                if (!initializeRandomTag()) {
                    Debug.println("Could not initialize random user tag!");
                    if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
                    return;
                }
                if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }
                Debug.println("User tag initialized!");
                break;

            case PageAddrs::AddBonusPoint:
                if (!addBonusPoint()) {
                    Debug.println("Could not write the data!");
                    if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
                    return;
                }
                if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }
                break;

            case PageAddrs::Skills:
                if (!checkUserTagPresent("show skills page")) return;

                switchPage(Page_Admin_Skills);
                break;

            case PageAddrs::ServiceMode:
                if (!framework->synchronizationMode.toggle()) {
                    Debug.println("Could not toggle synchronization mode!");
                    return;
                }

                break;

            case PageAddrs::Exit:
                kiosk->disableAdminMode();
                break;

            default:
                Debug.printf("Unknown command!!! 0x%x\n", addr);
                return;
        }
    }

    bool beforeLoad() override {
        const bool showData = framework->isTagConnected() && !kiosk->isAdminTagPresent();

        Debug.printf("Loading admin main page, will%s show data\n", showData ? "" : " NOT");
        playerData = kiosk->getLastPlayerData();

        if (!showPlayerData(showData)) {
            Debug.println("Could not show player data!");
            if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
            return false;
        }

        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading admin main page");
        return true;
    }

private:
    bool initializeRandomTag() {
        PlayerData newPlayerData = portal_PlayerData_init_zero;

        Serial.println("--------------------------------------------------------------------");
        Debug.println("Initializing tag to default values");

        strcpy(newPlayerData.secret, TagSecret.c_str());
        do {
            newPlayerData.user_id = esp_random() % 0xff; // random ID
        } while (newPlayerData.user_id == ADMIN_USER_ID); // ensure it doesn't collide

        newPlayerData.strength = 10;
        newPlayerData.magic = 10;
        newPlayerData.dexterity = 10;
        newPlayerData.bonus_points = 5;

        if (!framework->writePlayerData(newPlayerData)) {
            Debug.println("Can't initialize the tag!");
            return false;
        }

        return true;
    }

    bool showPlayerData(bool show) {
        if (show) {
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
        } else {
            if (!kiosk->display.writeTextVar(PageAddrs::Name, u8"-- none --")) {
                Debug.println("Could not set display value!");
                return false;
            }

            if (!kiosk->display.writeIntVar(PageAddrs::Strength, 0)) {
                Debug.println("Could not set display value!");
                return false;
            }

            if (!kiosk->display.writeIntVar(PageAddrs::Dexterity, 0)) {
                Debug.println("Could not set display value!");
                return false;
            }

            if (!kiosk->display.writeIntVar(PageAddrs::Magic, 0)) {
                Debug.println("Could not set display value!");
                return false;
            }
        }

        return true;
    }

    bool addBonusPoint() {
        if (!checkUserTagPresent("add user's bonus point")) return false;

        playerData.bonus_points++;

        const Transaction transaction = Transaction{
                .time = framework->clocks.getCurrentTime(),
                .device_id = framework->getDeviceConfig().deviceId,
                .user_id = (u16) playerData.user_id,
                .bonus_points = 1
        };

        return writeToTagAndCommit(transaction);
    }

    bool adjustPlayerData(const u16 addr) {
        if (!checkUserTagPresent("adjust user's data")) return false;

        Transaction transaction = Transaction{
                .time = framework->clocks.getCurrentTime(),
                .device_id = framework->getDeviceConfig().deviceId,
                .user_id = (u16) playerData.user_id,
        };

        switch (addr) {
            case PageAddrs::IncStrength:
                playerData.strength++;
                transaction.strength = 1;
                break;
            case PageAddrs::DecStrength:
                playerData.strength--;
                transaction.strength = -1;
                break;
            case PageAddrs::IncDexterity:
                playerData.dexterity++;
                transaction.dexterity = 1;
                break;
            case PageAddrs::DecDexterity:
                playerData.dexterity--;
                transaction.dexterity = -1;
                break;
            case PageAddrs::IncMagic:
                playerData.magic++;
                transaction.magic = 1;
                break;
            case PageAddrs::DecMagic:
                playerData.magic--;
                transaction.magic = -1;
                break;

            default:
                Debug.printf("Unknown command!!! 0x%x\n", addr);
                return false;
        }

        return writeToTagAndCommit(transaction);
    }

    bool checkUserTagPresent(const String &desc) {
        if (kiosk->isAdminTagPresent()) {
            Debug.printf("Can't %s - admin tag present!\n", desc.c_str());
            if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
            return false;
        }

        if (!framework->isTagConnected()) {
            Debug.printf("Can't %s - no tag present\n", desc.c_str());
            if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
            return false;
        }

        return true;
    }

    bool writeToTagAndCommit(const Transaction &transaction) {
        if (!framework->writePlayerData(playerData)) {
            Debug.println("Could not write the data!");
            if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
            return false;
        }

        if (!framework->storage.appendTransaction(transaction)) {
            Debug.println("Could not log transaction!");
            return false;
        }

        if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }

        return true;
    }

    PlayerData playerData{};
    PlayerData adminTagData = PlayerData{.user_id = ADMIN_USER_ID};
};


#endif //KIOSK_ADMINMAINPAGE_H
