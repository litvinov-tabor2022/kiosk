#ifndef KIOSK_KIOSK_H
#define KIOSK_KIOSK_H

#include <hw/DwinDisplay.h>
#include <pages/PagesManager.h>
#include <PortalFramework.h>

class Kiosk {
public:
    bool begin();

private:
    PortalFramework framework;
    DwinDisplay display;
    PagesManager pagesManager = PagesManager(&display);
};


#endif //KIOSK_KIOSK_H
