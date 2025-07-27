#ifndef FREQ_SCANNER_STUB_H
#define FREQ_SCANNER_STUB_H

#include "../../core/AppManager/BaseApp.h"

class FreqScannerApp : public BaseApp {
public:
    FreqScannerApp() {
        setMetadata("FreqScanner", "1.0", "remu.ii", "Frequency scanner", CATEGORY_TOOLS, 7168);
    }
    
    bool initialize() override {
        Serial.println("[FreqScanner] Initialized");
        return true;
    }
    
    void update() override {}
    
    void render() override {
        displayManager.clearScreen(COLOR_BLACK);
        displayManager.setFont(FONT_LARGE);
        displayManager.drawTextCentered(0, 100, SCREEN_WIDTH, "FreqScanner", COLOR_GREEN_PHOS);
        displayManager.setFont(FONT_MEDIUM);
        displayManager.drawTextCentered(0, 130, SCREEN_WIDTH, "Coming Soon", COLOR_WHITE);
    }
    
    bool handleTouch(TouchPoint touch) override {
        if (touch.isNewPress) return false;
        return true;
    }
    
    String getName() const override { return "FreqScanner"; }
    void setAppManager(void* manager) override {}
};

#endif