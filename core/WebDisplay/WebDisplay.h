#ifndef WEB_DISPLAY_H
#define WEB_DISPLAY_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "../Config.h"

// Web display mode - serves virtual display over HTTP + WebSocket
class WebDisplay {
private:
    WebServer* server;
    WebSocketsServer* wsServer;
    bool initialized;

    // Virtual display buffer (simplified for web)
    struct DisplayCommand {
        String type;  // "pixel", "rect", "text", "clear", etc.
        JsonObject params;
    };

    // Touch events from web client
    struct WebTouchEvent {
        int16_t x, y;
        bool pressed;
        unsigned long timestamp;
    };

    WebTouchEvent lastTouch;
    bool touchPending;

    // WebSocket message handlers
    static void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    static WebDisplay* instance;  // For static callback

    void handleWebSocketMessage(uint8_t clientNum, uint8_t* payload, size_t length);
    void sendDisplayUpdate(String command);

public:
    WebDisplay();
    ~WebDisplay();

    bool initialize(const char* ssid = "remu.ii", const char* password = "remuiiweb");
    void update();
    void shutdown();

    // Display methods (similar to DisplayManager but send to web)
    void clearScreen(uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void drawText(int16_t x, int16_t y, String text, uint16_t color, uint8_t size = 1);
    void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, String label,
                   bool pressed, uint16_t color);

    // Touch input from web
    bool hasTouchEvent();
    void getTouchEvent(int16_t& x, int16_t& y, bool& pressed);
    void clearTouchEvent();

    // Network info
    String getIPAddress();
    bool isClientConnected();

    // HTTP handlers
    void handleRoot();
    void handleNotFound();
};

#endif // WEB_DISPLAY_H
