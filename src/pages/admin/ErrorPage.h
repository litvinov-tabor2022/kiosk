#ifndef KIOSK_ADMINERRORPAGE_H
#define KIOSK_ADMINERRORPAGE_H

#include <utility>

#include "pages/DisplayPage.h"
#include "debugging.h"

class ErrorPage : public DisplayPage {
public:
    explicit ErrorPage(Kiosk *kiosk, std::function<void(const PageId pageId)> switchPage) : DisplayPage(kiosk, std::move(switchPage)) {};

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {}

    bool beforeLoad() override {
        Debug.println("Loading error page\n--------------------------------------------------------");

        // the error message should be already set
        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading error page");
        return true;
    }
};

#endif //KIOSK_ADMINERRORPAGE_H
