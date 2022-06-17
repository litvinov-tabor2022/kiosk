#ifndef KIOSK_HOMEPAGE_H
#define KIOSK_HOMEPAGE_H

#include <utility>

#include "pages/DisplayPage.h"
#include "debugging.h"

class HomePage : public DisplayPage {
public:
    explicit HomePage(Kiosk *kiosk, std::function<void(const PageId pageId)> switchPage) : DisplayPage(kiosk, std::move(switchPage)) {};

    void handleAsyncDisplayData(const u16 addr, const u8 *data, const u8 dataLen) override {
        if (addr == Addrs::Home::Touch) {
            if (shouldDim) return; // already planned dim?

            if (!kiosk->display.setBrightness(255)) {
                Debug.println("Could not set brightness");
                return;
            }

            shouldDim = true;

            Core0.once("dim", [this] {
                Tasker::sleep(2000);
                if (shouldDim) {
                    shouldDim = false;
                    if (!kiosk->display.setBrightness(0)) {
                        Debug.println("Could not set brightness");
                    }
                }
            });
        }
    }

    bool beforeLoad() override {
        Debug.println("Loading home page\n--------------------------------------------------------");

        return true;
    }

    bool beforeUnload() override {
        Debug.println("Unloading home page");
        shouldDim = false;
        return true;
    }

private:
    bool shouldDim = false;
};

#endif //KIOSK_HOMEPAGE_H
