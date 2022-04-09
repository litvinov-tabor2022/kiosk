#ifndef KIOSK_USERMAINPAGE_H
#define KIOSK_USERMAINPAGE_H

#include <utility>

#include "pages/DisplayPage.h"
#include "PortalFramework.h"

#define PageAddrs Addrs::User::Main

class UserMainPage : public DisplayPage {
public:
    explicit UserMainPage(Kiosk *kiosk, DwinDisplay *display, PortalFramework *framework,
                          std::function<void(const PageId pageId)> switchPage) :
            DisplayPage(kiosk, display, framework, std::move(switchPage)) {
    };

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {
        if (addr == PageAddrs::Strength) {
            const u8 value = data[1];
            if (value - playerData.strength != 1) {
                Serial.println("ERROR - diff != 1 for strength");
                return;
            }
            playerData.strength = value;
            freePoints--;
        }
        if (addr == PageAddrs::Dexterity) {
            const u8 value = data[1];
            if (value - playerData.dexterity != 1) {
                Serial.println("ERROR - diff != 1 for dexterity");
                return;
            }
            playerData.dexterity = value;
            freePoints--;
        }
        if (addr == PageAddrs::Magic) {
            const u8 value = data[1];
            if (value - playerData.magic != 1) {
                Serial.println("ERROR - diff != 1 for magic");
                return;
            }
            playerData.magic = value;
            freePoints--;
        }

        if (freePoints <= 0) switchPage(Page_UserMain);
    }

    bool beforeLoad() override {
        Debug.println("Loading user main page");

        playerData = kiosk->getLastPlayerData();

        // TODO show real name
        if (!display->writeTextVar(PageAddrs::Name, u8"Jenda")) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!display->writeIntVar(PageAddrs::Strength, playerData.strength)) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!display->writeIntVar(PageAddrs::Dexterity, playerData.dexterity)) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!display->writeIntVar(PageAddrs::Magic, playerData.magic)) {
            Debug.println("Could not set display value!");
            return false;
        }

        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading main page");
        return true;
    }

private:
    u8 freePoints = 5;
    PlayerData playerData;

};

#endif //KIOSK_BONUSPOINTSPAGE_H
