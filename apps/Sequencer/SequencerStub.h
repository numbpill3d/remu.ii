#ifndef SEQUENCER_STUB_H
#define SEQUENCER_STUB_H

#include "../../core/AppManager/BaseApp.h"

class SequencerApp : public BaseApp {
public:
    SequencerApp() {
        setMetadata("Sequencer", "1.0", "remu.ii", "Music sequencer", CATEGORY_MEDIA, 12288);
    }
    
    bool initialize() override {
        Serial.println("[Sequencer] Initialized");
        return true;
    }
    
    void update() override {}
    
    void render() override {
        displayManager.clearScreen(COLOR_BLACK);
        displayManager.setFont(FONT_LARGE);
        displayManager.drawTextCentered(0, 100, SCREEN_WIDTH, "Sequencer", COLOR_GREEN_PHOS);
        displayManager.setFont(FONT_MEDIUM);
        displayManager.drawTextCentered(0, 130, SCREEN_WIDTH, "Coming Soon", COLOR_WHITE);
    }
    
    bool handleTouch(TouchPoint touch) override {
        if (touch.isNewPress) return false;
        return true;
    }
    
    String getName() const override { return "Sequencer"; }
    void setAppManager(void* manager) override {}
};

#endif