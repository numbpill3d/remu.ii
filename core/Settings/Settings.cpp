#include "Settings.h"

// ========================================
// STATIC MEMBERS AND CONSTANTS
// ========================================

Settings* Settings::instance = nullptr;

// Predefined setting key constants
const String Settings::AUDIO_ENABLED = "audio.enabled";
const String Settings::AUDIO_VOLUME = "audio.volume";
const String Settings::AUDIO_SAMPLE_RATE = "audio.sample_rate";
const String Settings::AUDIO_OUTPUT_MODE = "audio.output_mode";

const String Settings::DISPLAY_BRIGHTNESS = "display.brightness";
const String Settings::DISPLAY_TIMEOUT = "display.timeout";
const String Settings::DISPLAY_THEME = "display.theme";
const String Settings::DISPLAY_ORIENTATION = "display.orientation";
const String Settings::DISPLAY_ANIMATIONS = "display.animations";

const String Settings::SYSTEM_PET_CHOICE = "system.pet_choice";
const String Settings::SYSTEM_LANGUAGE = "system.language";
const String Settings::SYSTEM_TIMEZONE = "system.timezone";
const String Settings::SYSTEM_AUTO_SAVE = "system.auto_save";
const String Settings::SYSTEM_DEBUG_MODE = "system.debug_mode";

const String Settings::INTERFACE_TOUCH_SENSITIVITY = "interface.touch_sensitivity";
const String Settings::INTERFACE_HAPTIC_FEEDBACK = "interface.haptic_feedback";
const String Settings::INTERFACE_BUTTON_SOUNDS = "interface.button_sounds";
const String Settings::INTERFACE_DOUBLE_TAP_SPEED = "interface.double_tap_speed";

const String Settings::SECURITY_AUTO_LOCK = "security.auto_lock";
const String Settings::SECURITY_LOCK_TIMEOUT = "security.lock_timeout";
const String Settings::SECURITY_REQUIRE_PIN = "security.require_pin";
const String Settings::SECURITY_HIDE_SENSITIVE = "security.hide_sensitive";

const String Settings::PERFORMANCE_FRAME_RATE = "performance.frame_rate";
const String Settings::PERFORMANCE_MEMORY_MONITOR = "performance.memory_monitor";
const String Settings::PERFORMANCE_BATTERY_SAVER = "performance.battery_saver";
const String Settings::PERFORMANCE_CPU_FREQUENCY = "performance.cpu_frequency";

const String Settings::DEBUG_LOG_LEVEL = "debug.log_level";
const String Settings::DEBUG_SERIAL_OUTPUT = "debug.serial_output";
const String Settings::DEBUG_SHOW_FPS = "debug.show_fps";
const String Settings::DEBUG_MEMORY_INFO = "debug.memory_info";

// ========================================
// CONSTRUCTOR AND SINGLETON
// ========================================

Settings::Settings() :
    settings(nullptr),
    settingCount(0),
    maxSettings(64),  // Maximum 64 settings
    changeCallback(nullptr)
{
    configPath = "/settings/config.json";
    backupPath = "/settings/config_backup.json";
    
    // Allocate settings array
    settings = new Setting[maxSettings];
    if (!settings) {
        Serial.println("ERROR: Failed to allocate settings memory");
        maxSettings = 0;
    }
}

Settings& Settings::getInstance() {
    if (!instance) {
        instance = new Settings();
    }
    return *instance;
}

void Settings::cleanup() {
    if (instance) {
        if (instance->settings) {
            delete[] instance->settings;
        }
        delete instance;
        instance = nullptr;
    }
}

// ========================================
// INITIALIZATION AND PERSISTENCE
// ========================================

bool Settings::initialize() {
    Serial.println("Settings: Initializing...");
    
    // Ensure settings directory exists
    filesystem.ensureDirExists("/settings");
    
    // Initialize default settings
    initializeDefaultSettings();
    
    // Try to load existing settings
    if (!loadSettings()) {
        Serial.println("Settings: No existing config found, using defaults");
        // Save defaults to file
        saveSettings();
    }
    
    Serial.println("Settings: Initialized successfully");
    return true;
}

void Settings::initializeDefaultSettings() {
    settingCount = 0;
    
    // Audio Settings
    Setting audioEnabled = {
        AUDIO_ENABLED, "Audio Enabled", "Enable/disable all audio output",
        SETTING_BOOL, CATEGORY_AUDIO,
        true, 0, 0.0f, "", 0x0000,  // values
        0, 1, nullptr, 0,            // constraints
        true, 0, 0.0f, "", 0x0000,   // defaults
        false, false, true           // flags
    };
    registerSetting(audioEnabled);
    
    Setting audioVolume = {
        AUDIO_VOLUME, "Master Volume", "System-wide audio volume level",
        SETTING_INT, CATEGORY_AUDIO,
        false, AUDIO_VOLUME_DEFAULT, 0.0f, "", 0x0000,
        0, AUDIO_VOLUME_MAX, nullptr, 0,
        false, AUDIO_VOLUME_DEFAULT, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(audioVolume);
    
    String* audioModes = new String[3];
    audioModes[0] = "DAC"; audioModes[1] = "I2S"; audioModes[2] = "PWM";
    Setting audioOutputMode = {
        AUDIO_OUTPUT_MODE, "Audio Output", "Audio output method",
        SETTING_ENUM, CATEGORY_AUDIO,
        false, 0, 0.0f, "DAC", 0x0000,
        0, 2, audioModes, 3,
        false, 0, 0.0f, "DAC", 0x0000,
        true, false, true
    };
    registerSetting(audioOutputMode);
    
    // Display Settings
    Setting displayBrightness = {
        DISPLAY_BRIGHTNESS, "Brightness", "Screen brightness level",
        SETTING_INT, CATEGORY_DISPLAY,
        false, DISPLAY_BRIGHTNESS, 0.0f, "", 0x0000,
        10, 255, nullptr, 0,
        false, DISPLAY_BRIGHTNESS, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(displayBrightness);
    
    Setting displayTimeout = {
        DISPLAY_TIMEOUT, "Screen Timeout", "Auto-dim timeout in seconds",
        SETTING_INT, CATEGORY_DISPLAY,
        false, 30, 0.0f, "", 0x0000,
        5, 300, nullptr, 0,
        false, 30, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(displayTimeout);
    
    String* themes = new String[4];
    themes[0] = "Dark"; themes[1] = "Light"; themes[2] = "Retro"; themes[3] = "Neon";
    Setting displayTheme = {
        DISPLAY_THEME, "UI Theme", "User interface color theme",
        SETTING_ENUM, CATEGORY_DISPLAY,
        false, 0, 0.0f, "Dark", 0x0000,
        0, 3, themes, 4,
        false, 0, 0.0f, "Dark", 0x0000,
        false, false, true
    };
    registerSetting(displayTheme);
    
    Setting displayAnimations = {
        DISPLAY_ANIMATIONS, "Animations", "Enable UI animations",
        SETTING_BOOL, CATEGORY_DISPLAY,
        true, 0, 0.0f, "", 0x0000,
        0, 1, nullptr, 0,
        true, 0, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(displayAnimations);
    
    // System Settings
    String* pets = new String[5];
    pets[0] = "Cat"; pets[1] = "Dog"; pets[2] = "Robot"; pets[3] = "Dragon"; pets[4] = "None";
    Setting systemPetChoice = {
        SYSTEM_PET_CHOICE, "Digital Pet", "Choose your digital companion",
        SETTING_ENUM, CATEGORY_SYSTEM,
        false, 0, 0.0f, "Cat", 0x0000,
        0, 4, pets, 5,
        false, 0, 0.0f, "Cat", 0x0000,
        false, false, true
    };
    registerSetting(systemPetChoice);
    
    Setting systemAutoSave = {
        SYSTEM_AUTO_SAVE, "Auto Save", "Automatically save app states",
        SETTING_BOOL, CATEGORY_SYSTEM,
        true, 0, 0.0f, "", 0x0000,
        0, 1, nullptr, 0,
        true, 0, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(systemAutoSave);
    
    Setting systemDebugMode = {
        SYSTEM_DEBUG_MODE, "Debug Mode", "Enable debug features",
        SETTING_BOOL, CATEGORY_SYSTEM,
        false, 0, 0.0f, "", 0x0000,
        0, 1, nullptr, 0,
        false, 0, 0.0f, "", 0x0000,
        true, false, true
    };
    registerSetting(systemDebugMode);
    
    // Interface Settings
    Setting touchSensitivity = {
        INTERFACE_TOUCH_SENSITIVITY, "Touch Sensitivity", "Touch pressure sensitivity",
        SETTING_INT, CATEGORY_INTERFACE,
        false, 50, 0.0f, "", 0x0000,
        10, 100, nullptr, 0,
        false, 50, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(touchSensitivity);
    
    Setting buttonSounds = {
        INTERFACE_BUTTON_SOUNDS, "Button Sounds", "Play sounds on button press",
        SETTING_BOOL, CATEGORY_INTERFACE,
        true, 0, 0.0f, "", 0x0000,
        0, 1, nullptr, 0,
        true, 0, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(buttonSounds);
    
    Setting doubleTapSpeed = {
        INTERFACE_DOUBLE_TAP_SPEED, "Double Tap Speed", "Double tap detection speed",
        SETTING_INT, CATEGORY_INTERFACE,
        false, 300, 0.0f, "", 0x0000,
        100, 1000, nullptr, 0,
        false, 300, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(doubleTapSpeed);
    
    // Performance Settings
    Setting frameRate = {
        PERFORMANCE_FRAME_RATE, "Frame Rate", "Target frames per second",
        SETTING_INT, CATEGORY_PERFORMANCE,
        false, FPS_TARGET, 0.0f, "", 0x0000,
        15, 60, nullptr, 0,
        false, FPS_TARGET, 0.0f, "", 0x0000,
        true, false, true
    };
    registerSetting(frameRate);
    
    Setting memoryMonitor = {
        PERFORMANCE_MEMORY_MONITOR, "Memory Monitor", "Show memory usage info",
        SETTING_BOOL, CATEGORY_PERFORMANCE,
        false, 0, 0.0f, "", 0x0000,
        0, 1, nullptr, 0,
        false, 0, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(memoryMonitor);
    
    Setting batterySaver = {
        PERFORMANCE_BATTERY_SAVER, "Battery Saver", "Enable power saving mode",
        SETTING_BOOL, CATEGORY_PERFORMANCE,
        false, 0, 0.0f, "", 0x0000,
        0, 1, nullptr, 0,
        false, 0, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(batterySaver);
    
    // Debug Settings
    String* logLevels = new String[5];
    logLevels[0] = "None"; logLevels[1] = "Error"; logLevels[2] = "Warning"; 
    logLevels[3] = "Info"; logLevels[4] = "Debug";
    Setting debugLogLevel = {
        DEBUG_LOG_LEVEL, "Log Level", "Debug logging verbosity",
        SETTING_ENUM, CATEGORY_DEBUG,
        false, DEFAULT_LOG_LEVEL, 0.0f, "Info", 0x0000,
        0, 4, logLevels, 5,
        false, DEFAULT_LOG_LEVEL, 0.0f, "Info", 0x0000,
        false, false, true
    };
    registerSetting(debugLogLevel);
    
    Setting debugShowFPS = {
        DEBUG_SHOW_FPS, "Show FPS", "Display frame rate counter",
        SETTING_BOOL, CATEGORY_DEBUG,
        false, 0, 0.0f, "", 0x0000,
        0, 1, nullptr, 0,
        false, 0, 0.0f, "", 0x0000,
        false, false, true
    };
    registerSetting(debugShowFPS);
    
    Serial.println("Settings: Default settings initialized (" + String(settingCount) + " settings)");
}

bool Settings::loadSettings() {
    if (!filesystem.fileExists(configPath)) {
        return false;
    }
    
    String jsonData = filesystem.readFile(configPath);
    if (jsonData.length() == 0) {
        Serial.println("Settings: Config file is empty");
        return false;
    }
    
    return loadFromJson(jsonData);
}

bool Settings::saveSettings() {
    String jsonData = saveToJson();
    if (jsonData.length() == 0) {
        Serial.println("Settings: Failed to serialize settings");
        return false;
    }
    
    if (!filesystem.writeFile(configPath, jsonData)) {
        Serial.println("Settings: Failed to write config file");
        return false;
    }
    
    Serial.println("Settings: Configuration saved");
    return true;
}

bool Settings::resetToDefaults() {
    Serial.println("Settings: Resetting to defaults...");
    
    // Reset all settings to default values
    for (uint8_t i = 0; i < settingCount; i++) {
        Setting* setting = &settings[i];
        switch (setting->type) {
            case SETTING_BOOL:
                setting->boolValue = setting->defaultBool;
                break;
            case SETTING_INT:
            case SETTING_ENUM:
                setting->intValue = setting->defaultInt;
                break;
            case SETTING_FLOAT:
                setting->floatValue = setting->defaultFloat;
                break;
            case SETTING_STRING:
                setting->stringValue = setting->defaultString;
                break;
            case SETTING_COLOR:
                setting->colorValue = setting->defaultColor;
                break;
        }
    }
    
    return saveSettings();
}

bool Settings::createBackup() {
    if (!filesystem.fileExists(configPath)) {
        return false;
    }
    
    return filesystem.copyFile(configPath, backupPath);
}

bool Settings::restoreBackup() {
    if (!filesystem.fileExists(backupPath)) {
        Serial.println("Settings: No backup file found");
        return false;
    }
    
    if (filesystem.copyFile(backupPath, configPath)) {
        return loadSettings();
    }
    
    return false;
}

// ========================================
// JSON SERIALIZATION
// ========================================

bool Settings::loadFromJson(const String& jsonStr) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        Serial.println("Settings: JSON parsing failed: " + String(error.c_str()));
        return false;
    }
    
    JsonObject root = doc.as<JsonObject>();
    
    for (JsonPair kv : root) {
        String key = kv.key().c_str();
        Setting* setting = findSetting(key);
        
        if (!setting) {
            Serial.println("Settings: Unknown setting: " + key);
            continue;
        }
        
        switch (setting->type) {
            case SETTING_BOOL:
                setting->boolValue = kv.value().as<bool>();
                break;
            case SETTING_INT:
            case SETTING_ENUM:
                setting->intValue = kv.value().as<int>();
                break;
            case SETTING_FLOAT:
                setting->floatValue = kv.value().as<float>();
                break;
            case SETTING_STRING:
                setting->stringValue = kv.value().as<String>();
                break;
            case SETTING_COLOR:
                setting->colorValue = kv.value().as<uint16_t>();
                break;
        }
    }
    
    Serial.println("Settings: Loaded from JSON");
    return true;
}

String Settings::saveToJson() {
    DynamicJsonDocument doc(4096);
    JsonObject root = doc.to<JsonObject>();
    
    for (uint8_t i = 0; i < settingCount; i++) {
        Setting* setting = &settings[i];
        
        switch (setting->type) {
            case SETTING_BOOL:
                root[setting->key] = setting->boolValue;
                break;
            case SETTING_INT:
            case SETTING_ENUM:
                root[setting->key] = setting->intValue;
                break;
            case SETTING_FLOAT:
                root[setting->key] = setting->floatValue;
                break;
            case SETTING_STRING:
                root[setting->key] = setting->stringValue;
                break;
            case SETTING_COLOR:
                root[setting->key] = setting->colorValue;
                break;
        }
    }
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    return jsonStr;
}

// ========================================
// SETTING MANAGEMENT
// ========================================

bool Settings::registerSetting(const Setting& setting) {
    if (settingCount >= maxSettings) {
        Serial.println("Settings: Maximum settings reached");
        return false;
    }
    
    if (!isValidKey(setting.key)) {
        Serial.println("Settings: Invalid key: " + setting.key);
        return false;
    }
    
    if (findSetting(setting.key) != nullptr) {
        Serial.println("Settings: Setting already exists: " + setting.key);
        return false;
    }
    
    settings[settingCount] = setting;
    settingCount++;
    
    return true;
}

bool Settings::unregisterSetting(const String& key) {
    Setting* setting = findSetting(key);
    if (!setting) {
        return false;
    }
    
    // Find index and shift array
    uint8_t index = setting - settings;
    for (uint8_t i = index; i < settingCount - 1; i++) {
        settings[i] = settings[i + 1];
    }
    settingCount--;
    
    return true;
}

Setting* Settings::findSetting(const String& key) {
    for (uint8_t i = 0; i < settingCount; i++) {
        if (settings[i].key == key) {
            return &settings[i];
        }
    }
    return nullptr;
}

bool Settings::validateSetting(const String& key, const Setting& setting) {
    switch (setting.type) {
        case SETTING_INT:
        case SETTING_ENUM:
            return (setting.intValue >= setting.minValue && 
                   setting.intValue <= setting.maxValue);
        case SETTING_FLOAT:
            return (setting.floatValue >= setting.minValue && 
                   setting.floatValue <= setting.maxValue);
        default:
            return true;
    }
}

// ========================================
// VALUE GETTERS
// ========================================

bool Settings::getBool(const String& key, bool defaultValue) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_BOOL) {
        return defaultValue;
    }
    return setting->boolValue;
}

int Settings::getInt(const String& key, int defaultValue) {
    Setting* setting = findSetting(key);
    if (!setting || (setting->type != SETTING_INT && setting->type != SETTING_ENUM)) {
        return defaultValue;
    }
    return setting->intValue;
}

float Settings::getFloat(const String& key, float defaultValue) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_FLOAT) {
        return defaultValue;
    }
    return setting->floatValue;
}

String Settings::getString(const String& key, const String& defaultValue) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_STRING) {
        return defaultValue;
    }
    return setting->stringValue;
}

uint16_t Settings::getColor(const String& key, uint16_t defaultValue) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_COLOR) {
        return defaultValue;
    }
    return setting->colorValue;
}

// ========================================
// VALUE SETTERS
// ========================================

bool Settings::setBool(const String& key, bool value) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_BOOL || setting->isReadOnly) {
        return false;
    }
    
    if (setting->boolValue != value) {
        setting->boolValue = value;
        notifyChange(key, *setting);
    }
    return true;
}

bool Settings::setInt(const String& key, int value) {
    Setting* setting = findSetting(key);
    if (!setting || (setting->type != SETTING_INT && setting->type != SETTING_ENUM) || 
        setting->isReadOnly) {
        return false;
    }
    
    if (value < setting->minValue || value > setting->maxValue) {
        return false;
    }
    
    if (setting->intValue != value) {
        setting->intValue = value;
        notifyChange(key, *setting);
    }
    return true;
}

bool Settings::setFloat(const String& key, float value) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_FLOAT || setting->isReadOnly) {
        return false;
    }
    
    if (value < setting->minValue || value > setting->maxValue) {
        return false;
    }
    
    if (setting->floatValue != value) {
        setting->floatValue = value;
        notifyChange(key, *setting);
    }
    return true;
}

bool Settings::setString(const String& key, const String& value) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_STRING || setting->isReadOnly) {
        return false;
    }
    
    if (setting->stringValue != value) {
        setting->stringValue = value;
        notifyChange(key, *setting);
    }
    return true;
}

bool Settings::setColor(const String& key, uint16_t value) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_COLOR || setting->isReadOnly) {
        return false;
    }
    
    if (setting->colorValue != value) {
        setting->colorValue = value;
        notifyChange(key, *setting);
    }
    return true;
}

// ========================================
// ENUM HELPERS
// ========================================

int Settings::getEnumIndex(const String& key, int defaultIndex) {
    return getInt(key, defaultIndex);
}

String Settings::getEnumValue(const String& key, const String& defaultValue) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_ENUM) {
        return defaultValue;
    }
    
    int index = setting->intValue;
    if (index < 0 || index >= setting->enumCount || !setting->enumOptions) {
        return defaultValue;
    }
    
    return setting->enumOptions[index];
}

bool Settings::setEnumIndex(const String& key, int index) {
    return setInt(key, index);
}

bool Settings::setEnumValue(const String& key, const String& value) {
    Setting* setting = findSetting(key);
    if (!setting || setting->type != SETTING_ENUM || !setting->enumOptions) {
        return false;
    }
    
    // Find matching enum value
    for (uint8_t i = 0; i < setting->enumCount; i++) {
        if (setting->enumOptions[i] == value) {
            return setInt(key, i);
        }
    }
    
    return false;
}

// ========================================
// QUERY METHODS
// ========================================

bool Settings::exists(const String& key) {
    return findSetting(key) != nullptr;
}

SettingType Settings::getType(const String& key) {
    Setting* setting = findSetting(key);
    return setting ? setting->type : SETTING_BOOL;
}

SettingCategory Settings::getCategory(const String& key) {
    Setting* setting = findSetting(key);
    return setting ? setting->category : CATEGORY_SYSTEM;
}

String Settings::getName(const String& key) {
    Setting* setting = findSetting(key);
    return setting ? setting->name : "";
}

String Settings::getDescription(const String& key) {
    Setting* setting = findSetting(key);
    return setting ? setting->description : "";
}

bool Settings::needsRestart(const String& key) {
    Setting* setting = findSetting(key);
    return setting ? setting->needsRestart : false;
}

bool Settings::isReadOnly(const String& key) {
    Setting* setting = findSetting(key);
    return setting ? setting->isReadOnly : true;
}

uint8_t Settings::getSettingsInCategory(SettingCategory category, String* keys, uint8_t maxKeys) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < settingCount && count < maxKeys; i++) {
        if (settings[i].category == category && settings[i].isVisible) {
            keys[count] = settings[i].key;
            count++;
        }
    }
    return count;
}

String Settings::getCategoryName(SettingCategory category) {
    switch (category) {
        case CATEGORY_AUDIO: return "Audio";
        case CATEGORY_DISPLAY: return "Display";
        case CATEGORY_SYSTEM: return "System";
        case CATEGORY_INTERFACE: return "Interface";
        case CATEGORY_SECURITY: return "Security";
        case CATEGORY_PERFORMANCE: return "Performance";
        case CATEGORY_DEBUG: return "Debug";
        default: return "Unknown";
    }
}

uint8_t Settings::getAllSettings(String* keys, uint8_t maxKeys) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < settingCount && count < maxKeys; i++) {
        if (settings[i].isVisible) {
            keys[count] = settings[i].key;
            count++;
        }
    }
    return count;
}

// ========================================
// UTILITY METHODS
// ========================================

void Settings::setChangeCallback(SettingsChangeCallback callback) {
    changeCallback = callback;
}

void Settings::notifyChange(const String& key, const Setting& setting) {
    if (changeCallback) {
        changeCallback(key, setting);
    }
}

bool Settings::isValidKey(const String& key) {
    if (key.length() == 0 || key.length() > 32) {
        return false;
    }
    
    // Check for valid characters (alphanumeric, dots, underscores)
    for (unsigned int i = 0; i < key.length(); i++) {
        char c = key.charAt(i);
        if (!isalnum(c) && c != '.' && c != '_') {
            return false;
        }
    }
    
    return true;
}

String Settings::getSettingsInfo() {
    String info = "Settings Info:\n";
    info += "  Total: " + String(settingCount) + "/" + String(maxSettings) + "\n";
    info += "  Config: " + configPath + "\n";
    info += "  Backup: " + backupPath + "\n";
    
    // Category counts
    uint8_t categoryCounts[7] = {0};
    for (uint8_t i = 0; i < settingCount; i++) {
        categoryCounts[settings[i].category]++;
    }
    
    info += "  Categories:\n";
    for (uint8_t i = 0; i < 7; i++) {
        info += "    " + getCategoryName((SettingCategory)i) + ": " + 
                String(categoryCounts[i]) + "\n";
    }
    
    return info;
}

void Settings::printSettings() {
    Serial.println("=== Settings Dump ===");
    for (uint8_t i = 0; i < settingCount; i++) {
        Setting* s = &settings[i];
        Serial.print(s->key + " (" + getCategoryName(s->category) + "): ");
        
        switch (s->type) {
            case SETTING_BOOL:
                Serial.println(s->boolValue ? "true" : "false");
                break;
            case SETTING_INT:
                Serial.println(String(s->intValue));
                break;
            case SETTING_ENUM:
                Serial.println(getEnumValue(s->key, ""));
                break;
            case SETTING_FLOAT:
                Serial.println(String(s->floatValue));
                break;
            case SETTING_STRING:
                Serial.println("\"" + s->stringValue + "\"");
                break;
            case SETTING_COLOR:
                Serial.println("0x" + String(s->colorValue, HEX));
                break;
        }
    }
    Serial.println("=== End Settings ===");
}