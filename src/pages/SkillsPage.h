#ifndef KIOSK_SKILLSPAGE_H
#define KIOSK_SKILLSPAGE_H


#include <utility>

#include "pages/DisplayPage.h"
#include "PortalFramework.h"
#include "Resources.h"
#include "PlayerDataUtils.h"

#define SKILLS_PAGE_COLS 1
#define SKILLS_PAGE_ROWS 4
#define SKILLS_PAGE_SIZE (SKILLS_PAGE_ROWS * SKILLS_PAGE_COLS)


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
                if (pageNo == 0) return;

                pageNo--;
                if (pageNo < 0) pageNo = 0;
                beforeLoad();
                break;
            case PageAddrs::NextButton:
                if (pageNo == maxPageNo) return;

                pageNo++;
                Serial.printf("New page %d, max %d\n", pageNo, maxPageNo);
                if (pageNo >= maxPageNo) pageNo = (i8) maxPageNo;
                beforeLoad();
                break;
        }
    }

    bool beforeLoad() override {
        Debug.printf("Loading skills page, page %d\n", pageNo);

        // this is not changed so it doesn't have to be reloaded every time, but it can't be loaded in ctor
        maxPageNo = kiosk->skillsList->getLength() / SKILLS_PAGE_SIZE;

        playerData = kiosk->getLastPlayerData();

        if (!kiosk->display.writeTextVar(PageAddrs::Name, kiosk->getPlayerMetadata(playerData.user_id).name)) {
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
        const auto it = kiosk->skillsList->getSkillsPageStart(pageNo, SKILLS_PAGE_SIZE);

        for (u8 row = 0; row < SKILLS_PAGE_ROWS; row++)
            for (u8 col = 0; col < SKILLS_PAGE_COLS; col++) {
                const u16 vpAddr = PageAddrs::VpAddrBase + row * 0x0100 + col * 0x0030;
                const u16 spAddr = PageAddrs::SpAddrBase + row * 0x0100 + col * 0x0030;

                const auto elem = it + row * SKILLS_PAGE_COLS + col;

                if (!kiosk->display.writeTextVar(vpAddr, elem->name)) return false;

                // color: can the player have this?
                const auto color = playerHasSkill(elem->skill) ? Colors::LightGreen : (playerCanHave(*elem) ? Colors::Black : Colors::Red);
                if (!kiosk->display.setTextDisplayColor(spAddr, color)) return false;
            }

        return true;
    }

    [[nodiscard]] bool playerHasSkill(const u16 id) const {
        return PlayerDataUtils::hasSkill(id, playerData);
    }

    [[nodiscard]] bool playerCanHave(const SkillsListEntry &skill) const {
        return PlayerDataUtils::canHaveSkill(skill, playerData);
    }

    PlayerData playerData;
    u8 maxPageNo;
    i8 pageNo = 0; // yes, i8... it's more convenient for handling the overflow
};


#endif //KIOSK_SKILLSPAGE_H
