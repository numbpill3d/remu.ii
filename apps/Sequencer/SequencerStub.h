#ifndef SEQUENCER_STUB_H
#define SEQUENCER_STUB_H

#include "../../core/AppManager/BaseApp.h"
#include "../../core/FileSystem.h"

struct SequencerPattern {
    bool steps[16][8];
    int bpm = 120;
    String name = "Pattern1";
};

class SequencerApp : public BaseApp {
private:
    SequencerPattern pattern;
    String saveFile = "/data/sequencer.json";
    int selectedStep = 0;
    int selectedTrack = 0;
    bool isPlaying = false;
    unsigned long lastStepTime = 0;
    int currentStep = 0;
    
    void loadPattern() {
        if (filesystem.fileExists(saveFile)) {
            String data = filesystem.readFile(saveFile);
            if (data.length() > 0) {
                // Simple parsing - just load BPM for now
                int bpmStart = data.indexOf("bpm") + 5;
                if (bpmStart > 4) {
                    pattern.bpm = data.substring(bpmStart, bpmStart + 3).toInt();
                }
            }
        }
    }
    
    void savePattern() {
        String data = "{\"bpm\":" + String(pattern.bpm) + ",\"name\":\"" + pattern.name + "\"}";
        filesystem.ensureDirExists("/data");
        filesystem.writeFile(saveFile, data);
    }
    
public:
    SequencerApp() {
        setMetadata("Sequencer", "1.0", "remu.ii", "16-step sequencer", CATEGORY_MEDIA, 12288);
        for (int s = 0; s < 16; s++) {
            for (int t = 0; t < 8; t++) {
                pattern.steps[s][t] = false;
            }
        }
    }
    
    bool initialize() override {
        Serial.println("[Sequencer] Initializing with SD storage...");
        loadPattern();
        return true;
    }
    
    void update() override {
        if (isPlaying) {
            unsigned long stepInterval = 60000 / (pattern.bpm * 4);
            if (millis() - lastStepTime > stepInterval) {
                currentStep = (currentStep + 1) % 16;
                lastStepTime = millis();
            }
        }
    }
    
    void render() override {
        displayManager.clearScreen(COLOR_BLACK);
        
        displayManager.setFont(FONT_MEDIUM);
        displayManager.drawText(10, 10, pattern.name, COLOR_GREEN_PHOS);
        displayManager.drawText(200, 10, "BPM:" + String(pattern.bpm), COLOR_WHITE);
        
        displayManager.drawText(10, 30, isPlaying ? "PLAYING" : "STOPPED", 
                               isPlaying ? COLOR_GREEN : COLOR_RED);
        
        // Step grid
        int gridX = 20, gridY = 60;
        int stepW = 16, stepH = 16;
        
        for (int s = 0; s < 16; s++) {
            for (int t = 0; t < 8; t++) {
                int x = gridX + s * stepW;
                int y = gridY + t * stepH;
                
                uint16_t color = COLOR_DARK_GRAY;
                if (pattern.steps[s][t]) color = COLOR_GREEN_PHOS;
                if (s == selectedStep && t == selectedTrack) color = COLOR_RED_GLOW;
                if (s == currentStep && isPlaying) color = COLOR_YELLOW;
                
                displayManager.drawRetroRect(x, y, stepW-1, stepH-1, color, pattern.steps[s][t]);
            }
        }
        
        displayManager.setFont(FONT_SMALL);
        displayManager.drawText(20, 200, "PLAY", COLOR_WHITE);
        displayManager.drawText(80, 200, "CLEAR", COLOR_WHITE);
        displayManager.drawText(140, 200, "SAVE", COLOR_WHITE);
        displayManager.drawText(200, 200, "EXIT", COLOR_WHITE);
    }
    
    bool handleTouch(TouchPoint touch) override {
        if (touch.isNewPress) {
            if (touch.y >= 60 && touch.y < 188) {
                int step = (touch.x - 20) / 16;
                int track = (touch.y - 60) / 16;
                if (step >= 0 && step < 16 && track >= 0 && track < 8) {
                    selectedStep = step;
                    selectedTrack = track;
                    pattern.steps[step][track] = !pattern.steps[step][track];
                    savePattern();
                }
            }
            else if (touch.y >= 190) {
                if (touch.x < 60) {
                    isPlaying = !isPlaying;
                    if (isPlaying) currentStep = 0;
                } else if (touch.x < 120) {
                    for (int s = 0; s < 16; s++) {
                        for (int t = 0; t < 8; t++) {
                            pattern.steps[s][t] = false;
                        }
                    }
                    savePattern();
                } else if (touch.x < 180) {
                    savePattern();
                } else {
                    savePattern();
                    return false;
                }
            }
        }
        return true;
    }
    
    String getName() const override { return "Sequencer"; }
    
    void cleanup() override {
        savePattern();
    }
    
    void setAppManager(void* manager) override {}
};

#endif