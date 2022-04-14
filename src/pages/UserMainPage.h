#ifndef KIOSK_USERMAINPAGE_H
#define KIOSK_USERMAINPAGE_H

#include <utility>

#include "pages/DisplayPage.h"
#include "PortalFramework.h"

#define PageAddrs Addrs::User::Main

class UserMainPage : public DisplayPage {
public:
    explicit UserMainPage(Kiosk *kiosk, std::function<void(const PageId pageId)> switchPage) : DisplayPage(kiosk, std::move(switchPage)) {};

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {
        if (addr == PageAddrs::SkillsButton) {
            switchPage(Page_UserSkills);
        }
    }

    bool beforeLoad() override {
        Debug.println("Loading user main page");

        playerData = kiosk->getLastPlayerData();

        if (!kiosk->display.writeTextVar(PageAddrs::Name, kiosk->framework.resources.getPlayerMetadata(playerData.user_id).name) ){
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
        Debug.println("Unloading main page");
        return true;
    }

private:
    PlayerData playerData;

};

#endif //KIOSK_USERMAINPAGE_H
