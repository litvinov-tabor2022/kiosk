#ifndef KIOSK_PAGESMANAGER_H
#define KIOSK_PAGESMANAGER_H

enum PageId {
    Page_Home = 4,
    Page_UserMain = 0,
    Page_BonusPoints = 1,
    Page_Admin_Main = 3,
};

#include <mutex>
#include "hw/DwinDisplay.h"

// pages:
#include "pages/UserMainPage.h"
#include "pages/HomePage.h"
#include "pages/BonusPointsPage.h"
#include "pages/admin/AdminMainPage.h"
// ------

class PagesManager {
public:
    PagesManager(Kiosk *kiosk, DwinDisplay *display, PortalFramework *framework);

    bool begin();

    bool switchPage(PageId pageId);
    bool reloadPage();

    void handleAsyncDisplayData(u16 addr, const u8 *data, u8 dataLen);

private:
    Kiosk *kiosk;
    DwinDisplay *display;
    PortalFramework *framework;

    DisplayPage *currentPage;
    std::mutex switchPageMutex;

    // pages:
    HomePage homePage = HomePage(kiosk, display, framework, [this](const PageId pageId) { this->switchPage(pageId); });
    UserMainPage mainPage = UserMainPage(kiosk, display, framework, [this](const PageId pageId) { this->switchPage(pageId); });
    BonusPointsPage bonusPointsPage = BonusPointsPage(kiosk, display, framework, [this](const PageId pageId) { this->switchPage(pageId); });
    AdminMainPage adminMainPage = AdminMainPage(kiosk, display, framework, [this](const PageId pageId) { this->switchPage(pageId); });
};

#endif //KIOSK_PAGESMANAGER_H
