#ifndef KIOSK_DISPLAYPAGE_H
#define KIOSK_DISPLAYPAGE_H

#include <functional>
#include <utility>

#include "types.h"
#include "pages/PagesManager.h"
#include "../hw/DwinDisplay.h"

class DisplayPage {
public:
    DisplayPage(DwinDisplay *display, std::function<void(const PageId pageId)> switchPage)
            : switchPage(std::move(switchPage)), display(display) {};

    virtual void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) = 0;

    virtual bool beforeLoad() = 0;

    virtual bool beforeUnload() = 0;

protected:
    std::function<void(const PageId pageId)> switchPage;
    DwinDisplay *display;
};

#endif //KIOSK_DISPLAYPAGE_H
