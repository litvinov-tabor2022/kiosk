#ifndef KIOSK_MAINPAGE_H
#define KIOSK_MAINPAGE_H

#include <utility>

#include "pages/DisplayPage.h"
#include "pages/PagesManager.h"
#include "debugging.h"

class MainPage : public DisplayPage {
public:
    explicit MainPage(DwinDisplay *display, std::function<void(const PageId pageId)> switchPage) :
            DisplayPage(display, std::move(switchPage)) {};

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {

    }

    bool beforeLoad() override {
        Debug.println("Loading main page");
        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading main page");
        return true;
    }

private:

};

#endif //KIOSK_MAINPAGE_H
