#ifndef KIOSK_BONUSPOINTSPAGE_H
#define KIOSK_BONUSPOINTSPAGE_H

#include <utility>

#include "pages/DisplayPage.h"
#include "pages/PagesManager.h"
#include "PortalFramework.h"

class BonusPointsPage : public DisplayPage {
public:
    explicit BonusPointsPage(DwinDisplay *display, std::function<void(const PageId pageId)> switchPage) :
            DisplayPage(display, std::move(switchPage)) {
        strcpy(stats.secret, TagSecret.c_str());
    };

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {
        if (addr == 0x1000) {
            const u8 value = data[1];
            if (value - stats.strength != 1) {
                Serial.println("ERROR - diff != 1 for strength");
                return;
            }
            stats.strength = value;
            freePoints--;
        }
        if (addr == 0x1001) {
            const u8 value = data[1];
            if (value - stats.dexterity != 1) {
                Serial.println("ERROR - diff != 1 for dexterity");
                return;
            }
            stats.dexterity = value;
            freePoints--;
        }
        if (addr == 0x1002) {
            const u8 value = data[1];
            if (value - stats.magic != 1) {
                Serial.println("ERROR - diff != 1 for magic");
                return;
            }
            stats.magic = value;
            freePoints--;
        }

        if (freePoints <= 0) switchPage(Page_Main);
    }

    bool beforeLoad() override {
        if (!display->writeIntVar(0x1000, stats.strength)) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!display->writeIntVar(0x1001, stats.dexterity)) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!display->writeIntVar(0x1002, stats.magic)) {
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
    u8 freePoints = 5;
    PlayerData stats = PlayerData{.user_id = 999, .strength = 10, .magic = 10, .dexterity = 10};

};

#endif //KIOSK_BONUSPOINTSPAGE_H
