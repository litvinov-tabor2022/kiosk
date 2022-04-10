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
                if (!kiosk->framework.writePlayerData(adminTagData)) {
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

            case PageAddrs::Exit:
                kiosk->disableAdminMode();
                break;
        }
    }

    bool beforeLoad() override {
        playerData = kiosk->getLastPlayerData();

        // this should prevent going the "show" branch the first time (when the page is actually shown)
        // when tag is present, but it's admin tag...
        const bool showData = kiosk->framework.isTagConnected() && isLoaded;

        Debug.println("Loading admin main page");

        if (!showPlayerData(showData)) {
            Debug.println("Could not show player data!");
            if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
            return false;
        }

        isLoaded = true;
        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading admin main page");
        isLoaded = false;
        return true;
    }

private:
    bool initializeRandomTag() {
        PlayerData playerData = portal_PlayerData_init_zero;

        Serial.println("--------------------------------------------------------------------");
        Debug.println("Initializing tag to default values");

        strcpy(playerData.secret, TagSecret.c_str());
        do {
            playerData.user_id = esp_random() % 0xff; // random ID
        } while (playerData.user_id == ADMIN_USER_ID); // ensure it doesn't collide

        playerData.strength = 10;
        playerData.magic = 10;
        playerData.dexterity = 10;
        playerData.bonus_points = 5;

        if (!kiosk->framework.writePlayerData(playerData)) {
            Debug.println("Can't initialize the tag!");
            return false;
        }

        return true;
    }

    bool addBonusPoint() {
        auto pdata = kiosk->getLastPlayerData();
        pdata.bonus_points++;

        //TODO create transaction

        return kiosk->framework.writePlayerData(pdata);
    }

    bool showPlayerData(bool show) {
        if (show) {
            Debug.println("Showing user's data");

            //TODO show real name
            if (!kiosk->display.writeTextVar(PageAddrs::Name, u8"Jenda")) {
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
            Debug.println("Hiding user's data");

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

    bool adjustPlayerData(const u16 addr) {
        //TODO create transaction

        switch (addr) {
            case PageAddrs::IncStrength:
                playerData.strength++;
                break;
            case PageAddrs::DecStrength:
                playerData.strength--;
                break;
            case PageAddrs::IncDexterity:
                playerData.dexterity++;
                break;
            case PageAddrs::DecDexterity:
                playerData.dexterity--;
                break;
            case PageAddrs::IncMagic:
                playerData.magic++;
                break;
            case PageAddrs::DecMagic:
                playerData.magic--;
                break;
        }

        if (!kiosk->framework.writePlayerData(playerData)) {
            Debug.println("Could not write the data!");
            if (!kiosk->display.beep(1000)) { Debug.println("Could not beep"); }
            return false;
        }
        if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }

        return true;
    }

    bool isLoaded = false;

    PlayerData playerData;
    PlayerData adminTagData = PlayerData{.user_id = ADMIN_USER_ID};
};


#endif //KIOSK_ADMINMAINPAGE_H
