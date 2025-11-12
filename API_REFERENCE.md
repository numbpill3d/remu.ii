# remu.ii API Reference

Developer documentation for creating applications and extending the remu.ii platform.

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Core Systems](#core-systems)
   - [SystemCore](#systemcore)
   - [DisplayManager](#displaymanager)
   - [TouchInterface](#touchinterface)
   - [AppManager](#appmanager)
   - [FileSystem](#filesystem)
   - [Settings](#settings)
3. [Creating Applications](#creating-applications)
4. [BaseApp API](#baseapp-api)
5. [UI Components](#ui-components)
6. [Data Structures](#data-structures)
7. [Examples](#examples)

---

## Architecture Overview

remu.ii uses a modular architecture with core systems and pluggable applications:

```
┌─────────────────────────────────────────┐
│          remu_ii.ino (Main Loop)        │
└─────────────────┬───────────────────────┘
                  │
    ┌─────────────┴────────────────┐
    │                              │
┌───▼──────┐              ┌────────▼────────┐
│   Core   │              │   Applications  │
│ Systems  │              │   (BaseApp)     │
└───┬──────┘              └────────┬────────┘
    │                              │
    ├─ SystemCore                  ├─ DigitalPet
    ├─ DisplayManager              ├─ Sequencer
    ├─ TouchInterface              ├─ WiFiTools
    ├─ AppManager                  ├─ BLEScanner
    ├─ FileSystem                  ├─ CarCloner
    └─ Settings                    ├─ FreqScanner
                                   └─ EntropyBeacon
```

### Key Principles

- **Single Active App**: Only one app runs at a time to conserve memory
- **Event-Driven**: Apps respond to touch, system messages, and timer events
- **Persistent State**: Apps can save/load state from SD card
- **Global Singletons**: Core systems are globally accessible

---

## Core Systems

### SystemCore

**Header**: `core/SystemCore/SystemCore.h`
**Instance**: `systemCore` (global)

Manages system state, power, entropy, and watchdog.

#### Initialization

```cpp
bool initialize();
```

Initializes entropy sources, power monitoring, and watchdog. Call once in `setup()`.

**Returns**: `true` on success, `false` on failure.

#### System State

```cpp
SystemState getSystemState() const;
void setSystemState(SystemState state);
bool isSystemHealthy() const;
```

**System States**:
- `SYSTEM_BOOT`: Initial boot state
- `SYSTEM_RUNNING`: Normal operation
- `SYSTEM_LOW_POWER`: Battery saving mode
- `SYSTEM_ERROR`: Error occurred
- `SYSTEM_SHUTDOWN`: Shutting down

#### Entropy Generation

```cpp
uint32_t getRandomSeed();
uint8_t getRandomByte();
uint16_t getRandomWord();
uint32_t getRandomDWord();
void getRandomBytes(uint8_t* buffer, size_t length);
uint32_t getEntropyPool() const;
```

**Example**:
```cpp
// Get random number for pet behavior
uint32_t randomSeed = systemCore.getRandomSeed();

// Fill buffer with random data
uint8_t data[16];
systemCore.getRandomBytes(data, 16);
```

#### Power Management

```cpp
void updatePower();
PowerState getPowerState() const;
float getBatteryVoltage() const;
uint8_t getBatteryPercentage() const;
bool getChargingState() const;
```

**Power States**:
- `POWER_FULL`: >75% battery
- `POWER_GOOD`: 25-75% battery
- `POWER_LOW`: 10-25% battery
- `POWER_CRITICAL`: <10% battery

**Example**:
```cpp
if (systemCore.getPowerState() == POWER_CRITICAL) {
    // Save state and return to launcher
    saveState();
    appManager.exitCurrentApp();
}
```

#### System Information

```cpp
unsigned long getUptime() const;
unsigned long getUptimeSeconds() const;
size_t getFreeHeap() const;
size_t getMinFreeHeap() const;
String getSystemInfo() const;
```

**Example**:
```cpp
Serial.println(systemCore.getSystemInfo());
// Output:
// === remu.ii System Information ===
// Uptime: 3600 seconds
// Free Heap: 45000 bytes
// Battery: 87% (4.1V)
```

#### Watchdog

```cpp
void feedWatchdog();
void enableWatchdog();
void disableWatchdog();
```

**Important**: Call `feedWatchdog()` regularly (at least every 30 seconds) to prevent system reset.

#### System Utilities

```cpp
void resetSystem();
void enterDeepSleep(uint64_t sleepTimeMs);
void shutdown();
```

---

### DisplayManager

**Header**: `core/DisplayManager/DisplayManager.h`
**Instance**: `displayManager` (global)

Manages ILI9341 TFT display and UI rendering.

#### Initialization

```cpp
bool initialize();
```

Initializes display, sets rotation, and clears screen.

#### Basic Drawing

```cpp
void clearScreen(uint16_t color = COLOR_BLACK);
void setFont(FontSize size);
void drawText(int16_t x, int16_t y, String text, uint16_t color);
void drawPixel(int16_t x, int16_t y, uint16_t color);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
```

**Example**:
```cpp
displayManager.clearScreen(COLOR_BLACK);
displayManager.setFont(FONT_MEDIUM);
displayManager.drawText(10, 10, "Hello remu.ii!", COLOR_GREEN_PHOS);
displayManager.fillCircle(160, 120, 50, COLOR_RED_GLOW);
```

#### Colors

Predefined colors (16-bit RGB565):

| Color Constant | RGB | Use Case |
|----------------|-----|----------|
| `COLOR_BLACK` | 0x0000 | Background |
| `COLOR_WHITE` | 0xFFFF | Text |
| `COLOR_RED_GLOW` | 0xF802 | Alerts, danger |
| `COLOR_GREEN_PHOS` | 0x07E2 | Primary UI, terminal |
| `COLOR_PURPLE_GLOW` | 0x8017 | Accents |
| `COLOR_BLUE_CYBER` | 0x001F | Links, info |
| `COLOR_YELLOW` | 0xFFE0 | Warnings |
| `COLOR_DARK_GRAY` | 0x2104 | UI elements |
| `COLOR_LIGHT_GRAY` | 0x8410 | Disabled elements |

**Example**:
```cpp
displayManager.drawText(0, 0, "ERROR", COLOR_RED_GLOW);
```

#### UI Components

```cpp
// Buttons
void drawButton(int16_t x, int16_t y, int16_t w, int16_t h,
                String label, ButtonState state, uint16_t color);

// Windows
void drawWindow(int16_t x, int16_t y, int16_t w, int16_t h,
                String title, uint16_t bgColor, uint16_t borderColor);

// Progress bars
void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h,
                     float progress, uint16_t fgColor, uint16_t bgColor);

// Scrollbars
void drawScrollbar(int16_t x, int16_t y, int16_t h,
                   float scrollPos, float visibleRatio);

// Checkboxes
void drawCheckbox(int16_t x, int16_t y, bool checked, String label);

// Sliders
void drawSlider(int16_t x, int16_t y, int16_t w, int16_t min,
                int16_t max, int16_t value);
```

**Button States**:
- `BUTTON_NORMAL`: Default state
- `BUTTON_PRESSED`: When touched
- `BUTTON_DISABLED`: Inactive

#### Advanced Drawing

```cpp
// Terminal-style text
void drawTerminalText(int16_t x, int16_t y, String text, uint16_t color);

// Glitch effects
void drawGlitchEffect(int16_t x, int16_t y, int16_t w, int16_t h);

// 3D borders
void draw3DBorder(int16_t x, int16_t y, int16_t w, int16_t h, bool raised);

// Icons and sprites (16x16, 32x32)
void drawIcon(int16_t x, int16_t y, const uint8_t* iconData, int16_t size);
```

#### Screen Management

```cpp
void setBrightness(uint8_t level);  // 0-255
void setRotation(uint8_t rotation); // 0-3
int16_t width();
int16_t height();
```

---

### TouchInterface

**Header**: `core/TouchInterface/TouchInterface.h`
**Instance**: `touchInterface` (global)

Handles 4-wire resistive touch input and gesture recognition.

#### Touch Point Structure

```cpp
struct TouchPoint {
    int16_t x, y;           // Calibrated screen coordinates
    int16_t rawX, rawY;     // Raw ADC values
    uint16_t pressure;      // 0-1023 (higher = harder press)
    bool isPressed;         // Currently touching
    bool justPressed;       // Pressed this frame
    bool justReleased;      // Released this frame
    bool isValid;           // Valid touch detected
    unsigned long timestamp;
};
```

#### Getting Touch Input

```cpp
TouchPoint getCurrentTouch();
```

**Example**:
```cpp
TouchPoint touch = touchInterface.getCurrentTouch();
if (touch.justPressed) {
    Serial.printf("Touch at (%d, %d), pressure: %d\n",
                  touch.x, touch.y, touch.pressure);
}
```

#### Gesture Detection

```cpp
Gesture detectGestures();
```

**Gesture Types**:
```cpp
enum GestureType {
    GESTURE_NONE,
    GESTURE_TAP,
    GESTURE_DOUBLE_TAP,
    GESTURE_LONG_PRESS,
    GESTURE_DRAG,
    GESTURE_DRAG_MOVE,
    GESTURE_DRAG_END,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN
};
```

**Gesture Structure**:
```cpp
struct Gesture {
    GestureType type;
    TouchPoint startPoint;
    TouchPoint currentPoint;
    TouchPoint endPoint;
    int16_t deltaX, deltaY;
    unsigned long duration;
    float velocity;  // pixels per second
};
```

**Example**:
```cpp
Gesture gesture = touchInterface.detectGestures();
if (gesture.type == GESTURE_SWIPE_LEFT) {
    // Go to next page
    currentPage++;
}
```

#### Calibration

```cpp
void startCalibration();
bool isCalibrating() const;
void saveCalibration();
void loadCalibration();
```

---

### AppManager

**Header**: `core/AppManager/AppManager.h`
**Instance**: `appManager` (global)

Manages application lifecycle and app launcher.

#### App Registration

```cpp
bool registerApp(BaseApp* app);
```

**Example**:
```cpp
DigitalPetApp* petApp = new DigitalPetApp();
appManager.registerApp(petApp);
```

#### App Launching

```cpp
bool launchApp(uint8_t appIndex);
bool launchApp(String appName);
void exitCurrentApp();
```

**Example**:
```cpp
// Launch by index
appManager.launchApp(0);

// Launch by name
appManager.launchApp("Digital Pet");

// Exit current app
appManager.exitCurrentApp();
```

#### App Information

```cpp
uint8_t getAppCount() const;
BaseApp* getApp(uint8_t index) const;
BaseApp* getCurrentApp() const;
bool hasRunningApp() const;
```

#### Launcher

```cpp
void showLauncher();
void renderLauncher();
bool handleLauncherTouch(TouchPoint touch);
```

---

### FileSystem

**Header**: `core/FileSystem.h`
**Instance**: `FileSystem::getInstance()` (singleton)

Provides SD card access and file operations.

#### Initialization

```cpp
bool initialize();
bool isReady() const;
```

#### File Operations

```cpp
bool fileExists(String path);
bool createDirectory(String path);
bool deleteFile(String path);
bool deleteDirectory(String path);
```

#### Reading Files

```cpp
String readFile(String path);
bool readFile(String path, uint8_t* buffer, size_t maxSize, size_t* bytesRead);
```

**Example**:
```cpp
auto& fs = FileSystem::getInstance();
String data = fs.readFile("/settings/config.json");
if (data.length() > 0) {
    // Parse JSON...
}
```

#### Writing Files

```cpp
bool writeFile(String path, String content);
bool writeFile(String path, const uint8_t* data, size_t size);
bool appendFile(String path, String content);
```

**Example**:
```cpp
auto& fs = FileSystem::getInstance();
bool success = fs.writeFile("/logs/activity.log", "App launched\n");
```

#### Directory Structure

Standard directories (auto-created):
- `/apps/` - Application data
- `/data/` - User data
- `/samples/` - Audio samples
- `/settings/` - Configuration files
- `/temp/` - Temporary files
- `/logs/` - Event logs

---

### Settings

**Header**: `core/Settings/Settings.h`
**Instance**: `Settings::getInstance()` (singleton)

Manages persistent JSON configuration.

#### Initialization

```cpp
bool initialize();
bool load();
bool save();
```

#### Get/Set Values

```cpp
String getString(String key, String defaultValue = "");
void setString(String key, String value);

int getInt(String key, int defaultValue = 0);
void setInt(String key, int value);

float getFloat(String key, float defaultValue = 0.0f);
void setFloat(String key, float value);

bool getBool(String key, bool defaultValue = false);
void setBool(String key, bool value);
```

**Example**:
```cpp
auto& settings = Settings::getInstance();

// Read setting
int brightness = settings.getInt("brightness", 128);

// Write setting
settings.setInt("brightness", 200);
settings.save();  // Persist to SD card
```

#### App-Specific Settings

```cpp
// Scoped to avoid conflicts
settings.setInt("pet.mood", CALM);
int mood = settings.getInt("pet.mood", RESTLESS);
```

---

## Creating Applications

### Step 1: Create App Class

Create header file `apps/MyApp/MyApp.h`:

```cpp
#ifndef MY_APP_H
#define MY_APP_H

#include "../../core/AppManager/BaseApp.h"

class MyApp : public BaseApp {
private:
    // Private state
    int counter;

public:
    MyApp();
    ~MyApp();

    // Required methods (pure virtual in BaseApp)
    bool initialize() override;
    void update() override;
    void render() override;
    bool handleTouch(TouchPoint touch) override;

    // Optional overrides
    void shutdown() override;
    bool saveState() override;
    bool loadState() override;
    void onLowMemory() override;
};

#endif
```

### Step 2: Implement Methods

Create implementation file `apps/MyApp/MyApp.cpp`:

```cpp
#include "MyApp.h"

MyApp::MyApp() : counter(0) {
    // Set metadata
    setMetadata(
        "My App",              // Name
        "1.0",                 // Version
        "Your Name",           // Author
        "Description",         // Description
        CATEGORY_TOOLS,        // Category
        10000                  // Memory requirement (bytes)
    );

    setRequirements(false, false, false);  // SD, WiFi, BLE
}

MyApp::~MyApp() {
    shutdown();
}

bool MyApp::initialize() {
    Serial.println("[MyApp] Initializing...");

    // Load saved state
    loadState();

    setState(APP_RUNNING);
    return true;
}

void MyApp::update() {
    // Called every frame
    counter++;
}

void MyApp::render() {
    // Draw UI
    displayManager.clearScreen(COLOR_BLACK);
    displayManager.setFont(FONT_LARGE);
    displayManager.drawText(50, 100, "Counter: " + String(counter),
                           COLOR_GREEN_PHOS);

    // Draw button
    displayManager.drawButton(100, 180, 120, 40, "Exit",
                             BUTTON_NORMAL, COLOR_RED_GLOW);
}

bool MyApp::handleTouch(TouchPoint touch) {
    if (!touch.justPressed) return false;

    // Check if exit button pressed
    if (touch.x >= 100 && touch.x <= 220 &&
        touch.y >= 180 && touch.y <= 220) {
        // Save and exit
        saveState();
        appManager.exitCurrentApp();
        return true;
    }

    return false;
}

void MyApp::shutdown() {
    saveState();
}

bool MyApp::saveState() {
    auto& fs = FileSystem::getInstance();

    // Create JSON
    DynamicJsonDocument doc(256);
    doc["counter"] = counter;

    // Serialize to string
    String json;
    serializeJson(doc, json);

    // Write to file
    return fs.writeFile("/apps/myapp/state.json", json);
}

bool MyApp::loadState() {
    auto& fs = FileSystem::getInstance();

    String json = fs.readFile("/apps/myapp/state.json");
    if (json.length() == 0) return false;

    // Parse JSON
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    if (error) return false;

    // Load state
    counter = doc["counter"] | 0;
    return true;
}
```

### Step 3: Register App

In `remu_ii.ino`, add your app:

```cpp
#include "apps/MyApp/MyApp.h"

MyApp* myApp;

void setup() {
    // ... existing setup ...

    // Register your app
    myApp = new MyApp();
    appManager.registerApp(myApp);
}
```

---

## BaseApp API

### Required Methods (Pure Virtual)

Must be implemented by all apps:

```cpp
virtual bool initialize() = 0;
virtual void update() = 0;
virtual void render() = 0;
virtual bool handleTouch(TouchPoint touch) = 0;
```

### Optional Methods (Virtual)

Can be overridden:

```cpp
virtual void cleanup();               // Called before app unloads
virtual void shutdown();              // Called on exit
virtual void onPause();               // App paused (not implemented yet)
virtual void onResume();              // App resumed (not implemented yet)
virtual void onLowMemory();           // Low heap warning
virtual void onLowBattery();          // Battery critical
virtual const uint8_t* getIcon() const;  // 16x16 icon bitmap
virtual bool saveState();             // Persist app state
virtual bool loadState();             // Restore app state
virtual bool handleMessage(AppMessage message);  // Inter-app messaging
```

### State Management

```cpp
void setState(AppState state);
AppState getState() const;
bool isRunning() const;
bool isPaused() const;
bool hasError() const;
```

### Redraw Management

```cpp
void setNeedsRedraw(bool redraw = true);
bool getNeedsRedraw() const;
```

**Example**:
```cpp
void MyApp::update() {
    if (dataChanged) {
        setNeedsRedraw();  // Request redraw on next frame
    }
}
```

---

## UI Components

### Creating a Button

```cpp
struct Button {
    int16_t x, y, w, h;
    String label;
    bool enabled;
};

Button myButton = {100, 180, 120, 40, "Click Me", true};

// In render():
displayManager.drawButton(myButton.x, myButton.y, myButton.w, myButton.h,
                         myButton.label,
                         buttonPressed ? BUTTON_PRESSED : BUTTON_NORMAL,
                         COLOR_GREEN_PHOS);

// In handleTouch():
if (touch.justPressed &&
    touch.x >= myButton.x && touch.x <= myButton.x + myButton.w &&
    touch.y >= myButton.y && touch.y <= myButton.y + myButton.h) {
    // Button clicked
}
```

### Creating a List View

```cpp
class ListView {
private:
    std::vector<String> items;
    int scrollOffset;
    int visibleItems;

public:
    void render(int16_t x, int16_t y, int16_t w, int16_t h) {
        int itemHeight = 20;
        for (int i = 0; i < visibleItems && (i + scrollOffset) < items.size(); i++) {
            int16_t itemY = y + i * itemHeight;
            displayManager.drawText(x + 5, itemY + 5,
                                   items[i + scrollOffset],
                                   COLOR_WHITE);
            displayManager.drawLine(x, itemY + itemHeight - 1,
                                   x + w, itemY + itemHeight - 1,
                                   COLOR_DARK_GRAY);
        }
    }

    void handleTouch(TouchPoint touch) {
        // Handle scrolling, selection
    }
};
```

---

## Data Structures

### Common Enums

```cpp
// App categories
enum AppCategory {
    CATEGORY_SYSTEM,
    CATEGORY_TOOLS,
    CATEGORY_GAMES,
    CATEGORY_MEDIA,
    CATEGORY_COMMUNICATION,
    CATEGORY_OTHER
};

// App states
enum AppState {
    APP_UNLOADED,
    APP_LOADING,
    APP_RUNNING,
    APP_PAUSED,
    APP_ERROR,
    APP_EXITING
};

// System states
enum SystemState {
    SYSTEM_BOOT,
    SYSTEM_RUNNING,
    SYSTEM_LOW_POWER,
    SYSTEM_ERROR,
    SYSTEM_SHUTDOWN
};
```

### App Messages

```cpp
enum AppMessageType {
    MSG_NONE = 0,
    MSG_ENTROPY_UPDATE,
    MSG_BATTERY_LOW,
    MSG_BATTERY_CRITICAL,
    MSG_SYSTEM_SHUTDOWN,
    MSG_MEMORY_WARNING,
    MSG_WIFI_CONNECTED,
    MSG_WIFI_DISCONNECTED,
    MSG_BLE_DEVICE_FOUND,
    MSG_SD_CARD_REMOVED,
    MSG_USER_CUSTOM = 100
};

struct AppMessage {
    AppMessageType type;
    void* data;
    size_t dataSize;
    unsigned long timestamp;
};
```

**Example - Handling Messages**:
```cpp
bool MyApp::handleMessage(AppMessage message) {
    switch (message.type) {
        case MSG_BATTERY_LOW:
            Serial.println("Battery low! Saving state...");
            saveState();
            return true;

        case MSG_ENTROPY_UPDATE:
            if (message.data) {
                float* entropy = (float*)message.data;
                // Use entropy value
            }
            return true;

        default:
            return false;
    }
}
```

---

## Examples

### Example 1: Simple Counter App

```cpp
class CounterApp : public BaseApp {
private:
    int count;
    Button incrementBtn;
    Button decrementBtn;

public:
    CounterApp() : count(0) {
        setMetadata("Counter", "1.0", "Dev", "Simple counter",
                   CATEGORY_TOOLS, 8000);
    }

    bool initialize() override {
        count = Settings::getInstance().getInt("counter.count", 0);
        incrementBtn = {50, 150, 100, 50, "+", true};
        decrementBtn = {170, 150, 100, 50, "-", true};
        setState(APP_RUNNING);
        return true;
    }

    void render() override {
        displayManager.clearScreen(COLOR_BLACK);

        // Display count
        displayManager.setFont(FONT_LARGE);
        String countText = String(count);
        displayManager.drawText(140, 80, countText, COLOR_GREEN_PHOS);

        // Draw buttons
        displayManager.drawButton(incrementBtn.x, incrementBtn.y,
                                 incrementBtn.w, incrementBtn.h,
                                 incrementBtn.label, BUTTON_NORMAL,
                                 COLOR_GREEN_PHOS);
        displayManager.drawButton(decrementBtn.x, decrementBtn.y,
                                 decrementBtn.w, decrementBtn.h,
                                 decrementBtn.label, BUTTON_NORMAL,
                                 COLOR_RED_GLOW);
    }

    void update() override {
        // Nothing to update each frame
    }

    bool handleTouch(TouchPoint touch) override {
        if (!touch.justPressed) return false;

        // Check increment button
        if (touch.x >= incrementBtn.x &&
            touch.x <= incrementBtn.x + incrementBtn.w &&
            touch.y >= incrementBtn.y &&
            touch.y <= incrementBtn.y + incrementBtn.h) {
            count++;
            setNeedsRedraw();
            return true;
        }

        // Check decrement button
        if (touch.x >= decrementBtn.x &&
            touch.x <= decrementBtn.x + decrementBtn.w &&
            touch.y >= decrementBtn.y &&
            touch.y <= decrementBtn.y + decrementBtn.h) {
            count--;
            setNeedsRedraw();
            return true;
        }

        return false;
    }

    bool saveState() override {
        Settings::getInstance().setInt("counter.count", count);
        return Settings::getInstance().save();
    }
};
```

### Example 2: Entropy Visualizer

```cpp
class EntropyViz : public BaseApp {
private:
    static const int BUFFER_SIZE = 100;
    uint8_t entropyBuffer[BUFFER_SIZE];
    int bufferIndex;
    unsigned long lastUpdate;

public:
    bool initialize() override {
        bufferIndex = 0;
        lastUpdate = 0;
        setState(APP_RUNNING);
        return true;
    }

    void update() override {
        unsigned long now = millis();
        if (now - lastUpdate >= 50) {  // 20 Hz sampling
            entropyBuffer[bufferIndex] = systemCore.getRandomByte();
            bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
            lastUpdate = now;
            setNeedsRedraw();
        }
    }

    void render() override {
        displayManager.clearScreen(COLOR_BLACK);

        // Draw waveform
        for (int i = 0; i < BUFFER_SIZE - 1; i++) {
            int x1 = i * 3;
            int y1 = 120 - (entropyBuffer[i] / 2);
            int x2 = (i + 1) * 3;
            int y2 = 120 - (entropyBuffer[(i + 1) % BUFFER_SIZE] / 2);

            displayManager.drawLine(x1, y1, x2, y2, COLOR_GREEN_PHOS);
        }

        // Draw current entropy value
        displayManager.setFont(FONT_MEDIUM);
        uint32_t entropy = systemCore.getEntropyPool();
        displayManager.drawText(10, 200, "Entropy: 0x" + String(entropy, HEX),
                               COLOR_YELLOW);
    }

    bool handleTouch(TouchPoint touch) override {
        // Exit on any touch
        if (touch.justPressed) {
            appManager.exitCurrentApp();
            return true;
        }
        return false;
    }
};
```

---

## Best Practices

### Memory Management

- **Monitor heap**: Check `systemCore.getFreeHeap()` regularly
- **Avoid large allocations**: Use static buffers where possible
- **Clean up**: Free resources in `shutdown()` or destructor
- **Respond to low memory**: Implement `onLowMemory()`

```cpp
void MyApp::update() {
    if (systemCore.getFreeHeap() < 8000) {
        // Emergency cleanup
        onLowMemory();
    }
}

void MyApp::onLowMemory() {
    // Free caches, reduce buffers, save state
}
```

### Performance

- **Minimize redraws**: Only redraw when necessary
- **Use dirty rectangles**: Only update changed regions
- **Batch drawing**: Group display operations
- **Avoid busy loops**: Use frame timing

```cpp
void MyApp::render() {
    if (!getNeedsRedraw()) return;  // Skip if no changes

    // Draw only changed elements
    drawChangedRegion();

    setNeedsRedraw(false);
}
```

### Touch Handling

- **Check justPressed**: Avoid repeated triggers
- **Use gesture detection**: For swipes, long presses
- **Provide visual feedback**: Show button press states
- **Debounce if needed**: Especially for mechanical switches

### File I/O

- **Check file exists** before reading
- **Handle errors**: File operations can fail
- **Close files**: Or use RAII pattern
- **Use JSON**: For structured data (ArduinoJson library)

### State Persistence

- **Save on exit**: Implement `saveState()`
- **Load on init**: Restore in `loadState()`
- **Handle failures**: Provide defaults if load fails
- **Version your data**: For future compatibility

```cpp
bool MyApp::saveState() {
    DynamicJsonDocument doc(512);
    doc["version"] = 1;  // Data format version
    doc["data"] = myData;

    String json;
    serializeJson(doc, json);
    return FileSystem::getInstance().writeFile("/apps/myapp/state.json", json);
}
```

---

## Debugging

### Serial Debug Output

```cpp
#define DEBUG 1

#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
#endif

// Usage:
DEBUG_PRINTLN("[MyApp] Starting initialization");
DEBUG_PRINTF("Free heap: %d bytes\n", systemCore.getFreeHeap());
```

### Error Logging

```cpp
void logError(String message) {
    String timestamp = String(millis());
    String logEntry = timestamp + ": " + message + "\n";

    FileSystem::getInstance().appendFile("/logs/errors.log", logEntry);
    Serial.println("[ERROR] " + message);
}
```

---

## Contributing

Want to contribute to remu.ii? Great!

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b my-new-feature`
3. **Write your code**: Follow existing style
4. **Test thoroughly**: On real hardware if possible
5. **Submit pull request**: With clear description

### Code Style

- **Indentation**: 4 spaces, no tabs
- **Naming**:
  - Classes: `PascalCase`
  - Functions: `camelCase()`
  - Variables: `camelCase`
  - Constants: `UPPER_CASE`
- **Comments**: Document public APIs
- **Includes**: Group by system, library, local

---

## Additional Resources

- **GitHub**: https://github.com/numbpill3d/remu.ii
- **Issues**: Report bugs and request features
- **Discussions**: Ask questions, share projects
- **Wiki**: Community-maintained guides

---

**Happy coding!**

*Build something weird.*

---

**Last Updated**: 2025-11-12
**API Version**: 1.0
**Compatibility**: remu.ii firmware 1.0+
