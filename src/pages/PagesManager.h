#ifndef KIOSK_PAGESMANAGER_H
#define KIOSK_PAGESMANAGER_H

enum PageId {
    Page_Main = 0,
    Page_BonusPoints = 1,
};

#include "pages/MainPage.h"
#include "pages/BonusPointsPage.h"
#include "hw/DwinDisplay.h"

#include <mutex>


class PagesManager {
public:
    PagesManager(DwinDisplay *display);

    bool begin(PageId pageId);

    bool switchPage(PageId pageId);

    void handleAsyncDisplayData(u16 addr, const u8 *data, u8 dataLen);

private:
    DwinDisplay *display;
    MainPage mainPage = MainPage(display, [this](const PageId pageId) { this->switchPage(pageId); });
    BonusPointsPage bonusPointsPage = BonusPointsPage(display, [this](const PageId pageId) { this->switchPage(pageId); });

    DisplayPage *currentPage;
    std::mutex switchPageMutex;
};

#endif //KIOSK_PAGESMANAGER_H
