#ifndef KIOSK_SKILLSPAGE_H
#define KIOSK_SKILLSPAGE_H


#include <utility>

#include "pages/DisplayPage.h"
#include "PortalFramework.h"

#define SKILLS_PAGE_COLS 3
#define SKILLS_PAGE_ROWS 5
#define SKILLS_PAGE_SIZE SKILLS_PAGE_ROWS * SKILLS_PAGE_COLS


#define PageAddrs Addrs::User::Skills

class SkillsPage : public DisplayPage {
public:
    explicit SkillsPage(Kiosk *kiosk, std::function<void(const PageId pageId)> switchPage) :
            DisplayPage(kiosk, std::move(switchPage)) {};

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {
        switch (addr) {
            case PageAddrs::BackButton:
                switchPage(Page_UserMain);
                break;
            case PageAddrs::PrevButton:
                pageNo--;
                if (pageNo < 0) pageNo = 0;
                beforeLoad();
                break;
            case PageAddrs::NextButton:
                pageNo++;
                //TODO don't allow this to happen!!!
                beforeLoad();
                break;
        }
    }

    bool beforeLoad() override {
        Debug.println("Loading skills page");

        playerData = kiosk->getLastPlayerData();

        if (!kiosk->display.writeTextVar(PageAddrs::Name, kiosk->framework.resources.getPlayerMetadata(playerData.user_id).name)) {
            Debug.println("Could not set display value!");
            return false;
        }

        if (!displaySkillsPage()) {
            Debug.println("Could not display skills!");
            return false;
        }

        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading skills page");
        return true;
    }

private:
    bool displaySkillsPage() {
        const auto skills = kiosk->framework.resources.getPriceListPage(pageNo, SKILLS_PAGE_SIZE);
        const auto it = skills.cbegin();

        for (u8 row = 0; row < SKILLS_PAGE_ROWS; row++)
            for (u8 col = 0; col < SKILLS_PAGE_COLS; col++) {
                const u16 addr = PageAddrs::SkillBase + row * 0x0100 + col * 0x0030;

                const auto elem = it + row * SKILLS_PAGE_COLS + col;

//                Serial.printf("Col %d row %d, addr 0x%x, code %s name %s\n", col, row, addr, elem->code.c_str(), elem->name.c_str());

                if (!kiosk->display.writeTextVar(addr, elem->name)) return false;
            }

        return true;
    }

    PlayerData playerData;
    i8 pageNo = 0; // yes, i8... it's more convenient for handling the overflow
};


#endif //KIOSK_SKILLSPAGE_H
