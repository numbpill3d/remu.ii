#ifndef WIFI_TOOLS_STUB_H
#define WIFI_TOOLS_STUB_H

#include "../../core/AppManager/BaseApp.h"
#include "../../core/FileSystem.h"
#include <WiFi.h>

struct WiFiNetwork {
    String ssid;
    int32_t rssi;
    wifi_auth_mode_t encryption;
    String bssid;
};

class WiFiToolsApp : public BaseApp {
private:
    std::vector<WiFiNetwork> networks;
    String logFile = "/logs/wifi_scan.log";
    int selectedNetwork = 0;
    int mode = 0; // 0=scan, 1=logs
    bool scanning = false;
    unsigned long lastScan = 0;
    
    void scanNetworks() {
        if (scanning) return;
        scanning = true;
        networks.clear();
        
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        int n = WiFi.scanNetworks();
        
        for (int i = 0; i < n; i++) {
            WiFiNetwork net;
            net.ssid = WiFi.SSID(i);
            net.rssi = WiFi.RSSI(i);
            net.encryption = WiFi.encryptionType(i);
            net.bssid = WiFi.BSSIDstr(i);
            networks.push_back(net);
        }
        
        logScanResults();
        scanning = false;
        lastScan = millis();
    }
    
    void logScanResults() {
        filesystem.ensureDirExists("/logs");
        String timestamp = String(millis());
        String logEntry = "[" + timestamp + "] Scan found " + String(networks.size()) + " networks:\n";
        
        for (const auto& net : networks) {
            logEntry += "  " + net.ssid + " (" + String(net.rssi) + "dBm) " + net.bssid + "\n";
        }
        logEntry += "\n";
        
        filesystem.appendFile(logFile, logEntry);
    }
    
public:
    WiFiToolsApp() {
        setMetadata("WiFiTools", "1.0", "remu.ii", "WiFi scanner with SD logging", CATEGORY_TOOLS, 10240);
    }
    
    bool initialize() override {
        Serial.println("[WiFiTools] Initializing WiFi scanner...");
        WiFi.mode(WIFI_STA);
        return true;
    }
    
    void update() override {
        // Auto-scan every 30 seconds
        if (mode == 0 && !scanning && millis() - lastScan > 30000) {
            scanNetworks();
        }
    }
    
    void render() override {
        displayManager.clearScreen(COLOR_BLACK);
        
        displayManager.setFont(FONT_MEDIUM);
        displayManager.drawText(10, 10, "WiFi Tools", COLOR_GREEN_PHOS);
        
        if (mode == 0) {
            displayManager.drawText(10, 30, scanning ? "Scanning..." : "Networks: " + String(networks.size()), COLOR_WHITE);
            
            // Show networks
            int y = 50;
            for (int i = 0; i < min(8, (int)networks.size()); i++) {
                uint16_t color = (i == selectedNetwork) ? COLOR_RED_GLOW : COLOR_WHITE;
                String line = networks[i].ssid;
                if (line.length() > 20) line = line.substring(0, 17) + "...";
                line += " " + String(networks[i].rssi);
                
                displayManager.setFont(FONT_SMALL);
                displayManager.drawText(15, y, line, color);
                y += 15;
            }
            
            displayManager.setFont(FONT_SMALL);
            displayManager.drawText(20, 200, "SCAN", COLOR_WHITE);
            displayManager.drawText(80, 200, "LOGS", COLOR_WHITE);
            displayManager.drawText(140, 200, "CLEAR", COLOR_WHITE);
            displayManager.drawText(200, 200, "EXIT", COLOR_WHITE);
        } else {
            displayManager.drawText(10, 30, "Scan Logs", COLOR_WHITE);
            
            if (filesystem.fileExists(logFile)) {
                String logs = filesystem.readFile(logFile);
                int lines = 0;
                int pos = 0;
                while (pos < logs.length() && lines < 10) {
                    int nextLine = logs.indexOf('\n', pos);
                    if (nextLine == -1) break;
                    
                    String line = logs.substring(pos, nextLine);
                    if (line.length() > 35) line = line.substring(0, 32) + "...";
                    
                    displayManager.setFont(FONT_SMALL);
                    displayManager.drawText(15, 50 + lines * 12, line, COLOR_WHITE);
                    
                    pos = nextLine + 1;
                    lines++;
                }
            } else {
                displayManager.drawText(15, 50, "No logs found", COLOR_LIGHT_GRAY);
            }
            
            displayManager.setFont(FONT_SMALL);
            displayManager.drawText(20, 200, "BACK", COLOR_WHITE);
            displayManager.drawText(80, 200, "CLEAR", COLOR_WHITE);
            displayManager.drawText(200, 200, "EXIT", COLOR_WHITE);
        }
    }
    
    bool handleTouch(TouchPoint touch) override {
        if (touch.isNewPress) {
            if (touch.y >= 190) {
                if (mode == 0) {
                    if (touch.x < 60) { // SCAN
                        scanNetworks();
                    } else if (touch.x < 120) { // LOGS
                        mode = 1;
                    } else if (touch.x < 180) { // CLEAR
                        filesystem.deleteFile(logFile);
                    } else { // EXIT
                        return false;
                    }
                } else {
                    if (touch.x < 60) { // BACK
                        mode = 0;
                    } else if (touch.x < 120) { // CLEAR
                        filesystem.deleteFile(logFile);
                    } else if (touch.x > 180) { // EXIT
                        return false;
                    }
                }
            } else if (mode == 0 && touch.y >= 50 && touch.y < 170) {
                int selected = (touch.y - 50) / 15;
                if (selected < networks.size()) {
                    selectedNetwork = selected;
                }
            }
        }
        return true;
    }
    
    String getName() const override { return "WiFiTools"; }
    void setAppManager(void* manager) override {}
};

#endif