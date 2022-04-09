#include "PagesManager.h"
#include "debugging.h"

bool PagesManager::switchPage(PageId pageId) {
    std::lock_guard<std::mutex> lg(switchPageMutex);

    if (currentPage != nullptr && !currentPage->beforeUnload()) {
        Debug.println("Could not unload current page!");
        return false;
    }

    switch (pageId) {
        case Page_Main:
            currentPage = &mainPage;
            break;
        case Page_BonusPoints:
            currentPage = &bonusPointsPage;
            break;
    }

    if (!currentPage->beforeLoad()) {
        Debug.println("Could not unload current page!");
        return false;
    }

    return display->setPage(u8(pageId));
}

void PagesManager::handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) {
    currentPage->handleAsyncDisplayData(addr, data, dataLen);
}

PagesManager::PagesManager(DwinDisplay *display) : display(display) {}

bool PagesManager::begin(PageId pageId) {
    return switchPage(pageId);
}
