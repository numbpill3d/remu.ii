#ifndef DIGITAL_PET_STUB_H
#define DIGITAL_PET_STUB_H

#include "../../core/AppManager/BaseApp.h"

class DigitalPetApp : public BaseApp {
public:
    DigitalPetApp() {
        setMetadata("DigitalPet", "1.0", "remu.ii", "Digital pet companion", CATEGORY_GAMES, 8192);
    }
    
    bool initialize() override {
        Serial.println("[DigitalPet] Initialized");
        return true;
    }
    
    void update() override {
        // Minimal update
    }
    
    void render() override {
        displayManager.clearScreen(COLOR_BLACK);
        displayManager.setFont(FONT_LARGE);
        displayManager.drawTextCentered(0, 100, SCREEN_WIDTH, "DigitalPet", COLOR_GREEN_PHOS);
        displayManager.setFont(FONT_MEDIUM);
        displayManager.drawTextCentered(0, 130, SCREEN_WIDTH, "Coming Soon", COLOR_WHITE);
    }
    
    bool handleTouch(TouchPoint touch) override {
        if (touch.isNewPress) {
            // Exit on any touch
            return false; // Signal to return to launcher
        }
        return true;
    }
    
    String getName() const override { return "DigitalPet"; }
    
    void setAppManager(void* manager) override {
        // Store reference if needed
    }
};

#endif