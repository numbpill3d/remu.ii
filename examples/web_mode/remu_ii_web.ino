/*
 * remu.ii - Web Mode Example
 *
 * This sketch runs remu.ii in web mode, displaying the interface
 * in a web browser instead of on a physical TFT display.
 *
 * Perfect for:
 * - Testing without hardware
 * - Development and debugging
 * - Running on ESP32 connected to computer
 *
 * Hardware Required:
 * - ESP32 board (any variant)
 * - USB cable to computer
 *
 * No display, touch, SD card, or battery needed!
 */

#include "../../core/WebDisplay/WebDisplay.h"
#include "../../core/SystemCore/SystemCore.h"

// Configuration
#define WEB_MODE_SSID "remu.ii"
#define WEB_MODE_PASSWORD "remuiiweb"

// Global instances
WebDisplay webDisplay;
SystemCore systemCore;

// Simple demo app state
int counter = 0;
unsigned long lastUpdate = 0;
bool buttonPressed = false;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("========================================");
    Serial.println("  remu.ii - Web Mode");
    Serial.println("========================================");
    Serial.println();

    // Initialize system core (for entropy, etc.)
    Serial.println("[Setup] Initializing system core...");
    if (!systemCore.initialize()) {
        Serial.println("[Setup] ERROR: Failed to initialize system core!");
        return;
    }

    // Initialize web display
    Serial.println("[Setup] Initializing web display...");
    if (!webDisplay.initialize(WEB_MODE_SSID, WEB_MODE_PASSWORD)) {
        Serial.println("[Setup] ERROR: Failed to initialize web display!");
        return;
    }

    Serial.println();
    Serial.println("========================================");
    Serial.println("  SETUP COMPLETE!");
    Serial.println("========================================");
    Serial.println();
    Serial.println("INSTRUCTIONS:");
    Serial.println("1. Connect to WiFi network: " WEB_MODE_SSID);
    Serial.println("2. Password: " WEB_MODE_PASSWORD);
    Serial.println("3. Open browser to: http://" + webDisplay.getIPAddress());
    Serial.println();
    Serial.println("The virtual remu.ii display will appear in your browser!");
    Serial.println("========================================");
    Serial.println();

    // Clear screen to start
    webDisplay.clearScreen(0x0000);  // Black

    lastUpdate = millis();
}

void loop() {
    // Update web display (handle WebSocket and HTTP)
    webDisplay.update();

    // Update system core
    systemCore.update();
    systemCore.feedWatchdog();

    // Simple demo - update every 100ms
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= 100) {
        lastUpdate = currentTime;

        // Clear screen
        webDisplay.clearScreen(0x0000);  // Black

        // Draw title
        webDisplay.drawText(60, 20, "remu.ii Web Mode", 0x07E2, 2);  // Green

        // Draw counter
        webDisplay.drawText(80, 80, "Counter: " + String(counter), 0xFFFF, 2);  // White

        // Draw button
        webDisplay.drawButton(85, 130, 150, 50, "Click Me!", buttonPressed, 0xF802);  // Red

        // Draw status
        String status = webDisplay.isClientConnected() ? "Connected" : "Waiting...";
        webDisplay.drawText(10, 220, status, 0xFFE0, 1);  // Yellow

        // Draw battery (fake for demo)
        uint8_t batteryPercent = systemCore.getBatteryPercentage();
        webDisplay.drawText(240, 220, "Bat: " + String(batteryPercent) + "%", 0xFFFF, 1);
    }

    // Handle touch input from web
    if (webDisplay.hasTouchEvent()) {
        int16_t x, y;
        bool pressed;
        webDisplay.getTouchEvent(x, y, pressed);

        // Check if button was clicked (85, 130, 150x50)
        if (pressed && x >= 85 && x <= 235 && y >= 130 && y <= 180) {
            buttonPressed = true;
            counter++;
            Serial.printf("[Touch] Button clicked! Counter: %d\n", counter);
        } else if (!pressed) {
            buttonPressed = false;
        }

        webDisplay.clearTouchEvent();
    }

    // Small delay to prevent tight loop
    delay(10);
}
