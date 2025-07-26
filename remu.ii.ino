/*
 * remu.ii - ESP32 Anti-Phone Framework
 * 
 * A stylus-based handheld system for autonomous hacking, anomaly interaction,
 * and cyberpet companionship.
 * 
 * Hardware:
 * - ESP32 WROOM
 * - Adafruit ILI9341 2.8" TFT with 4-wire resistive touch
 * - SD card for app storage
 * - FuelRod USB battery
 * 
 * Author: remu.ii project
 * Version: 1.0
 */

// ========================================
// INCLUDES
// ========================================

// Core system modules
#include "core/Config/hardware_pins.h"
#include "core/SystemCore/SystemCore.h"
#include "core/DisplayManager/DisplayManager.h"
#include "core/TouchInterface/TouchInterface.h"
#include "core/AppManager/AppManager.h"
#include "core/Settings/Settings.h"
#include "core/FileSystem.h"

// Standard libraries
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>

// ========================================
// GLOBAL VARIABLES
// ========================================

// System state
bool systemInitialized = false;
bool systemError = false;
String lastError = "";

// Timing control
unsigned long lastFrameTime = 0;
unsigned long frameCount = 0;
const unsigned long TARGET_FRAME_TIME = 33; // ~30 FPS (33ms per frame)

// Performance monitoring
unsigned long lastPerformanceCheck = 0;
float currentFPS = 0.0f;
size_t minFreeHeap = 0;

// System flags
bool lowPowerMode = false;
bool calibrationRequired = false;

// ========================================
// FORWARD DECLARATIONS
// ========================================

void initializeSystem();
void handleSystemError(String error);
void updatePerformanceStats();
void checkSystemHealth();
void handleLowPower();
void printSystemInfo();
void handleSerialCommands();

// ========================================
// ARDUINO SETUP
// ========================================

void setup() {
  // Initialize serial communication first for debugging
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("         remu.ii v1.0 Starting");
  Serial.println("    ESP32 Anti-Phone Framework");
  Serial.println("========================================");
  
  // Initialize EEPROM for settings storage
  EEPROM.begin(512);
  
  // Initialize system
  initializeSystem();
  
  // Print system information
  printSystemInfo();
  
  Serial.println("[MAIN] Setup complete - entering main loop");
  Serial.println("========================================");
  
  // Record initial heap for tracking
  minFreeHeap = ESP.getFreeHeap();
}

// ========================================
// ARDUINO MAIN LOOP
// ========================================

void loop() {
  unsigned long currentTime = millis();
  
  // Frame rate control
  if (currentTime - lastFrameTime < TARGET_FRAME_TIME) {
    return; // Skip this iteration to maintain frame rate
  }
  
  // Check for system errors
  if (systemError) {
    handleSystemError(lastError);
    return;
  }
  
  // Check for serial commands (debug interface)
  if (Serial.available()) {
    handleSerialCommands();
  }
  
  // Update all core systems
  if (systemInitialized) {
    // Update system core (entropy, power monitoring, watchdog)
    systemCore.update();
    
    // Update touch interface
    touchInterface.update();
    
    // Update display manager
    displayManager.update();
    
    // Update app manager (handles current app and launcher)
    appManager.update();
    
    // Handle touch input
    TouchPoint currentTouch = touchInterface.getCurrentTouch();
    if (currentTouch.isPressed || currentTouch.isNewPress || currentTouch.isNewRelease) {
      appManager.handleTouch(currentTouch);
    }
    
    // Render current screen (launcher or current app)
    appManager.render();
    
    // Check system health periodically
    if (currentTime - lastPerformanceCheck > 5000) { // Every 5 seconds
      checkSystemHealth();
      updatePerformanceStats();
      lastPerformanceCheck = currentTime;
    }
  }
  
  // Update frame timing
  lastFrameTime = currentTime;
  frameCount++;
  
  // Feed watchdog
  systemCore.feedWatchdog();
  
  // Handle low power mode if needed
  if (lowPowerMode) {
    handleLowPower();
  }
  
  // Small delay to prevent overwhelming the system
  delay(1);
}

// ========================================
// SYSTEM INITIALIZATION
// ========================================

void initializeSystem() {
  Serial.println("[MAIN] Initializing core systems...");
  
  // Initialize system core first (entropy, power monitoring, watchdog)
  Serial.print("[MAIN] Initializing SystemCore... ");
  if (!systemCore.initialize()) {
    handleSystemError("Failed to initialize SystemCore");
    return;
  }
  Serial.println("OK");
  
  // Initialize display manager
  Serial.print("[MAIN] Initializing DisplayManager... ");
  if (!displayManager.initialize()) {
    handleSystemError("Failed to initialize DisplayManager");
    return;
  }
  Serial.println("OK");
  
  // Show early boot message
  displayManager.clearScreen(COLOR_BLACK);
  displayManager.setFont(FONT_MEDIUM);
  displayManager.drawTextCentered(0, 100, SCREEN_WIDTH, "Initializing...", COLOR_GREEN_PHOS);
  
  // Initialize touch interface
  Serial.print("[MAIN] Initializing TouchInterface... ");
  if (!touchInterface.initialize()) {
    handleSystemError("Failed to initialize TouchInterface");
    return;
  }
  Serial.println("OK");
  
  // Check if touch calibration is needed
  if (!touchInterface.isCalibrated()) {
    Serial.println("[MAIN] Touch calibration required");
    calibrationRequired = true;
    
    // Show calibration prompt
    displayManager.clearScreen(COLOR_BLACK);
    displayManager.setFont(FONT_MEDIUM);
    displayManager.drawTextCentered(0, 80, SCREEN_WIDTH, "Touch Calibration", COLOR_RED_GLOW);
    displayManager.drawTextCentered(0, 110, SCREEN_WIDTH, "Required", COLOR_RED_GLOW);
    displayManager.setFont(FONT_SMALL);
    displayManager.drawTextCentered(0, 140, SCREEN_WIDTH, "Touch corners when prompted", COLOR_WHITE);
    delay(2000);
    
    // Perform calibration
    runTouchCalibration();
  }
  
  // Initialize filesystem (SD card)
  Serial.print("[MAIN] Initializing FileSystem... ");
  if (!filesystem.begin()) {
    Serial.println("WARNING: FileSystem initialization failed");
    Serial.println("[MAIN] System will continue without SD card support");
  } else {
    Serial.println("OK");
  }
  
  // Initialize WiFi (but don't connect yet)
  Serial.print("[MAIN] Initializing WiFi... ");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("OK");
  
  // Initialize app manager
  Serial.print("[MAIN] Initializing AppManager... ");
  if (!appManager.initialize()) {
    handleSystemError("Failed to initialize AppManager");
    return;
  }
  Serial.println("OK");
  
  // System initialization complete
  systemInitialized = true;
  systemError = false;
  
  Serial.println("[MAIN] All systems initialized successfully");
  
  // Show boot complete message briefly
  displayManager.clearScreen(COLOR_BLACK);
  displayManager.setFont(FONT_LARGE);
  displayManager.drawTextCentered(0, 90, SCREEN_WIDTH, "remu.ii", COLOR_RED_GLOW);
  displayManager.setFont(FONT_MEDIUM);
  displayManager.drawTextCentered(0, 120, SCREEN_WIDTH, "READY", COLOR_GREEN_PHOS);
  delay(1500);
  
  // Launch into app manager (shows launcher)
  appManager.showLauncherScreen();
}

// ========================================
// TOUCH CALIBRATION
// ========================================

void runTouchCalibration() {
  Serial.println("[MAIN] Running touch calibration...");
  
  touchInterface.startCalibration();
  
  // Calibration point 1: Top-left corner
  displayManager.clearScreen(COLOR_BLACK);
  displayManager.setFont(FONT_MEDIUM);
  displayManager.drawTextCentered(0, 50, SCREEN_WIDTH, "Touch the", COLOR_WHITE);
  displayManager.drawTextCentered(0, 80, SCREEN_WIDTH, "TOP-LEFT corner", COLOR_RED_GLOW);
  displayManager.drawTextCentered(0, 140, SCREEN_WIDTH, "with stylus", COLOR_WHITE);
  
  // Draw target crosshair
  displayManager.drawLine(5, 20, 15, 20, COLOR_RED_GLOW);
  displayManager.drawLine(10, 15, 10, 25, COLOR_RED_GLOW);
  
  if (!touchInterface.calibratePoint(0, 0)) {
    handleSystemError("Touch calibration failed");
    return;
  }
  
  delay(500);
  
  // Calibration point 2: Bottom-right corner
  displayManager.clearScreen(COLOR_BLACK);
  displayManager.setFont(FONT_MEDIUM);
  displayManager.drawTextCentered(0, 50, SCREEN_WIDTH, "Touch the", COLOR_WHITE);
  displayManager.drawTextCentered(0, 80, SCREEN_WIDTH, "BOTTOM-RIGHT corner", COLOR_RED_GLOW);
  displayManager.drawTextCentered(0, 140, SCREEN_WIDTH, "with stylus", COLOR_WHITE);
  
  // Draw target crosshair
  displayManager.drawLine(SCREEN_WIDTH-15, SCREEN_HEIGHT-20, SCREEN_WIDTH-5, SCREEN_HEIGHT-20, COLOR_RED_GLOW);
  displayManager.drawLine(SCREEN_WIDTH-10, SCREEN_HEIGHT-25, SCREEN_WIDTH-10, SCREEN_HEIGHT-15, COLOR_RED_GLOW);
  
  if (!touchInterface.calibratePoint(SCREEN_WIDTH-1, SCREEN_HEIGHT-1)) {
    handleSystemError("Touch calibration failed");
    return;
  }
  
  // Finish calibration
  if (touchInterface.finishCalibration()) {
    displayManager.clearScreen(COLOR_BLACK);
    displayManager.setFont(FONT_MEDIUM);
    displayManager.drawTextCentered(0, 100, SCREEN_WIDTH, "Calibration", COLOR_GREEN_PHOS);
    displayManager.drawTextCentered(0, 130, SCREEN_WIDTH, "Complete!", COLOR_GREEN_PHOS);
    delay(2000);
    
    Serial.println("[MAIN] Touch calibration successful");
  } else {
    handleSystemError("Failed to save calibration");
  }
}

// ========================================
// ERROR HANDLING
// ========================================

void handleSystemError(String error) {
  Serial.printf("[MAIN] SYSTEM ERROR: %s\n", error.c_str());
  
  systemError = true;
  lastError = error;
  
  // Show error on display
  displayManager.clearScreen(COLOR_BLACK);
  displayManager.setFont(FONT_MEDIUM);
  displayManager.drawTextCentered(0, 60, SCREEN_WIDTH, "SYSTEM ERROR", COLOR_RED_GLOW);
  
  displayManager.setFont(FONT_SMALL);
  // Split long error messages
  if (error.length() > 20) {
    String line1 = error.substring(0, 20);
    String line2 = error.substring(20);
    displayManager.drawTextCentered(0, 100, SCREEN_WIDTH, line1, COLOR_WHITE);
    displayManager.drawTextCentered(0, 120, SCREEN_WIDTH, line2, COLOR_WHITE);
  } else {
    displayManager.drawTextCentered(0, 110, SCREEN_WIDTH, error, COLOR_WHITE);
  }
  
  displayManager.drawTextCentered(0, 160, SCREEN_WIDTH, "Reset required", COLOR_LIGHT_GRAY);
  
  // Add some glitch effects for the error screen
  for (int i = 0; i < 10; i++) {
    displayManager.drawGlitch(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    delay(100);
  }
  
  // Halt system - require reset
  while (true) {
    delay(1000);
    Serial.printf("[MAIN] System halted due to error: %s\n", error.c_str());
  }
}

// ========================================
// SYSTEM MONITORING
// ========================================

void checkSystemHealth() {
  // Check memory
  size_t currentHeap = ESP.getFreeHeap();
  if (currentHeap < minFreeHeap) {
    minFreeHeap = currentHeap;
  }
  
  if (currentHeap < 10000) { // Less than 10KB
    Serial.println("[MAIN] WARNING: Low memory detected");
    appManager.handleLowMemory();
  }
  
  // Check power
  PowerState powerState = systemCore.getPowerState();
  if (powerState == POWER_CRITICAL) {
    Serial.println("[MAIN] WARNING: Critical battery level");
    lowPowerMode = true;
  } else if (powerState == POWER_LOW && !lowPowerMode) {
    Serial.println("[MAIN] WARNING: Low battery level");
  }
  
  // Check system core health
  if (!systemCore.isSystemHealthy()) {
    Serial.println("[MAIN] WARNING: System health check failed");
  }
}

void updatePerformanceStats() {
  // Calculate FPS
  static unsigned long lastFPSCalculation = 0;
  static unsigned long lastFrameCount = 0;
  
  unsigned long currentTime = millis();
  if (currentTime - lastFPSCalculation >= 1000) { // Every second
    currentFPS = (float)(frameCount - lastFrameCount) * 1000.0f / (currentTime - lastFPSCalculation);
    lastFPSCalculation = currentTime;
    lastFrameCount = frameCount;
  }
}

void handleLowPower() {
  // Reduce system performance in low power mode
  static unsigned long lastLowPowerCheck = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastLowPowerCheck > 10000) { // Every 10 seconds in low power
    Serial.println("[MAIN] Low power mode active");
    
    // Could reduce display brightness, CPU frequency, etc.
    displayManager.setBrightness(128); // Reduce brightness
    
    lastLowPowerCheck = currentTime;
  }
  
  // Add extra delay in low power mode
  delay(10);
}

// ========================================
// DEBUG INTERFACE
// ========================================

void handleSerialCommands() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toLowerCase();
  
  Serial.printf("[DEBUG] Command received: %s\n", command.c_str());
  
  if (command == "help") {
    Serial.println("Available commands:");
    Serial.println("  help - Show this help");
    Serial.println("  info - System information");
    Serial.println("  stats - Performance statistics");
    Serial.println("  apps - List registered apps");
    Serial.println("  memory - Memory usage");
    Serial.println("  touch - Touch interface status");
    Serial.println("  filesystem - FileSystem status and operations");
    Serial.println("  calibrate - Recalibrate touch");
    Serial.println("  reset - Restart system");
    Serial.println("  launcher - Return to launcher");
    
  } else if (command == "info") {
    printSystemInfo();
    
  } else if (command == "stats") {
    Serial.printf("FPS: %.1f\n", currentFPS);
    Serial.printf("Frame count: %lu\n", frameCount);
    Serial.printf("Uptime: %lu seconds\n", systemCore.getUptimeSeconds());
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Min free heap: %d bytes\n", minFreeHeap);
    
  } else if (command == "apps") {
    appManager.printAppRegistry();
    
  } else if (command == "memory") {
    appManager.printMemoryUsage();
    systemCore.dumpSystemStats();
    
  } else if (command == "touch") {
    touchInterface.printTouchInfo();
    touchInterface.printCalibrationInfo();
    
  } else if (command == "filesystem") {
    Serial.println("FileSystem status:");
    filesystem.printStats();
    if (filesystem.isReady()) {
      filesystem.printDirectoryTree("/", 3);
    }
    
  } else if (command == "calibrate") {
    Serial.println("Starting touch recalibration...");
    runTouchCalibration();
    
  } else if (command == "reset") {
    Serial.println("Restarting system...");
    delay(1000);
    ESP.restart();
    
  } else if (command == "launcher") {
    Serial.println("Returning to launcher...");
    appManager.returnToLauncher();
    
  } else {
    Serial.printf("Unknown command: %s (type 'help' for commands)\n", command.c_str());
  }
}

void printSystemInfo() {
  Serial.println();
  Serial.println("=== remu.ii System Information ===");
  Serial.printf("Version: 1.0\n");
  Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
  Serial.printf("ESP32 Chip: %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM: %s\n", psramFound() ? "Available" : "Not found");
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
  
  // System status
  Serial.printf("System Initialized: %s\n", systemInitialized ? "YES" : "NO");
  Serial.printf("System Error: %s\n", systemError ? "YES" : "NO");
  Serial.printf("Touch Calibrated: %s\n", touchInterface.isCalibrated() ? "YES" : "NO");
  Serial.printf("Low Power Mode: %s\n", lowPowerMode ? "YES" : "NO");
  
  // Hardware status
  Serial.printf("Display Resolution: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
  Serial.printf("Battery Level: %d%%\n", systemCore.getBatteryPercentage());
  Serial.printf("Battery Voltage: %.2fV\n", systemCore.getBatteryVoltage());
  Serial.printf("FileSystem Ready: %s\n", filesystem.isReady() ? "YES" : "NO");
  if (filesystem.isReady()) {
    Serial.printf("SD Card Space: %.1f MB free / %.1f MB total\n",
                  filesystem.getFreeSpace() / (1024.0 * 1024.0),
                  filesystem.getTotalSpace() / (1024.0 * 1024.0));
  }
  
  Serial.println("===================================");
  Serial.println();
}

// ========================================
// ARDUINO REQUIRED FUNCTIONS
// ========================================

// These functions are called by Arduino framework during system events

void yield() {
  // Allow other tasks to run
  systemCore.feedWatchdog();
}