#ifndef KIOSK_DISPLAYPAGE_H
#define KIOSK_DISPLAYPAGE_H

#include <functional>
#include <utility>

#include "types.h"
#include "../hw/DwinDisplay.h"
#include "PortalFramework.h"
#include "pages/PagesManager.h"
#include "pages/addrs.h"
#include "Kiosk.h"


class DisplayPage {
public:
    DisplayPage(Kiosk *kiosk, DwinDisplay *display, PortalFramework *framework, std::function<void(const PageId pageId)> switchPage)
            : switchPage(std::move(switchPage)), display(display), framework(framework), kiosk(kiosk) {};

    virtual void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) = 0;

    virtual bool beforeLoad() = 0;

    virtual bool beforeUnload() = 0;

protected:
    std::function<void(const PageId pageId)> switchPage;

    Kiosk *kiosk;
    DwinDisplay *display;
    PortalFramework *framework;
};

#endif //KIOSK_DISPLAYPAGE_H
