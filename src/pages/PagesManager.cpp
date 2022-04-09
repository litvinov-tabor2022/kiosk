#include "PagesManager.h"
#include "debugging.h"

bool PagesManager::switchPage(PageId pageId) {
    std::lock_guard<std::mutex> lg(switchPageMutex);

    DisplayPage *newPage;

    switch (pageId) {
        case Page_Home:
            newPage = &homePage;
            break;
        case Page_UserMain:
            newPage = &mainPage;
            break;
        case Page_BonusPoints:
            newPage = &bonusPointsPage;
            break;
            // ------------
            // admin pages:
        case Page_Admin_Main:
            newPage = &adminMainPage;
            break;
    }

    const bool isChange = currentPage != newPage;

    if (isChange) {
        if (currentPage != nullptr && !currentPage->beforeUnload()) {
            Debug.println("Could not unload old page!");
            return false;
        }

        currentPage = newPage;
    } else {
        Debug.println("Not changing the page; just reloading");
    }

    if (!currentPage->beforeLoad()) {
        Debug.println("Could not load new page!");
        return false;
    }

    return !isChange || display->setPage(u8(pageId));
}

void PagesManager::handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) {
    currentPage->handleAsyncDisplayData(addr, data, dataLen);
}

PagesManager::PagesManager(Kiosk *kiosk, DwinDisplay *display, PortalFramework *framework) :
        kiosk(kiosk), display(display), framework(framework) {}

bool PagesManager::begin() {
    currentPage = &homePage;
    if (!homePage.beforeLoad()) {
        Debug.println("Could not load homepage!");
        return false;
    }

    return display->setPage(u8(Page_Home));
}

bool PagesManager::reloadPage() {
    Debug.println("Reloading page");
    return currentPage->beforeLoad();
}
