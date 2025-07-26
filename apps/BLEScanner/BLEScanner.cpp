#include "BLEScanner.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <algorithm>
#include <cmath>

// Icon data for BLE Scanner (16x16, 1-bit per pixel)
const uint8_t ble_scanner_icon[32] = {
    0x00, 0x00, 0x01, 0x80, 0x03, 0xC0, 0x07, 0xE0,
    0x0F, 0xF0, 0x1D, 0xB8, 0x39, 0x9C, 0x71, 0x8E,
    0x71, 0x8E, 0x39, 0x9C, 0x1D, 0xB8, 0x0F, 0xF0,
    0x07, 0xE0, 0x03, 0xC0, 0x01, 0x80, 0x00, 0x00
};

// ========================================
// RSSI History Implementation
// ========================================

void RSSIHistory::updateStatistics() {
    if (values.empty()) return;
    
    // Calculate mean
    float sum = 0;
    for (int8_t val : values) {
        sum += val;
    }
    mean = sum / values.size();
    
    // Calculate variance
    float varianceSum = 0;
    for (int8_t val : values) {
        float diff = val - mean;
        varianceSum += diff * diff;
    }
    variance = varianceSum / values.size();
    standardDeviation = sqrt(variance);
    
    // Find min/max
    min = *std::min_element(values.begin(), values.end());
    max = *std::max_element(values.begin(), values.end());
    
    lastUpdated = millis();
}

bool RSSIHistory::isOutlier(int8_t rssi) const {
    if (values.size() < 3) return false;
    return abs(rssi - mean) > (2.0 * standardDeviation);
}

// ========================================
// BLE Device Info Helper Methods
// ========================================

String BLEDeviceInfo::getStatusString() const {
    String status = "";
    if (statusFlags & DEVICE_ACTIVE) status += "Active ";
    if (statusFlags & DEVICE_LABELED) status += "Labeled ";
    if (statusFlags & DEVICE_SUSPICIOUS) status += "Suspicious ";
    if (statusFlags & DEVICE_NEW) status += "New ";
    if (statusFlags & DEVICE_HIDDEN) status += "Hidden ";
    return status.isEmpty() ? "Unknown" : status;
}

String BLEDeviceInfo::getAnomalyString() const {
    String anomalyStr = "";
    if (anomalies & ANOMALY_NEW_DEVICE) anomalyStr += "New ";
    if (anomalies & ANOMALY_RSSI_SUDDEN_CHANGE) anomalyStr += "RSSI-Jump ";
    if (anomalies & ANOMALY_RSSI_OUTLIER) anomalyStr += "RSSI-Outlier ";
    if (anomalies & ANOMALY_MAC_RANDOMIZED) anomalyStr += "Random-MAC ";
    if (anomalies & ANOMALY_TIMING_IRREGULAR) anomalyStr += "Timing ";
    if (anomalies & ANOMALY_ENTROPY_HIGH) anomalyStr += "High-Entropy ";
    if (anomalies & ANOMALY_ENTROPY_LOW) anomalyStr += "Low-Entropy ";
    if (anomalies & ANOMALY_SIGNAL_SPOOFING) anomalyStr += "Spoofing ";
    if (anomalies & ANOMALY_RAPID_APPEARING) anomalyStr += "Rapid ";
    return anomalyStr.isEmpty() ? "None" : anomalyStr;
}

// ========================================
// BLE Scanner Implementation
// ========================================

BLEScanner::BLEScanner() : BaseApp() {
    // Initialize member variables
    pBLEScan = nullptr;
    bleInitialized = false;
    scanning = false;
    lastScanTime = 0;
    scanStartTime = 0;
    deviceCount = 0;
    entropyIndex = 0;
    lastAnomalyCheck = 0;
    lastLogWrite = 0;
    
    // Initialize colors
    colorNormal = COLOR_WHITE;
    colorLabeled = COLOR_GREEN;
    colorAnomaly = COLOR_RED;
    colorNew = COLOR_YELLOW;
    colorBackground = COLOR_BLACK;
    colorText = COLOR_WHITE;
    
    // Set app metadata
    metadata.name = "BLE Scanner";
    metadata.version = "1.0.0";
    metadata.author = "remu.ii";
    metadata.description = "Advanced BLE device scanner with anomaly detection";
    metadata.category = CATEGORY_TOOLS;
    metadata.icon = ble_scanner_icon;
    metadata.maxMemory = 65536; // 64KB
    metadata.requiresSD = true;
    metadata.requiresWiFi = false;
    metadata.requiresBLE = true;
    
    // Initialize entropy pool
    for (int i = 0; i < 256; i++) {
        entropyPool[i] = 0.0;
    }
    
    // Set file paths
    logFilePath = BLE_SCAN_LOG_FILE;
    labelFilePath = BLE_DEVICE_LABELS_FILE;
    configFilePath = BLE_CONFIG_FILE;
}

BLEScanner::~BLEScanner() {
    cleanup();
}

// ========================================
// BaseApp Interface Implementation
// ========================================

bool BLEScanner::initialize() {
    setState(APP_INITIALIZING);
    
    debugLog("BLEScanner: Initializing...");
    
    // Create app data directory
    if (!createAppDataDir()) {
        debugLog("BLEScanner: Failed to create data directory");
        setState(APP_ERROR);
        return false;
    }
    
    // Initialize filesystem logging
    initializeLogging();
    
    // Load configuration
    loadConfiguration();
    
    // Load device labels
    loadDeviceLabels();
    
    // Initialize BLE
    if (!initializeBLE()) {
        debugLog("BLEScanner: BLE initialization failed");
        setState(APP_ERROR);
        return false;
    }
    
    // Initialize UI state
    uiState.currentView = VIEW_DEVICE_LIST;
    uiState.selectedDevice = -1;
    uiState.scrollOffset = 0;
    uiState.scanningActive = false;
    uiState.showAnomalyAlert = false;
    
    // Start initial scan
    startScan();
    
    setState(APP_RUNNING);
    debugLog("BLEScanner: Initialization complete");
    return true;
}

void BLEScanner::update() {
    if (currentState != APP_RUNNING) return;
    
    unsigned long currentTime = millis();
    
    // Process scan results if scanning
    if (scanning) {
        processScanResults();
    }
    
    // Perform periodic anomaly detection
    if (config.enableAnomalyDetection && 
        (currentTime - lastAnomalyCheck) > 5000) { // Every 5 seconds
        performAnomalyDetection();
        lastAnomalyCheck = currentTime;
    }
    
    // Update statistics
    updateStatistics();
    
    // Clean up old devices
    if ((currentTime - lastScanTime) > 30000) { // Every 30 seconds
        cleanupOldDevices();
        lastScanTime = currentTime;
    }
    
    // Auto log to SD card
    if (config.logToSD && (currentTime - lastLogWrite) > 10000) { // Every 10 seconds
        for (auto& devicePair : devices) {
            if (devicePair.second.isActive()) {
                logScanEvent(devicePair.second, "ACTIVE");
            }
        }
        lastLogWrite = currentTime;
    }
    
    // Auto-restart scan if stopped
    if (!scanning && uiState.scanningActive && 
        (currentTime - scanStartTime) > config.scanDuration) {
        startScan();
    }
    
    frameCount++;
}

void BLEScanner::render() {
    if (currentState != APP_RUNNING) return;
    
    // Clear display
    DisplayManager& display = DisplayManager::getInstance();
    display.fillScreen(colorBackground);
    
    // Render header
    renderHeader();
    
    // Render current view
    switch (uiState.currentView) {
        case VIEW_DEVICE_LIST:
            renderDeviceList();
            break;
        case VIEW_DEVICE_DETAILS:
            renderDeviceDetails();
            break;
        case VIEW_ANOMALY_ALERTS:
            renderAnomalyAlerts();
            break;
        case VIEW_STATISTICS:
            renderStatistics();
            break;
        case VIEW_LABELING:
            renderLabelingInterface();
            break;
        case VIEW_LOGS:
            renderLogView();
            break;
    }
    
    // Render status bar
    renderStatusBar();
    
    // Show anomaly alert if needed
    if (uiState.showAnomalyAlert) {
        display.fillRect(10, 60, SCREEN_WIDTH - 20, 80, COLOR_RED);
        display.drawRect(10, 60, SCREEN_WIDTH - 20, 80, COLOR_WHITE);
        display.setTextColor(COLOR_WHITE);
        display.setCursor(15, 70);
        display.print("ANOMALY DETECTED!");
        display.setCursor(15, 85);
        display.print(uiState.alertMessage);
        display.setCursor(15, 115);
        display.print("Touch to dismiss");
    }
    
    uiState.lastUIUpdate = millis();
}

bool BLEScanner::handleTouch(TouchPoint touch) {
    if (currentState != APP_RUNNING) return false;
    
    // Handle anomaly alert dismissal
    if (uiState.showAnomalyAlert) {
        uiState.showAnomalyAlert = false;
        uiState.alertMessage = "";
        return true;
    }
    
    // Identify touch zone
    TouchZone zone = identifyTouchZone(touch);
    
    switch (zone) {
        case ZONE_BACK_BUTTON:
            if (uiState.currentView != VIEW_DEVICE_LIST) {
                uiState.currentView = VIEW_DEVICE_LIST;
                uiState.selectedDevice = -1;
            } else {
                exitApp();
            }
            return true;
            
        case ZONE_SCAN_TOGGLE:
            toggleScanning();
            return true;
            
        case ZONE_VIEW_TOGGLE:
            // Cycle through views
            switch (uiState.currentView) {
                case VIEW_DEVICE_LIST:
                    uiState.currentView = VIEW_STATISTICS;
                    break;
                case VIEW_STATISTICS:
                    uiState.currentView = VIEW_ANOMALY_ALERTS;
                    break;
                case VIEW_ANOMALY_ALERTS:
                    uiState.currentView = VIEW_LOGS;
                    break;
                case VIEW_LOGS:
                    uiState.currentView = VIEW_DEVICE_LIST;
                    break;
                default:
                    uiState.currentView = VIEW_DEVICE_LIST;
                    break;
            }
            return true;
            
        case ZONE_DEVICE_LIST:
            handleDeviceListTouch(touch);
            return true;
            
        case ZONE_LABEL_BUTTON:
            if (uiState.selectedDevice >= 0) {
                uiState.currentView = VIEW_LABELING;
            }
            return true;
            
        case ZONE_LOG_BUTTON:
            if (uiState.selectedDevice >= 0) {
                String macAddress = deviceOrder[uiState.selectedDevice];
                logScanEvent(devices[macAddress], "USER_MARKED");
            }
            return true;
            
        default:
            break;
    }
    
    // View-specific touch handling
    switch (uiState.currentView) {
        case VIEW_DEVICE_DETAILS:
            handleDeviceDetailsTouch(touch);
            break;
        case VIEW_LABELING:
            handleLabelingTouch(touch);
            break;
        default:
            break;
    }
    
    uiState.lastTouch = touch;
    uiState.lastTouchTime = millis();
    
    return false;
}

void BLEScanner::cleanup() {
    debugLog("BLEScanner: Cleaning up...");
    
    // Stop scanning
    stopScan();
    
    // Save state
    saveState();
    saveConfiguration();
    saveDeviceLabels();
    
    // Cleanup BLE
    if (bleInitialized && pBLEScan) {
        pBLEScan->stop();
        pBLEScan = nullptr;
    }
    
    // Clear device list
    devices.clear();
    deviceOrder.clear();
    anomalyEvents.clear();
    
    setState(APP_INACTIVE);
}

String BLEScanner::getName() const {
    return "BLE Scanner";
}

const uint8_t* BLEScanner::getIcon() const {
    return ble_scanner_icon;
}

// ========================================
// BLE Core Methods
// ========================================

bool BLEScanner::initializeBLE() {
    debugLog("BLEScanner: Initializing BLE...");
    
    try {
        BLEDevice::init("remu.ii-BLEScanner");
        pBLEScan = BLEDevice::getScan();
        
        if (!pBLEScan) {
            debugLog("BLEScanner: Failed to get BLE scan object");
            return false;
        }
        
        // Set scan parameters
        pBLEScan->setAdvertisedDeviceCallbacks(new BLEScanCallback(this));
        pBLEScan->setActiveScan(true);
        pBLEScan->setInterval(config.scanInterval);
        pBLEScan->setWindow(config.scanInterval - 1);
        
        bleInitialized = true;
        debugLog("BLEScanner: BLE initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        debugLog("BLEScanner: BLE initialization exception: " + String(e.what()));
        return false;
    }
}

void BLEScanner::startScan() {
    if (!bleInitialized || !pBLEScan) return;
    
    debugLog("BLEScanner: Starting BLE scan...");
    
    try {
        pBLEScan->start(config.scanDuration / 1000, false);
        scanning = true;
        uiState.scanningActive = true;
        scanStartTime = millis();
        
        stats.totalScanTime += config.scanDuration;
        
    } catch (const std::exception& e) {
        debugLog("BLEScanner: Scan start exception: " + String(e.what()));
        scanning = false;
        uiState.scanningActive = false;
    }
}

void BLEScanner::stopScan() {
    if (!bleInitialized || !pBLEScan || !scanning) return;
    
    debugLog("BLEScanner: Stopping BLE scan...");
    
    try {
        pBLEScan->stop();
        scanning = false;
        uiState.scanningActive = false;
        
    } catch (const std::exception& e) {
        debugLog("BLEScanner: Scan stop exception: " + String(e.what()));
    }
}

void BLEScanner::processScanResults() {
    if (!pBLEScan) return;
    
    BLEScanResults foundDevices = pBLEScan->getResults();
    int deviceCount = foundDevices.getCount();
    
    for (int i = 0; i < deviceCount; i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        updateDeviceInfo(device);
    }
    
    pBLEScan->clearResults();
}

void BLEScanner::updateDeviceInfo(BLEAdvertisedDevice advertisedDevice) {
    String macAddress = String(advertisedDevice.getAddress().toString().c_str());
    
    // Check if device already exists
    bool isNewDevice = (devices.find(macAddress) == devices.end());
    
    if (isNewDevice) {
        // Create new device entry
        BLEDeviceInfo newDevice;
        newDevice.macAddress = macAddress;
        newDevice.firstSeen = millis();
        newDevice.statusFlags = DEVICE_NEW | DEVICE_ACTIVE;
        newDevice.anomalies = ANOMALY_NEW_DEVICE;
        
        devices[macAddress] = newDevice;
        deviceOrder.push_back(macAddress);
        deviceCount++;
        
        stats.uniqueDevicesFound++;
        stats.totalDevicesFound++;
        
        // Log new device
        logScanEvent(newDevice, "NEW_DEVICE");
        
        // Create anomaly alert for new devices
        addAnomalyEvent(macAddress, ANOMALY_NEW_DEVICE, 
                       "New device discovered", 0.5);
    }
    
    // Update existing device info
    BLEDeviceInfo& device = devices[macAddress];
    
    // Update basic info
    if (advertisedDevice.haveName()) {
        device.deviceName = String(advertisedDevice.getName().c_str());
    }
    
    if (advertisedDevice.haveRSSI()) {
        int8_t newRSSI = advertisedDevice.getRSSI();
        
        // Check for RSSI anomalies
        if (device.rssiHistory.isOutlier(newRSSI)) {
            device.anomalies |= ANOMALY_RSSI_OUTLIER;
            addAnomalyEvent(macAddress, ANOMALY_RSSI_OUTLIER,
                           "RSSI outlier detected", 0.6);
        }
        
        // Check for sudden RSSI changes
        if (abs(newRSSI - device.rssi) > 20) {
            device.anomalies |= ANOMALY_RSSI_SUDDEN_CHANGE;
            addAnomalyEvent(macAddress, ANOMALY_RSSI_SUDDEN_CHANGE,
                           "Sudden RSSI change", 0.7);
        }
        
        device.rssi = newRSSI;
        device.rssiHistory.addValue(newRSSI);
    }
    
    // Update timestamps and counters
    device.lastSeen = millis();
    device.lastUpdate = millis();
    device.scanCount++;
    device.statusFlags |= DEVICE_ACTIVE;
    device.statusFlags &= ~DEVICE_TIMEOUT;
    
    // Track appearance times for timing analysis
    device.appearanceTimes.push_back(millis());
    if (device.appearanceTimes.size() > 20) {
        device.appearanceTimes.erase(device.appearanceTimes.begin());
    }
    
    // Update statistics
    stats.totalDevicesFound++;
}

// ========================================
// Anomaly Detection Methods
// ========================================

void BLEScanner::performAnomalyDetection() {
    for (auto& devicePair : devices) {
        BLEDeviceInfo& device = devicePair.second;
        
        if (!device.isActive()) continue;
        
        // Analyze RSSI anomalies
        analyzeRSSIAnomalies(device);
        
        // Analyze MAC randomization
        analyzeMACRandomization(device);
        
        // Analyze timing anomalies
        analyzeTimingAnomalies(device);
        
        // Analyze entropy patterns
        analyzeEntropyPattern(device);
    }
    
    // Detect signal spoofing across all devices
    detectSignalSpoofing();
}

void BLEScanner::analyzeRSSIAnomalies(BLEDeviceInfo& device) {
    if (device.rssiHistory.values.size() < 5) return;
    
    // Check for consistent outliers
    int outlierCount = 0;
    for (int8_t rssi : device.rssiHistory.values) {
        if (device.rssiHistory.isOutlier(rssi)) {
            outlierCount++;
        }
    }
    
    if (outlierCount > device.rssiHistory.values.size() / 2) {
        device.anomalies |= ANOMALY_RSSI_OUTLIER;
        addAnomalyEvent(device.macAddress, ANOMALY_RSSI_OUTLIER,
                       "Consistent RSSI anomalies detected", 0.8);
    }
    
    // Check for signal strength spoofing (unusually stable RSSI)
    if (device.rssiHistory.standardDeviation < 1.0 && 
        device.rssiHistory.values.size() > 10) {
        device.anomalies |= ANOMALY_SIGNAL_SPOOFING;
        addAnomalyEvent(device.macAddress, ANOMALY_SIGNAL_SPOOFING,
                       "Possible signal spoofing (too stable)", 0.9);
    }
}

void BLEScanner::analyzeMACRandomization(BLEDeviceInfo& device) {
    // Calculate MAC address entropy
    float macEntropy = calculateMACEntropy(device.macAddress);
    device.entropyScore = macEntropy;
    
    // Check for randomized MAC patterns
    String mac = device.macAddress;
    mac.replace(":", "");
    
    // Check for common randomization patterns
    bool hasRandomPattern = false;
    
    // Check for locally administered bit (2nd bit of first octet)
    if (mac.length() >= 2) {
        int firstOctet = strtol(mac.substring(0, 2).c_str(), NULL, 16);
        if (firstOctet & 0x02) {
            hasRandomPattern = true;
        }
    }
    
    // High entropy indicates randomization
    if (macEntropy > 0.85) {
        device.anomalies |= ANOMALY_MAC_RANDOMIZED;
        device.isMacRandomized = true;
        addAnomalyEvent(device.macAddress, ANOMALY_MAC_RANDOMIZED,
                       "Randomized MAC address detected", 0.4);
    }
    
    // Low entropy might indicate spoofing
    if (macEntropy < 0.3) {
        device.anomalies |= ANOMALY_ENTROPY_LOW;
        addAnomalyEvent(device.macAddress, ANOMALY_ENTROPY_LOW,
                       "Unusually low MAC entropy", 0.6);
    }
}

void BLEScanner::analyzeTimingAnomalies(BLEDeviceInfo& device) {
    if (device.appearanceTimes.size() < 5) return;
    
    // Calculate intervals between appearances
    std::vector<unsigned long> intervals;
    for (size_t i = 1; i < device.appearanceTimes.size(); i++) {
        intervals.push_back(device.appearanceTimes[i] - device.appearanceTimes[i-1]);
    }
    
    // Calculate mean and variance of intervals
    unsigned long sum = 0;
    for (unsigned long interval : intervals) {
        sum += interval;
    }
    float meanInterval = sum / (float)intervals.size();
    
    float varianceSum = 0;
    for (unsigned long interval : intervals) {
        float diff = interval - meanInterval;
        varianceSum += diff * diff;
    }
    float variance = varianceSum / intervals.size();
    float stdDev = sqrt(variance);
    
    // Check for timing irregularities
    if (stdDev > meanInterval * 0.5) { // High variance in timing
        device.anomalies |= ANOMALY_TIMING_IRREGULAR;
        addAnomalyEvent(device.macAddress, ANOMALY_TIMING_IRREGULAR,
                       "Irregular appearance timing", 0.5);
    }
    
    // Check for rapid appearing/disappearing
    int rapidCount = 0;
    for (unsigned long interval : intervals) {
        if (interval < 1000) { // Less than 1 second
            rapidCount++;
        }
    }
    
    if (rapidCount > intervals.size() / 2) {
        device.anomalies |= ANOMALY_RAPID_APPEARING;
        addAnomalyEvent(device.macAddress, ANOMALY_RAPID_APPEARING,
                       "Rapid appearing/disappearing pattern", 0.7);
    }
}

void BLEScanner::analyzeEntropyPattern(BLEDeviceInfo& device) {
    // Update entropy pool with device data
    uint8_t macBytes[6];
    sscanf(device.macAddress.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &macBytes[0], &macBytes[1], &macBytes[2], 
           &macBytes[3], &macBytes[4], &macBytes[5]);
    
    for (int i = 0; i < 6; i++) {
        entropyPool[entropyIndex] = macBytes[i] / 255.0;
        entropyIndex = (entropyIndex + 1) % 256;
    }
    
    // Calculate entropy of recent pool data
    std::vector<uint8_t> recentData;
    for (int i = 0; i < 64; i++) {
        int idx = (entropyIndex - i + 256) % 256;
        recentData.push_back((uint8_t)(entropyPool[idx] * 255));
    }
    
    float entropy = calculateEntropy(recentData);
    
    // Check for entropy anomalies
    if (entropy > 0.95) {
        device.anomalies |= ANOMALY_ENTROPY_HIGH;
        addAnomalyEvent(device.macAddress, ANOMALY_ENTROPY_HIGH,
                       "High entropy pattern detected", 0.6);
    } else if (entropy < 0.1) {
        device.anomalies |= ANOMALY_ENTROPY_LOW;
        addAnomalyEvent(device.macAddress, ANOMALY_ENTROPY_LOW,
                       "Low entropy pattern detected", 0.6);
    }
}

float BLEScanner::calculateEntropy(const std::vector<uint8_t>& data) {
    if (data.empty()) return 0.0;
    
    // Count frequency of each byte value
    int freq[256] = {0};
    for (uint8_t byte : data) {
        freq[byte]++;
    }
    
    // Calculate Shannon entropy
    float entropy = 0.0;
    int total = data.size();
    
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            float probability = (float)freq[i] / total;
            entropy -= probability * log2(probability);
        }
    }
    
    return entropy / 8.0; // Normalize to 0-1 range
}

float BLEScanner::calculateMACEntropy(const String& macAddress) {
    String cleanMAC = macAddress;
    cleanMAC.replace(":", "");
    
    std::vector<uint8_t> macBytes;
    for (int i = 0; i < cleanMAC.length(); i += 2) {
        String byteStr = cleanMAC.substring(i, i + 2);
        uint8_t byte = strtol(byteStr.c_str(), NULL, 16);
        macBytes.push_back(byte);
    }
    
    return calculateEntropy(macBytes);
}

void BLEScanner::detectSignalSpoofing() {
    // Look for devices with identical or suspiciously similar characteristics
    for (auto& device1 : devices) {
        for (auto& device2 : devices) {
            if (device1.first == device2.first) continue;
            
            BLEDeviceInfo& dev1 = device1.second;
            BLEDeviceInfo& dev2 = device2.second;
            
            // Check for identical device names but different MACs
            if (!dev1.deviceName.isEmpty() && 
                dev1.deviceName == dev2.deviceName &&
                dev1.macAddress != dev2.macAddress) {
                
                // Check if RSSI values are suspiciously similar
                if (abs(dev1.rssi - dev2.rssi) < 3) {
                    dev1.anomalies |= ANOMALY_SIGNAL_SPOOFING;
                    dev2.anomalies |= ANOMALY_SIGNAL_SPOOFING;
                    
                    addAnomalyEvent(dev1.macAddress, ANOMALY_SIGNAL_SPOOFING,
                                   "Possible spoofing: identical name/RSSI", 0.9);
                }
            }
        }
    }
}

void BLEScanner::addAnomalyEvent(const String& macAddress, AnomalyType type,
                                const String& description, float severity) {
    AnomalyEvent event(macAddress, type, description, severity);
    event.details = devices[macAddress].getStatusString();
    
    anomalyEvents.push_back(event);
    
    // Limit anomaly event history
    if (anomalyEvents.size() > 100) {
        anomalyEvents.erase(anomalyEvents.begin());
    }
    
    // Log anomaly event
    logAnomalyEvent(event);
    
    // Show alert for high severity anomalies
    if (severity > 0.7) {
        uiState.showAnomalyAlert = true;
        uiState.alertMessage = description + " (" + macAddress + ")";
    }
    
    stats.anomaliesDetected++;
}

// ========================================
// Device Labeling Methods
// ========================================

void BLEScanner::loadDeviceLabels() {
    if (!filesystem.fileExists(labelFilePath)) {
        debugLog("BLEScanner: No existing labels file");
        return;
    }
    
    String content = filesystem.readFile(labelFilePath);
    if (content.isEmpty()) {
        debugLog("BLEScanner: Empty labels file");
        return;
    }
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, content);
    
    if (error) {
        debugLog("BLEScanner: Failed to parse labels JSON: " + String(error.c_str()));
        return;
    }
    
    JsonObject labels = doc.as<JsonObject>();
    for (JsonPair kv : labels) {
        String macAddress = kv.key().c_str();
        String label = kv.value().as<String>();
        
        if (devices.find(macAddress) != devices.end()) {
            devices[macAddress].label = label;
            devices[macAddress].statusFlags |= DEVICE_LABELED;
        }
    }
    
    debugLog("BLEScanner: Loaded device labels");
}

void BLEScanner::saveDeviceLabels() {
    DynamicJsonDocument doc(4096);
    JsonObject labels = doc.to<JsonObject>();
    
    for (auto& devicePair : devices) {
        if (!devicePair.second.label.isEmpty()) {
            labels[devicePair.first] = devicePair.second.label;
        }
    }
    
    String content;
    serializeJsonPretty(doc, content);
    
    if (filesystem.writeFile(labelFilePath, content)) {
        debugLog("BLEScanner: Saved device labels");
    } else {
        debugLog("BLEScanner: Failed to save device labels");
    }
}

void BLEScanner::labelDevice(const String& macAddress, const String& label) {
    if (devices.find(macAddress) != devices.end()) {
        devices[macAddress].label = label;
        devices[macAddress].statusFlags |= DEVICE_LABELED;
        
        logScanEvent(devices[macAddress], "LABELED");
        saveDeviceLabels();
        
        stats.labeledDevices++;
    }
}

void BLEScanner::removeLabelFromDevice(const String& macAddress) {
    if (devices.find(macAddress) != devices.end()) {
        devices[macAddress].label = "";
        devices[macAddress].statusFlags &= ~DEVICE_LABELED;
        
        logScanEvent(devices[macAddress], "LABEL_REMOVED");
        saveDeviceLabels();
        
        if (stats.labeledDevices > 0) {
            stats.labeledDevices--;
        }
    }
}

String BLEScanner::generateAutoLabel(const BLEDeviceInfo& device) {
    String autoLabel = "";
    
    if (!device.deviceName.isEmpty()) {
        autoLabel = device.deviceName;
    } else {
        // Generate label based on MAC address pattern
        String mac = device.macAddress.substring(0, 8);
        autoLabel = "Device-" + mac;
    }
    
    // Add suffix for randomized MACs
    if (device.isMacRandomized) {
        autoLabel += " (Random)";
    }
    
    return autoLabel;
}

// ========================================
// Data Logging Methods
// ========================================

void BLEScanner::initializeLogging() {
    // Ensure log directories exist
    filesystem.ensureDirExists(LOGS_DIR);
    filesystem.ensureDirExists(BLE_SCANNER_DATA_DIR);
    
    // Rotate logs if needed
    rotateLogs();
    
    debugLog("BLEScanner: Logging initialized");
}

void BLEScanner::logScanEvent(const BLEDeviceInfo& device, const String& event) {
    if (!config.logToSD) return;
    
    String logEntry = formatLogEntry(device, event);
    filesystem.appendFile(logFilePath, logEntry + "\n");
}

void BLEScanner::logAnomalyEvent(const AnomalyEvent& event) {
    if (!config.logToSD) return;
    
    String logEntry = String(event.timestamp) + "," +
                     event.macAddress + "," +
                     String((int)event.type) + "," +
                     event.description + "," +
                     String(event.severity, 2) + "," +
                     event.details;
    
    filesystem.appendFile(BLE_ANOMALY_LOG_FILE, logEntry + "\n");
}

void BLEScanner::exportLogData(const String& format) {
    String exportPath = BLE_SCANNER_DATA_DIR "/export_" + String(millis()) + "." + format;
    
    if (format == "json") {
        // Export as JSON
        DynamicJsonDocument doc(8192);
        JsonArray deviceArray = doc.createNestedArray("devices");
        
        for (auto& devicePair : devices) {
            JsonObject deviceObj = deviceArray.createNestedObject();
            const BLEDeviceInfo& device = devicePair.second;
            
            deviceObj["macAddress"] = device.macAddress;
            deviceObj["deviceName"] = device.deviceName;
            deviceObj["label"] = device.label;
            deviceObj["rssi"] = device.rssi;
            deviceObj["firstSeen"] = device.firstSeen;
            deviceObj["lastSeen"] = device.lastSeen;
            deviceObj["scanCount"] = device.scanCount;
            deviceObj["anomalies"] = device.anomalies;
            deviceObj["entropyScore"] = device.entropyScore;
            deviceObj["isMacRandomized"] = device.isMacRandomized;
        }
        
        JsonArray anomalyArray = doc.createNestedArray("anomalies");
        for (const AnomalyEvent& event : anomalyEvents) {
            JsonObject anomalyObj = anomalyArray.createNestedObject();
            anomalyObj["timestamp"] = event.timestamp;
            anomalyObj["macAddress"] = event.macAddress;
            anomalyObj["type"] = (int)event.type;
            anomalyObj["description"] = event.description;
            anomalyObj["severity"] = event.severity;
        }
        
        String content;
        serializeJsonPretty(doc, content);
        filesystem.writeFile(exportPath, content);
        
    } else if (format == "csv") {
        // Export as CSV
        String csvContent = "MAC Address,Device Name,Label,RSSI,First Seen,Last Seen,Scan Count,Anomalies,Entropy Score\n";
        
        for (auto& devicePair : devices) {
            const BLEDeviceInfo& device = devicePair.second;
            csvContent += device.macAddress + "," +
                         device.deviceName + "," +
                         device.label + "," +
                         String(device.rssi) + "," +
                         String(device.firstSeen) + "," +
                         String(device.lastSeen) + "," +
                         String(device.scanCount) + "," +
                         String(device.anomalies) + "," +
                         String(device.entropyScore, 3) + "\n";
        }
        
        filesystem.writeFile(exportPath, csvContent);
    }
    
    debugLog("BLEScanner: Exported data to " + exportPath);
}

void BLEScanner::rotateLogs() {
    // Check log file sizes and rotate if needed
    if (filesystem.getFileSize(logFilePath) > MAX_LOG_FILE_SIZE) {
        String backupPath = logFilePath + ".old";
        filesystem.renameFile(logFilePath, backupPath);
    }
    
    if (filesystem.getFileSize(BLE_ANOMALY_LOG_FILE) > MAX_LOG_FILE_SIZE) {
        String backupPath = BLE_ANOMALY_LOG_FILE + ".old";
        filesystem.renameFile(BLE_ANOMALY_LOG_FILE, backupPath);
    }
}

String BLEScanner::formatLogEntry(const BLEDeviceInfo& device, const String& event) {
    return String(millis()) + "," +
           device.macAddress + "," +
           device.deviceName + "," +
           device.label + "," +
           String(device.rssi) + "," +
           event + "," +
           String(device.anomalies);
}

// ========================================
// Statistics Methods
// ========================================

void BLEScanner::updateStatistics() {
    stats.uniqueDevicesFound = devices.size();
    
    // Count labeled devices
    stats.labeledDevices = 0;
    float rssiSum = 0;
    int activeDevices = 0;
    
    for (auto& devicePair : devices) {
        const BLEDeviceInfo& device = devicePair.second;
        
        if (device.isLabeled()) {
            stats.labeledDevices++;
        }
        
        if (device.isActive()) {
            rssiSum += device.rssi;
            activeDevices++;
        }
    }
    
    if (activeDevices > 0) {
        stats.averageRSSI = rssiSum / activeDevices;
    }
    
    // Calculate entropy mean
    float entropySum = 0;
    int entropyCount = 0;
    for (int i = 0; i < 256; i++) {
        if (entropyPool[i] > 0) {
            entropySum += entropyPool[i];
            entropyCount++;
        }
    }
    
    if (entropyCount > 0) {
        stats.entropyMean = entropySum / entropyCount;
    }
}

void BLEScanner::resetStatistics() {
    stats = ScanStatistics();
    stats.lastReset = millis();
}

String BLEScanner::generateStatsReport() {
    String report = "=== BLE Scanner Statistics ===\n";
    report += "Total Devices Found: " + String(stats.totalDevicesFound) + "\n";
    report += "Unique Devices: " + String(stats.uniqueDevicesFound) + "\n";
    report += "Labeled Devices: " + String(stats.labeledDevices) + "\n";
    report += "Anomalies Detected: " + String(stats.anomaliesDetected) + "\n";
    report += "Average RSSI: " + String(stats.averageRSSI, 1) + " dBm\n";
    report += "Entropy Mean: " + String(stats.entropyMean, 3) + "\n";
    report += "Total Scan Time: " + formatDuration(stats.totalScanTime) + "\n";
    report += "Runtime: " + formatDuration(getRunTime()) + "\n";
    
    return report;
}

// ========================================
// UI Rendering Methods
// ========================================

void BLEScanner::renderHeader() {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Draw header background
    display.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_GRAY_DARK);
    
    // Draw title
    display.setTextColor(COLOR_WHITE);
    display.setTextSize(1);
    display.setCursor(5, 5);
    display.print("BLE Scanner");
    
    // Draw scan status
    display.setCursor(SCREEN_WIDTH - 60, 5);
    if (scanning) {
        display.setTextColor(COLOR_GREEN);
        display.print("SCAN");
    } else {
        display.setTextColor(COLOR_RED);
        display.print("IDLE");
    }
    
    // Draw view indicator
    display.setCursor(SCREEN_WIDTH - 120, 5);
    display.setTextColor(COLOR_WHITE);
    switch (uiState.currentView) {
        case VIEW_DEVICE_LIST: display.print("LIST"); break;
        case VIEW_DEVICE_DETAILS: display.print("DETAIL"); break;
        case VIEW_ANOMALY_ALERTS: display.print("ALERT"); break;
        case VIEW_STATISTICS: display.print("STATS"); break;
        case VIEW_LABELING: display.print("LABEL"); break;
        case VIEW_LOGS: display.print("LOGS"); break;
    }
}

void BLEScanner::renderStatusBar() {
    DisplayManager& display = DisplayManager::getInstance();
    
    int statusY = SCREEN_HEIGHT - STATUS_BAR_HEIGHT;
    display.fillRect(0, statusY, SCREEN_WIDTH, STATUS_BAR_HEIGHT, COLOR_GRAY_DARK);
    
    // Device count
    display.setTextColor(COLOR_WHITE);
    display.setTextSize(1);
    display.setCursor(5, statusY + 2);
    display.print("Dev: " + String(deviceCount));
    
    // Anomaly count
    display.setCursor(60, statusY + 2);
    if (anomalyEvents.size() > 0) {
        display.setTextColor(COLOR_RED);
        display.print("Anom: " + String(anomalyEvents.size()));
    } else {
        display.setTextColor(COLOR_GREEN);
        display.print("No Anom");
    }
    
    // Memory usage
    display.setCursor(SCREEN_WIDTH - 80, statusY + 2);
    display.setTextColor(COLOR_WHITE);
    size_t memUsage = getMemoryUsage();
    display.print("Mem: " + String(memUsage / 1024) + "K");
    
    // Frame rate
    display.setCursor(SCREEN_WIDTH - 40, statusY + 2);
    display.print(String(getFPS(), 0) + "fps");
}

void BLEScanner::renderDeviceList() {
    DisplayManager& display = DisplayManager::getInstance();
    
    int listY = HEADER_HEIGHT + 5;
    int maxVisible = DEVICE_LIST_MAX_VISIBLE;
    int startIndex = uiState.scrollOffset;
    int endIndex = min(startIndex + maxVisible, (int)deviceOrder.size());
    
    // Draw device entries
    for (int i = startIndex; i < endIndex; i++) {
        int y = listY + (i - startIndex) * DEVICE_LIST_ITEM_HEIGHT;
        String macAddress = deviceOrder[i];
        const BLEDeviceInfo& device = devices[macAddress];
        
        bool selected = (i == uiState.selectedDevice);
        drawDeviceEntry(y, device, selected);
    }
    
    // Draw scrollbar
    if (deviceOrder.size() > maxVisible) {
        renderScrollbar();
    }
    
    // Draw controls
    int controlY = SCREEN_HEIGHT - STATUS_BAR_HEIGHT - 25;
    
    // Scan toggle button
    display.fillRect(5, controlY, 60, 20, scanning ? COLOR_GREEN : COLOR_RED);
    display.drawRect(5, controlY, 60, 20, COLOR_WHITE);
    display.setTextColor(COLOR_WHITE);
    display.setCursor(10, controlY + 6);
    display.print(scanning ? "STOP" : "START");
    
    // View toggle button
    display.fillRect(75, controlY, 60, 20, COLOR_BLUE);
    display.drawRect(75, controlY, 60, 20, COLOR_WHITE);
    display.setCursor(85, controlY + 6);
    display.print("VIEW");
    
    // Clear button
    display.fillRect(145, controlY, 60, 20, COLOR_YELLOW);
    display.drawRect(145, controlY, 60, 20, COLOR_BLACK);
    display.setTextColor(COLOR_BLACK);
    display.setCursor(155, controlY + 6);
    display.print("CLEAR");
}

void BLEScanner::renderDeviceDetails() {
    if (uiState.selectedDevice < 0 || uiState.selectedDevice >= deviceOrder.size()) {
        uiState.currentView = VIEW_DEVICE_LIST;
        return;
    }
    
    DisplayManager& display = DisplayManager::getInstance();
    String macAddress = deviceOrder[uiState.selectedDevice];
    const BLEDeviceInfo& device = devices[macAddress];
    
    int y = HEADER_HEIGHT + 10;
    display.setTextColor(COLOR_WHITE);
    display.setTextSize(1);
    
    // MAC Address
    display.setCursor(5, y);
    display.print("MAC: " + device.macAddress);
    y += 15;
    
    // Device Name
    display.setCursor(5, y);
    display.print("Name: " + (device.deviceName.isEmpty() ? "Unknown" : device.deviceName));
    y += 15;
    
    // Label
    display.setCursor(5, y);
    display.setTextColor(device.isLabeled() ? COLOR_GREEN : COLOR_GRAY_LIGHT);
    display.print("Label: " + (device.label.isEmpty() ? "None" : device.label));
    y += 15;
    
    // RSSI
    display.setCursor(5, y);
    display.setTextColor(device.rssi > -50 ? COLOR_GREEN : 
                        device.rssi > -70 ? COLOR_YELLOW : COLOR_RED);
    display.print("RSSI: " + formatRSSI(device.rssi));
    y += 15;
    
    // Statistics
    display.setTextColor(COLOR_WHITE);
    display.setCursor(5, y);
    display.print("Seen: " + String(device.scanCount) + " times");
    y += 15;
    
    display.setCursor(5, y);
    display.print("First: " + formatTime(device.firstSeen));
    y += 15;
    
    display.setCursor(5, y);
    display.print("Last: " + formatTime(device.lastSeen));
    y += 15;
    
    // Anomalies
    display.setCursor(5, y);
    display.setTextColor(device.hasAnomalies() ? COLOR_RED : COLOR_GREEN);
    display.print("Anomalies: " + device.getAnomalyString());
    y += 15;
    
    // RSSI History
    if (device.rssiHistory.values.size() > 1) {
        display.setTextColor(COLOR_WHITE);
        display.setCursor(5, y);
        display.print("RSSI Stats:");
        y += 12;
        
        display.setCursor(10, y);
        display.print("Mean: " + String(device.rssiHistory.mean, 1));
        y += 12;
        
        display.setCursor(10, y);
        display.print("StdDev: " + String(device.rssiHistory.standardDeviation, 1));
        y += 12;
        
        display.setCursor(10, y);
        display.print("Range: " + String(device.rssiHistory.min) + 
                     " to " + String(device.rssiHistory.max));
        y += 15;
    }
    
    // Action buttons
    int buttonY = SCREEN_HEIGHT - STATUS_BAR_HEIGHT - 30;
    
    // Label button
    display.fillRect(5, buttonY, 60, 20, COLOR_GREEN);
    display.drawRect(5, buttonY, 60, 20, COLOR_WHITE);
    display.setTextColor(COLOR_WHITE);
    display.setCursor(15, buttonY + 6);
    display.print("LABEL");
    
    // Log button
    display.fillRect(75, buttonY, 60, 20, COLOR_BLUE);
    display.drawRect(75, buttonY, 60, 20, COLOR_WHITE);
    display.setCursor(90, buttonY + 6);
    display.print("LOG");
    
    // Back button
    display.fillRect(SCREEN_WIDTH - 65, buttonY, 60, 20, COLOR_GRAY_DARK);
    display.drawRect(SCREEN_WIDTH - 65, buttonY, 60, 20, COLOR_WHITE);
    display.setCursor(SCREEN_WIDTH - 55, buttonY + 6);
    display.print("BACK");
}

void BLEScanner::renderAnomalyAlerts() {
    DisplayManager& display = DisplayManager::getInstance();
    
    int y = HEADER_HEIGHT + 10;
    display.setTextSize(1);
    
    if (anomalyEvents.empty()) {
        display.setTextColor(COLOR_GREEN);
        display.setCursor(5, y);
        display.print("No anomalies detected");
        return;
    }
    
    // Show recent anomalies
    int maxShow = min(8, (int)anomalyEvents.size());
    for (int i = anomalyEvents.size() - maxShow; i < anomalyEvents.size(); i++) {
        const AnomalyEvent& event = anomalyEvents[i];
        
        // Color based on severity
        uint16_t color = event.severity > 0.8 ? COLOR_RED :
                        event.severity > 0.5 ? COLOR_YELLOW : COLOR_WHITE;
        
        display.setTextColor(color);
        display.setCursor(5, y);
        display.print(formatTime(event.timestamp));
        y += 12;
        
        display.setCursor(10, y);
        display.print(event.macAddress.substring(0, 17)); // Show MAC
        y += 12;
        
        display.setCursor(10, y);
        display.print(event.description);
        y += 15;
        
        if (y > SCREEN_HEIGHT - STATUS_BAR_HEIGHT - 40) break;
    }
}

void BLEScanner::renderStatistics() {
    DisplayManager& display = DisplayManager::getInstance();
    
    int y = HEADER_HEIGHT + 10;
    display.setTextColor(COLOR_WHITE);
    display.setTextSize(1);
    
    // Device statistics
    display.setCursor(5, y);
    display.print("=== Device Stats ===");
    y += 15;
    
    display.setCursor(5, y);
    display.print("Total Found: " + String(stats.totalDevicesFound));
    y += 12;
    
    display.setCursor(5, y);
    display.print("Unique: " + String(stats.uniqueDevicesFound));
    y += 12;
    
    display.setCursor(5, y);
    display.setTextColor(COLOR_GREEN);
    display.print("Labeled: " + String(stats.labeledDevices));
    y += 12;
    
    display.setTextColor(COLOR_WHITE);
    display.setCursor(5, y);
    display.print("Avg RSSI: " + String(stats.averageRSSI, 1) + " dBm");
    y += 15;
    
    // Anomaly statistics
    display.setCursor(5, y);
    display.print("=== Anomaly Stats ===");
    y += 15;
    
    display.setCursor(5, y);
    display.setTextColor(stats.anomaliesDetected > 0 ? COLOR_RED : COLOR_GREEN);
    display.print("Total: " + String(stats.anomaliesDetected));
    y += 12;
    
    display.setTextColor(COLOR_WHITE);
    display.setCursor(5, y);
    display.print("Entropy: " + String(stats.entropyMean, 3));
    y += 15;
    
    // System statistics
    display.setCursor(5, y);
    display.print("=== System Stats ===");
    y += 15;
    
    display.setCursor(5, y);
    display.print("Runtime: " + formatDuration(getRunTime()));
    y += 12;
    
    display.setCursor(5, y);
    display.print("Scan Time: " + formatDuration(stats.totalScanTime));
    y += 12;
    
    display.setCursor(5, y);
    display.print("Memory: " + String(getMemoryUsage() / 1024) + "KB");
    y += 12;
    
    display.setCursor(5, y);
    display.print("FPS: " + String(getFPS(), 1));
}

void BLEScanner::renderLabelingInterface() {
    DisplayManager& display = DisplayManager::getInstance();
    
    if (uiState.selectedDevice < 0 || uiState.selectedDevice >= deviceOrder.size()) {
        uiState.currentView = VIEW_DEVICE_DETAILS;
        return;
    }
    
    String macAddress = deviceOrder[uiState.selectedDevice];
    const BLEDeviceInfo& device = devices[macAddress];
    
    int y = HEADER_HEIGHT + 10;
    display.setTextColor(COLOR_WHITE);
    display.setTextSize(1);
    
    display.setCursor(5, y);
    display.print("Label Device:");
    y += 15;
    
    display.setCursor(5, y);
    display.print("MAC: " + device.macAddress);
    y += 15;
    
    display.setCursor(5, y);
    display.print("Name: " + (device.deviceName.isEmpty() ? "Unknown" : device.deviceName));
    y += 20;
    
    display.setCursor(5, y);
    display.print("Current Label:");
    y += 12;
    
    display.setTextColor(device.isLabeled() ? COLOR_GREEN : COLOR_GRAY_LIGHT);
    display.setCursor(10, y);
    display.print(device.label.isEmpty() ? "None" : device.label);
    y += 20;
    
    // Suggested labels
    display.setTextColor(COLOR_WHITE);
    display.setCursor(5, y);
    display.print("Suggestions:");
    y += 15;
    
    String autoLabel = generateAutoLabel(device);
    display.setTextColor(COLOR_YELLOW);
    display.setCursor(10, y);
    display.print("1. " + autoLabel);
    y += 12;
    
    display.setCursor(10, y);
    display.print("2. My Device");
    y += 12;
    
    display.setCursor(10, y);
    display.print("3. Phone");
    y += 12;
    
    display.setCursor(10, y);
    display.print("4. Laptop");
    y += 20;
    
    // Action buttons
    int buttonY = SCREEN_HEIGHT - STATUS_BAR_HEIGHT - 50;
    
    // Apply auto label
    display.fillRect(5, buttonY, 80, 20, COLOR_GREEN);
    display.drawRect(5, buttonY, 80, 20, COLOR_WHITE);
    display.setTextColor(COLOR_WHITE);
    display.setCursor(15, buttonY + 6);
    display.print("AUTO");
    
    // Remove label
    if (device.isLabeled()) {
        display.fillRect(95, buttonY, 80, 20, COLOR_RED);
        display.drawRect(95, buttonY, 80, 20, COLOR_WHITE);
        display.setCursor(115, buttonY + 6);
        display.print("REMOVE");
    }
    
    // Back button
    buttonY += 25;
    display.fillRect(5, buttonY, 60, 20, COLOR_GRAY_DARK);
    display.drawRect(5, buttonY, 60, 20, COLOR_WHITE);
    display.setCursor(15, buttonY + 6);
    display.print("BACK");
}

void BLEScanner::renderLogView() {
    DisplayManager& display = DisplayManager::getInstance();
    
    int y = HEADER_HEIGHT + 10;
    display.setTextColor(COLOR_WHITE);
    display.setTextSize(1);
    
    display.setCursor(5, y);
    display.print("=== Recent Logs ===");
    y += 15;
    
    // Show recent log entries
    String logContent = filesystem.readFile(logFilePath);
    if (logContent.isEmpty()) {
        display.setTextColor(COLOR_GRAY_LIGHT);
        display.setCursor(5, y);
        display.print("No log entries");
        return;
    }
    
    // Parse and show last few entries
    int lineCount = 0;
    int lastNewline = logContent.lastIndexOf('\n');
    while (lastNewline > 0 && lineCount < 8) {
        int prevNewline = logContent.lastIndexOf('\n', lastNewline - 1);
        String line = logContent.substring(prevNewline + 1, lastNewline);
        
        if (!line.isEmpty()) {
            // Parse CSV line
            int comma1 = line.indexOf(',');
            int comma2 = line.indexOf(',', comma1 + 1);
            int comma3 = line.indexOf(',', comma2 + 1);
            
            if (comma1 > 0 && comma2 > comma1 && comma3 > comma2) {
                String timestamp = line.substring(0, comma1);
                String mac = line.substring(comma1 + 1, comma2);
                String event = line.substring(comma2 + 1, comma3);
                
                display.setTextColor(COLOR_WHITE);
                display.setCursor(5, y);
                display.print(formatTime(timestamp.toInt()));
                y += 10;
                
                display.setTextColor(COLOR_CYAN);
                display.setCursor(10, y);
                display.print(mac.substring(0, 17));
                y += 10;
                
                display.setTextColor(COLOR_YELLOW);
                display.setCursor(10, y);
                display.print(event);
                y += 15;
                
                lineCount++;
            }
        }
        
        lastNewline = prevNewline;
        if (y > SCREEN_HEIGHT - STATUS_BAR_HEIGHT - 30) break;
    }
    
    // Export button
    int buttonY = SCREEN_HEIGHT - STATUS_BAR_HEIGHT - 25;
    display.fillRect(5, buttonY, 80, 20, COLOR_BLUE);
    display.drawRect(5, buttonY, 80, 20, COLOR_WHITE);
    display.setTextColor(COLOR_WHITE);
    display.setCursor(15, buttonY + 6);
    display.print("EXPORT");
}

void BLEScanner::renderScrollbar() {
    if (deviceOrder.size() <= DEVICE_LIST_MAX_VISIBLE) return;
    
    DisplayManager& display = DisplayManager::getInstance();
    
    int scrollBarX = SCREEN_WIDTH - SCROLL_BAR_WIDTH - 2;
    int scrollBarY = HEADER_HEIGHT + 5;
    int scrollBarHeight = DEVICE_LIST_MAX_VISIBLE * DEVICE_LIST_ITEM_HEIGHT;
    
    // Draw scrollbar background
    display.drawRect(scrollBarX, scrollBarY, SCROLL_BAR_WIDTH, scrollBarHeight, COLOR_GRAY_LIGHT);
    
    // Calculate thumb position and size
    int totalItems = deviceOrder.size();
    int thumbHeight = max(10, scrollBarHeight * DEVICE_LIST_MAX_VISIBLE / totalItems);
    int thumbY = scrollBarY + (scrollBarHeight - thumbHeight) * uiState.scrollOffset / 
                (totalItems - DEVICE_LIST_MAX_VISIBLE);
    
    // Draw scrollbar thumb
    display.fillRect(scrollBarX + 1, thumbY, SCROLL_BAR_WIDTH - 2, thumbHeight, COLOR_WHITE);
}

// ========================================
// UI Helper Methods
// ========================================

void BLEScanner::drawDeviceEntry(int y, const BLEDeviceInfo& device, bool selected) {
    DisplayManager& display = DisplayManager::getInstance();
    
    // Background
    uint16_t bgColor = selected ? COLOR_GRAY_DARK : colorBackground;
    display.fillRect(0, y, SCREEN_WIDTH - SCROLL_BAR_WIDTH - 5, DEVICE_LIST_ITEM_HEIGHT, bgColor);
    
    if (selected) {
        display.drawRect(0, y, SCREEN_WIDTH - SCROLL_BAR_WIDTH - 5, DEVICE_LIST_ITEM_HEIGHT, COLOR_WHITE);
    }
    
    // Device color based on status
    uint16_t deviceColor = getDeviceColor(device);
    
    // Draw status indicators
    int iconX = 5;
    
    // Signal strength icon
    drawSignalStrengthIcon(iconX, y + 6, device.rssi);
