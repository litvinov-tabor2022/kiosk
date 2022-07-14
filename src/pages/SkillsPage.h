#ifndef KIOSK_SKILLSPAGE_H
#define KIOSK_SKILLSPAGE_H


#include <utility>

#include "pages/DisplayPage.h"
#include "PortalFramework.h"
#include "Resources.h"
#include "PlayerDataUtils.h"

#define SKILLS_PAGE_COLS 1
#define SKILLS_PAGE_ROWS 5
#define SKILLS_PAGE_SIZE (SKILLS_PAGE_ROWS * SKILLS_PAGE_COLS)


#define PageAddrs Addrs::User::Skills

class SkillsPage : public DisplayPage {
public:
    explicit SkillsPage(Kiosk *kiosk, std::function<void(const PageId pageId)> switchPage) :
            DisplayPage(kiosk, std::move(switchPage)) {};

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {
        switch (addr) {
            case PageAddrs::BackButton:
                if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }
                switchPage(Page_UserMain);
                break;
            case PageAddrs::PrevButton:
                if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }
                if (pageNo == 0) return;

                pageNo--;
                if (pageNo < 0) pageNo = 0;
                beforeLoad();
                break;
            case PageAddrs::NextButton:
                if (!kiosk->display.beep(100)) { Debug.println("Could not beep"); }
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

        playerData = kiosk->getLastPlayerData();

        const auto it = kiosk->skillsList->getAll();

        std::vector<SkillsListEntry> skillsList;

        for (u8 i = 0; i < kiosk->skillsList->getLength(); i++) {
            const auto elem = it + i;
            if (playerHasSkill(elem->skill)) skillsList.push_back(*elem);
        }

        ownedSkillsList = new SkillsList(skillsList);
        maxPageNo = ownedSkillsList->getLength() / SKILLS_PAGE_SIZE;

        if (!displaySkillsPage()) {
            Debug.println("Could not display skills!");
            return false;
        }

        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading skills page");
        free(ownedSkillsList);
        return true;
    }

private:
    bool displaySkillsPage() {
        const auto it = ownedSkillsList->getSkillsPageStart(pageNo, SKILLS_PAGE_SIZE);
        const u8 count = ownedSkillsList->getLength();

        Debug.printf("Showing %d/%d user's skills\n", SKILLS_PAGE_SIZE, count);

        for (u8 row = 0; row < SKILLS_PAGE_ROWS; row++)
            for (u8 col = 0; col < SKILLS_PAGE_COLS; col++) {
                const u16 vpAddr = PageAddrs::VpAddrBase + row * 0x0100 + col * 0x0030;

                const u8 offset = row * SKILLS_PAGE_COLS + col;

                if (offset >= count) {
                    // for cases there's not enough skills to fill a single page...
                    // we need to empty all the unused fields then
                    if (!kiosk->display.writeTextVar(vpAddr, "")) return false;
                    continue;
                }

                const auto elem = it + offset;

                Serial.printf("Showing skill: %s \n", elem->name.c_str());

                if (!kiosk->display.writeTextVar(vpAddr, elem->name)) return false;
            }

        return true;
    }

    [[nodiscard]] bool playerHasSkill(const u16 id) const {
        return PlayerDataUtils::hasSkill(id, playerData);
    }

    SkillsList *ownedSkillsList;
    PlayerData playerData;
    u8 maxPageNo;
    i8 pageNo = 0; // yes, i8... it's more convenient for handling the overflow
};


#endif //KIOSK_SKILLSPAGE_H
