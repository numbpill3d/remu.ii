#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include "BaseApp.h"
#include "../DisplayManager/DisplayManager.h"
#include "../TouchInterface/TouchInterface.h"
#include "../SystemCore/SystemCore.h"

// ========================================
// AppManager - Dynamic app loading and launcher for remu.ii
// Manages app lifecycle, launcher UI, and context switching
// ========================================

// Maximum number of apps that can be registered
#define MAX_APPS 16
#define LAUNCHER_GRID_COLS 4
#define LAUNCHER_GRID_ROWS 4
#define LAUNCHER_ICON_SIZE 64
#define LAUNCHER_ICON_SPACING 80

// App registry entry
struct AppRegistryEntry {
    String name;
    String className;
    String filePath;
    AppMetadata metadata;
    BaseApp* instance;  // nullptr when not loaded
    bool isLoaded;
    bool isEnabled;
    size_t memoryUsage;
};

// Launcher UI state
enum LauncherState {
    LAUNCHER_MAIN,      // Main app grid
    LAUNCHER_MENU,      // System menu
    LAUNCHER_SETTINGS,  // Settings screen
    LAUNCHER_INFO,      // System info
    LAUNCHER_LOADING    // Loading screen
};

// App transition types
enum AppTransition {
    TRANSITION_NONE,
    TRANSITION_SLIDE_LEFT,
    TRANSITION_SLIDE_RIGHT,
    TRANSITION_FADE,
    TRANSITION_ZOOM
};

class AppManager {
private:
    // App registry
    AppRegistryEntry appRegistry[MAX_APPS];
    uint8_t registeredAppCount;
    
    // Current app state
    BaseApp* currentApp;
    int8_t currentAppIndex;
    String pendingAppName;
    
    // Launcher state
    LauncherState launcherState;
    uint8_t selectedAppIndex;
    uint8_t launcherPage;
    uint8_t totalPages;
    
    // UI state
    bool showLauncher;
    unsigned long lastUpdateTime;
    AppTransition currentTransition;
    uint8_t transitionProgress;
    
    // Memory management
    size_t availableMemory;
    size_t memoryLimit;
    
    // Private methods - App Management
    bool loadApp(uint8_t appIndex);
    void unloadApp(uint8_t appIndex);
    bool switchToApp(uint8_t appIndex);
    void scanForApps();
    bool registerApp(String name, String className, String filePath);
    
    // Private methods - Launcher UI
    void drawLauncher();
    void drawAppGrid();
    void drawAppIcon(uint8_t appIndex, int16_t x, int16_t y, bool selected);
    void drawSystemMenu();
    void drawSettingsScreen();
    void drawInfoScreen();
    void drawStatusBar();
    void drawLoadingScreen();
    
    // Private methods - Navigation
    void handleLauncherTouch(TouchPoint touch);
    void handleAppGridTouch(TouchPoint touch);
    void handleMenuTouch(TouchPoint touch);
    uint8_t getTouchedAppIndex(TouchPoint touch);
    
    // Private methods - Transitions
    void startTransition(AppTransition type);
    void updateTransition();
    void finishTransition();
    
    // Private methods - Memory Management
    void checkMemoryUsage();
    bool hasEnoughMemoryForApp(uint8_t appIndex);
    void freeMemoryForApp(size_t requiredMemory);
    
    // Built-in icons (16x16 bitmaps)
    const uint8_t* getDefaultIcon(AppCategory category);
    
public:
    AppManager();
    ~AppManager();
    
    // Core lifecycle
    bool initialize();
    void update();
    void render();
    void shutdown();
    
    // App management
    bool launchApp(String appName);
    bool launchApp(uint8_t appIndex);
    void exitCurrentApp();
    void returnToLauncher();
    bool isAppRunning() const { return currentApp != nullptr; }
    BaseApp* getCurrentApp() const { return currentApp; }
    String getCurrentAppName() const;
    
    // App registry
    uint8_t getAppCount() const { return registeredAppCount; }
    AppRegistryEntry getAppInfo(uint8_t index) const;
    int8_t findAppByName(String name) const;
    bool isAppLoaded(String name) const;
    bool isAppEnabled(String name) const;
    void setAppEnabled(String name, bool enabled);
    
    // Launcher control
    void showLauncherScreen() { showLauncher = true; launcherState = LAUNCHER_MAIN; }
    void hideLauncherScreen() { showLauncher = false; }
    bool isLauncherVisible() const { return showLauncher; }
    void setLauncherPage(uint8_t page);
    uint8_t getCurrentPage() const { return launcherPage; }
    uint8_t getTotalPages() const { return totalPages; }
    
    // Input handling
    bool handleTouch(TouchPoint touch);
    bool handleSystemEvent(uint8_t event);
    
    // System integration
    void handleLowMemory();
    void handleLowBattery();
    void handleSystemShutdown();
    
    // Configuration
    void setMemoryLimit(size_t limit) { memoryLimit = limit; }
    size_t getMemoryLimit() const { return memoryLimit; }
    size_t getAvailableMemory() const;
    size_t getTotalMemoryUsage() const;
    
    // Debugging and diagnostics
    void printAppRegistry();
    void printMemoryUsage();
    String getSystemStatus();
    void dumpAppState();
    
    // Built-in app registration (called from main)
    void registerBuiltinApps();
    
private:
    // Built-in app instances (forward declarations)
    void registerDigitalPetApp();
    void registerSequencerApp();
    void registerWiFiToolsApp();
    void registerBLEScannerApp();
    void registerCarClonerApp();
    void registerFreqScannerApp();
    void registerEntropyBeaconApp();
    
    // Built-in icons
    static const uint8_t ICON_SYSTEM[32];
    static const uint8_t ICON_TOOLS[32];
    static const uint8_t ICON_GAMES[32];
    static const uint8_t ICON_MEDIA[32];
    static const uint8_t ICON_COMM[32];
    static const uint8_t ICON_OTHER[32];
    static const uint8_t ICON_DIGITALPET[32];
    static const uint8_t ICON_SEQUENCER[32];
    static const uint8_t ICON_WIFI[32];
    static const uint8_t ICON_BLE[32];
    static const uint8_t ICON_CAR[32];
    static const uint8_t ICON_FREQ[32];
    static const uint8_t ICON_ENTROPY[32];
};

// Global app manager instance
extern AppManager appManager;

// Utility macros for app registration
#define REGISTER_APP(manager, name, className) \
    manager.registerApp(name, #className, "/apps/" name "/" name ".cpp")

#define CREATE_APP_INSTANCE(className) \
    new className()

#endif // APP_MANAGER_H