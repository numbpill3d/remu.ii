#ifndef BASE_APP_H
#define BASE_APP_H

#include <Arduino.h>
#include "../DisplayManager/DisplayManager.h"
#include "../TouchInterface/TouchInterface.h"
#include "../SystemCore/SystemCore.h"

// ========================================
// BaseApp - Abstract base class for all remu.ii applications
// Defines standardized app lifecycle and common functionality
// ========================================

// App state enumeration
enum AppState {
    APP_INACTIVE,       // App not loaded/running
    APP_INITIALIZING,   // App is starting up
    APP_RUNNING,        // App is active and running normally
    APP_PAUSED,         // App is paused (background state)
    APP_ERROR,          // App encountered an error
    APP_CLEANUP         // App is shutting down
};

// App category for organization
enum AppCategory {
    CATEGORY_SYSTEM,    // System utilities
    CATEGORY_TOOLS,     // Hacking/diagnostic tools
    CATEGORY_GAMES,     // Games and entertainment
    CATEGORY_MEDIA,     // Audio/visual apps
    CATEGORY_COMM,      // Communication apps
    CATEGORY_OTHER      // Miscellaneous
};

// App metadata structure
struct AppMetadata {
    String name;        // Display name
    String version;     // Version string
    String author;      // App author
    String description; // Brief description
    AppCategory category;
    const uint8_t* icon; // 16x16 icon data (optional)
    size_t maxMemory;   // Max memory usage estimate
    bool requiresSD;    // Requires SD card access
    bool requiresWiFi;  // Requires WiFi functionality
    bool requiresBLE;   // Requires Bluetooth functionality
};

// Common UI element IDs for standardization
enum CommonUIElements {
    UI_BACK_BUTTON = 0,
    UI_MENU_BUTTON = 1,
    UI_HOME_BUTTON = 2,
    UI_SETTINGS_BUTTON = 3,
    UI_HELP_BUTTON = 4,
    UI_CUSTOM_START = 10  // Custom UI elements start from ID 10
};

// App message types for inter-app communication
enum AppMessage {
    MSG_NONE = 0,
    MSG_PAUSE = 1,
    MSG_RESUME = 2,
    MSG_SHUTDOWN = 3,
    MSG_LOW_MEMORY = 4,
    MSG_LOW_BATTERY = 5,
    MSG_USER_DEFINED = 100  // User-defined messages start from 100
};

// Forward declaration
class AppManager;

class BaseApp {
protected:
    AppState currentState;
    AppMetadata metadata;
    AppManager* appManager;
    
    // App timing
    unsigned long startTime;
    unsigned long lastUpdateTime;
    unsigned long frameCount;
    
    // Memory tracking
    size_t initialHeap;
    size_t currentHeap;
    
    // Common UI state
    bool showBackButton;
    bool showStatusBar;
    uint16_t backgroundColor;
    uint16_t foregroundColor;
    
    // Helper methods for common operations
    virtual void drawCommonUI();
    virtual void drawStatusBar();
    virtual void drawBackButton();
    virtual bool handleCommonTouch(TouchPoint touch);
    
    // State transition helpers
    void setState(AppState newState);
    void logStateChange(AppState oldState, AppState newState);
    
    // Memory management helpers
    void trackMemoryUsage();
    bool checkMemoryLimit();
    
public:
    BaseApp();
    virtual ~BaseApp();
    
    // ===========================================
    // MANDATORY VIRTUAL METHODS - Must be implemented by all apps
    // ===========================================
    
    /**
     * Initialize the application
     * Called once when app is first loaded
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;
    
    /**
     * Update application logic
     * Called every frame while app is running
     * Should be lightweight and non-blocking
     */
    virtual void update() = 0;
    
    /**
     * Render the application
     * Called every frame after update()
     * All drawing operations should be done here
     */
    virtual void render() = 0;
    
    /**
     * Handle touch input
     * Called when touch events occur
     * @param touch Current touch state
     * @return true if touch was handled
     */
    virtual bool handleTouch(TouchPoint touch) = 0;
    
    /**
     * Cleanup resources
     * Called when app is being unloaded
     * Must free all allocated memory and resources
     */
    virtual void cleanup() = 0;
    
    /**
     * Get application name
     * @return App display name
     */
    virtual String getName() const = 0;
    
    /**
     * Get application icon
     * @return Pointer to 16x16 icon data (can return nullptr)
     */
    virtual const uint8_t* getIcon() const = 0;
    
    // ===========================================
    // OPTIONAL VIRTUAL METHODS - Can be overridden
    // ===========================================
    
    /**
     * Handle app pause (going to background)
     * Default implementation saves state
     */
    virtual void onPause();
    
    /**
     * Handle app resume (coming from background)
     * Default implementation restores state
     */
    virtual void onResume();
    
    /**
     * Handle app messages
     * @param message Message type
     * @param data Optional message data
     * @return true if message was handled
     */
    virtual bool handleMessage(AppMessage message, void* data = nullptr);
    
    /**
     * Handle system events (low battery, memory warnings, etc.)
     * @param event System event type
     * @return true if event was handled
     */
    virtual bool handleSystemEvent(uint8_t event);
    
    /**
     * Save app state to persistent storage
     * Called before app shutdown or system sleep
     * @return true if save successful
     */
    virtual bool saveState();
    
    /**
     * Load app state from persistent storage
     * Called during app initialization
     * @return true if load successful
     */
    virtual bool loadState();
    
    /**
     * Get app settings menu items
     * Override to provide app-specific settings
     * @return Number of settings items (0 = no settings)
     */
    virtual uint8_t getSettingsCount() const { return 0; }
    
    /**
     * Get settings item name
     * @param index Settings item index
     * @return Setting name
     */
    virtual String getSettingName(uint8_t index) const { return ""; }
    
    /**
     * Handle settings item selection
     * @param index Settings item index
     */
    virtual void handleSetting(uint8_t index) {}
    
    // ===========================================
    // PUBLIC INTERFACE - Used by AppManager
    // ===========================================
    
    // State management
    AppState getState() const { return currentState; }
    bool isRunning() const { return currentState == APP_RUNNING; }
    bool isActive() const { return currentState == APP_RUNNING || currentState == APP_PAUSED; }
    
    // Metadata access
    AppMetadata getMetadata() const { return metadata; }
    void setAppManager(AppManager* manager) { appManager = manager; }
    
    // Statistics
    unsigned long getRunTime() const { return millis() - startTime; }
    unsigned long getFrameCount() const { return frameCount; }
    float getFPS() const;
    size_t getMemoryUsage() const { return initialHeap - currentHeap; }
    
    // Common UI configuration
    void setShowBackButton(bool show) { showBackButton = show; }
    void setShowStatusBar(bool show) { showStatusBar = show; }
    void setColors(uint16_t bg, uint16_t fg) { backgroundColor = bg; foregroundColor = fg; }
    
    // Utility methods for apps
    void exitApp(); // Request return to launcher
    void launchApp(String appName); // Request launch of another app
    bool isSDAvailable(); // Check if SD card is available
    bool isWiFiAvailable(); // Check if WiFi is available
    bool isBLEAvailable(); // Check if BLE is available
    
    // File system helpers (if SD available)
    String getAppDataPath(); // Get app's data directory path
    bool createAppDataDir(); // Create app data directory
    
    // Common dialog helpers
    void showMessageDialog(String title, String message);
    bool showConfirmDialog(String title, String message);
    String showTextInputDialog(String title, String prompt, String defaultText = "");
    
    // Debug helpers
    void debugLog(String message);
    void debugPrint(String key, String value);
    void debugPrintMemory();
};

// Macro to help implement basic app metadata
#define IMPLEMENT_APP_METADATA(name, ver, auth, desc, cat) \
    String getName() const override { return name; } \
    void setMetadata() { \
        metadata.name = name; \
        metadata.version = ver; \
        metadata.author = auth; \
        metadata.description = desc; \
        metadata.category = cat; \
    }

// Macro for simple apps that don't need complex state management
#define SIMPLE_APP_METHODS() \
    void onPause() override {} \
    void onResume() override {} \
    bool saveState() override { return true; } \
    bool loadState() override { return true; }

#endif // BASE_APP_H