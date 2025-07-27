#ifndef BLE_SCANNER_STUB_H
#define BLE_SCANNER_STUB_H

#include "../../core/AppManager/BaseApp.h"

class BLEScannerApp : public BaseApp {
public:
    BLEScannerApp() {
        setMetadata("BLEScanner", "1.0", "remu.ii", "Bluetooth LE scanner", CATEGORY_TOOLS, 9216);
    }
    
    bool initialize() override {
        Serial.println("[BLEScanner] Initialized");
        return true;
    }
    
    void update() override {}
    
    void render() override {
        displayManager.clearScreen(COLOR_BLACK);
        displayManager.setFont(FONT_LARGE);
        displayManager.drawTextCentered(0, 100, SCREEN_WIDTH, "BLEScanner", COLOR_GREEN_PHOS);
        displayManager.setFont(FONT_MEDIUM);
        displayManager.drawTextCentered(0, 130, SCREEN_WIDTH, "Coming Soon", COLOR_WHITE);
    }
    
    bool handleTouch(TouchPoint touch) override {
        if (touch.isNewPress) return false;
        return true;
    }
    
    String getName() const override { return "BLEScanner"; }
    void setAppManager(void* manager) override {}
};

#endif