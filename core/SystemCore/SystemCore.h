#ifndef SYSTEM_CORE_H
#define SYSTEM_CORE_H

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include "../Config/hardware_pins.h"

// ========================================
// SystemCore - Core system management for remu.ii
// Handles entropy generation, power monitoring, watchdog, uptime
// ========================================

// System states
enum SystemState {
    SYSTEM_BOOT,
    SYSTEM_RUNNING,
    SYSTEM_LOW_POWER,
    SYSTEM_ERROR,
    SYSTEM_SHUTDOWN
};

// Power states
enum PowerState {
    POWER_FULL,      // >75%
    POWER_GOOD,      // 25-75%
    POWER_LOW,       // 10-25%
    POWER_CRITICAL   // <10%
};

// Entropy buffer configuration
#define ENTROPY_BUFFER_SIZE 256
#define ENTROPY_SAMPLE_INTERVAL 10  // milliseconds
#define POWER_CHECK_INTERVAL 5000   // 5 seconds
#define WATCHDOG_TIMEOUT 30         // 30 seconds

class SystemCore {
private:
    // System state
    SystemState currentState;
    PowerState currentPowerState;
    
    // Timing
    unsigned long bootTime;
    unsigned long lastEntropyUpdate;
    unsigned long lastPowerCheck;
    
    // Entropy management
    uint8_t entropyBuffer[ENTROPY_BUFFER_SIZE];
    uint16_t entropyIndex;
    uint32_t entropyPool;
    
    // Power monitoring
    float batteryVoltage;
    uint8_t batteryPercentage;
    bool isCharging;
    
    // Private methods
    void initializeWatchdog();
    void updateEntropyFromPin(uint8_t pin);
    float readBatteryVoltage();
    uint8_t calculateBatteryPercentage(float voltage);
    void mixEntropy(uint32_t newEntropy);

public:
    SystemCore();
    ~SystemCore();
    
    // Core initialization and lifecycle
    bool initialize();
    void update();
    void shutdown();
    
    // System state management
    SystemState getSystemState() const { return currentState; }
    void setSystemState(SystemState state) { currentState = state; }
    bool isSystemHealthy() const;
    
    // Entropy generation
    void updateEntropy();
    uint32_t getRandomSeed();
    uint8_t getRandomByte();
    uint16_t getRandomWord();
    uint32_t getRandomDWord();
    void getRandomBytes(uint8_t* buffer, size_t length);
    uint32_t getEntropyPool() const { return entropyPool; }
    
    // Power management
    void updatePower();
    PowerState getPowerState() const { return currentPowerState; }
    float getBatteryVoltage() const { return batteryVoltage; }
    uint8_t getBatteryPercentage() const { return batteryPercentage; }
    bool getChargingState() const { return isCharging; }
    
    // System information
    unsigned long getUptime() const;
    unsigned long getUptimeSeconds() const;
    size_t getFreeHeap() const;
    size_t getMinFreeHeap() const;
    float getCPUTemperature() const;
    
    // Watchdog management
    void feedWatchdog();
    void enableWatchdog();
    void disableWatchdog();
    
    // System utilities
    void resetSystem();
    void enterDeepSleep(uint64_t sleepTimeMs);
    String getSystemInfo() const;
    void dumpSystemStats() const;
};

// Global system core instance
extern SystemCore systemCore;

#endif // SYSTEM_CORE_H