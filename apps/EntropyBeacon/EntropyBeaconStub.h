#ifndef ENTROPY_BEACON_STUB_H
#define ENTROPY_BEACON_STUB_H

#include "../../core/AppManager/BaseApp.h"

class EntropyBeaconApp : public BaseApp {
public:
    EntropyBeaconApp() {
        setMetadata("EntropyBeacon", "1.0", "remu.ii", "Entropy beacon", CATEGORY_OTHER, 6144);
    }
    
    bool initialize() override {
        Serial.println("[EntropyBeacon] Initialized");
        return true;
    }
    
    void update() override {}
    
    void render() override {
        displayManager.clearScreen(COLOR_BLACK);
        displayManager.setFont(FONT_LARGE);
        displayManager.drawTextCentered(0, 100, SCREEN_WIDTH, "EntropyBeacon", COLOR_GREEN_PHOS);
        displayManager.setFont(FONT_MEDIUM);
        displayManager.drawTextCentered(0, 130, SCREEN_WIDTH, "Coming Soon", COLOR_WHITE);
    }
    
    bool handleTouch(TouchPoint touch) override {
        if (touch.isNewPress) return false;
        return true;
    }
    
    String getName() const override { return "EntropyBeacon"; }
    void setAppManager(void* manager) override {}
};

#endif