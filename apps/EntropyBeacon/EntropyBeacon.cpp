#include "EntropyBeacon.h"
#include <math.h>

// ========================================
// ICON DATA AND CONSTANTS
// ========================================

const uint8_t EntropyBeaconApp::ENTROPY_ICON[32] = {
    0x00, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0xFF, 0xFF, 0x7E, 0x7E,
    0x3C, 0x3C, 0x18, 0x18, 0x81, 0x81, 0xC3, 0xC3, 0x66, 0x66, 0x3C, 0x3C,
    0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ========================================
// CONSTRUCTOR / DESTRUCTOR
// ========================================

EntropyBeaconApp::EntropyBeaconApp() :
    bufferIndex(0),
    bufferFull(false),
    lastSampleTime(0),
    sampleInterval(1000), // 1ms default (1kHz)
    dacEnabled(false),
    dacPin(25), // ESP32 DAC1
    selectedZone(-1)
{
    // Set app metadata
    metadata.name = "EntropyBeacon";
    metadata.version = "1.0";
    metadata.author = "remu.ii";
    metadata.description = "Real-time entropy visualization";
    metadata.category = CATEGORY_TOOLS;
    metadata.maxMemory = 30000; // 30KB for buffers
    metadata.requiresSD = true;
    metadata.requiresWiFi = false;
    metadata.requiresBLE = false;
    
    // Set colors
    backgroundColor = COLOR_BLACK;
    foregroundColor = COLOR_GREEN_PHOS;
    
    showBackButton = true;
    showStatusBar = true;
    
    // Initialize visualization settings
    viz.mode = VIZ_OSCILLOSCOPE;
    viz.sampleRate = RATE_1KHZ;
    viz.dacMode = DAC_OFF;
    viz.timeScale = 1.0f;
    viz.amplitudeScale = 1.0f;
    viz.triggerLevel = 128;
    viz.autoScale = true;
    viz.showGrid = true;
    viz.persistence = 50;
    viz.traceColors[0] = COLOR_GREEN_PHOS;
    viz.traceColors[1] = COLOR_RED_GLOW;
    viz.traceColors[2] = COLOR_PURPLE_GLOW;
    viz.activeTraces = 0x01; // First trace active
    viz.spectrumBars = 32;
    viz.logScale = false;
    viz.spectrumGain = 1.0f;
    viz.recordingEnabled = false;
    viz.recordStartTime = 0;
    viz.samplesRecorded = 0;
    
    // Initialize anomaly detector
    initializeAnomalyDetector();
}

EntropyBeaconApp::~EntropyBeaconApp() {
    cleanup();
}

// ========================================
// MANDATORY BASEAPP METHODS
// ========================================

bool EntropyBeaconApp::initialize() {
    debugLog("EntropyBeacon initializing...");
    
    setState(APP_INITIALIZING);
    
    // Create app data directory
    if (!createAppDataDir()) {
        debugLog("WARNING: Could not create app data directory");
    }
    
    // Initialize DAC
    pinMode(dacPin, OUTPUT);
    dacWrite(dacPin, 0);
    
    // Clear buffers
    memset(entropyBuffer, 0, sizeof(entropyBuffer));
    memset(spectrumData, 0, sizeof(spectrumData));
    memset(waterfallData, 0, sizeof(waterfallData));
    memset(histogramBins, 0, sizeof(histogramBins));
    memset(dacBuffer, 0, sizeof(dacBuffer));
    
    // Setup touch zones
    setupTouchZones();
    
    // Calculate sample interval
    calculateSampleInterval();
    
    // Initialize recording path
    recordingPath = getAppDataPath() + "/entropy_data.csv";
    
    setState(APP_RUNNING);
    debugLog("EntropyBeacon initialized successfully");
    
    return true;
}

void EntropyBeaconApp::update() {
    if (currentState != APP_RUNNING) return;
    
    unsigned long currentTime = micros();
    
    // Sample entropy at specified rate
    if (currentTime - lastSampleTime >= sampleInterval) {
        sampleEntropy();
        lastSampleTime = currentTime;
    }
    
    // Update DAC output if enabled
    if (viz.dacMode != DAC_OFF) {
        updateDACOutput();
    }
    
    frameCount++;
}

void EntropyBeaconApp::render() {
    if (currentState != APP_RUNNING) return;
    
    // Clear screen
    displayManager.clearScreen(backgroundColor);
    
    // Draw interface
    drawInterface();
    
    // Draw visualization based on current mode
    switch (viz.mode) {
        case VIZ_OSCILLOSCOPE:
            drawOscilloscope();
            break;
        case VIZ_SPECTRUM:
            drawSpectrum();
            break;
        case VIZ_WATERFALL:
            drawWaterfall();
            break;
        case VIZ_SCATTER:
            drawScatterPlot();
            break;
        case VIZ_HISTOGRAM:
            drawHistogram();
            break;
        case VIZ_ANOMALY:
            drawAnomalyView();
            break;
    }
    
    // Draw controls and status
    drawControls();
    drawStatusBar();
    
    // Draw common UI elements
    drawCommonUI();
}

bool EntropyBeaconApp::handleTouch(TouchPoint touch) {
    // Handle common UI first
    if (handleCommonTouch(touch)) {
        return true;
    }
    
    if (!touch.isNewPress) return false;
    
    // Handle control touches
    handleControlTouch(touch);
    
    return true;
}

void EntropyBeaconApp::cleanup() {
    // Stop recording if active
    if (viz.recordingEnabled) {
        stopRecording();
    }
    
    // Turn off DAC
    dacWrite(dacPin, 0);
    
    debugLog("EntropyBeacon cleanup complete");
}

const uint8_t* EntropyBeaconApp::getIcon() const {
    return ENTROPY_ICON;
}

// ========================================
// OPTIONAL BASEAPP METHODS
// ========================================

void EntropyBeaconApp::onPause() {
    // Stop recording when pausing
    if (viz.recordingEnabled) {
        stopRecording();
    }
}

void EntropyBeaconApp::onResume() {
    // Recalculate sample interval in case system timing changed
    calculateSampleInterval();
}

String EntropyBeaconApp::getSettingName(uint8_t index) const {
    switch (index) {
        case 0: return "Oscilloscope";
        case 1: return "Spectrum";
        case 2: return "Waterfall";
        case 3: return "Anomaly View";
        case 4: return "Start Recording";
        case 5: return "Export Data";
        default: return "";
    }
}

void EntropyBeaconApp::handleSetting(uint8_t index) {
    switch (index) {
        case 0: // Oscilloscope
            setVisualizationMode(VIZ_OSCILLOSCOPE);
            break;
        case 1: // Spectrum
            setVisualizationMode(VIZ_SPECTRUM);
            break;
        case 2: // Waterfall
            setVisualizationMode(VIZ_WATERFALL);
            break;
        case 3: // Anomaly View
            setVisualizationMode(VIZ_ANOMALY);
            break;
        case 4: // Start Recording
            if (viz.recordingEnabled) {
                stopDataRecording();
            } else {
                startDataRecording();
            }
            break;
        case 5: // Export Data
            exportData("entropy_export.json", "json");
            break;
    }
}

// ========================================
// SAMPLING AND DATA PROCESSING
// ========================================

void EntropyBeaconApp::sampleEntropy() {
    // Create new data point
    EntropyPoint point;
    point.timestamp = millis();
    point.anomaly = false;
    
    // Sample from multiple entropy sources and combine
    uint16_t sample1 = readEntropySource(ENTROPY_PIN_1);
    uint16_t sample2 = readEntropySource(ENTROPY_PIN_2);
    uint16_t sample3 = readEntropySource(ENTROPY_PIN_3);
    
    // Combine samples (XOR for entropy mixing)
    point.value = sample1 ^ (sample2 << 4) ^ (sample3 << 8);
    point.value &= 0xFFF; // Limit to 12-bit range
    
    // Normalize to 0.0-1.0 range
    point.normalized = (float)point.value / 4095.0f;
    
    // Process the point
    processEntropyPoint(point);
    
    // Store in circular buffer
    entropyBuffer[bufferIndex] = point;
    bufferIndex = (bufferIndex + 1) % ENTROPY_BUFFER_SIZE;
    
    if (!bufferFull && bufferIndex == 0) {
        bufferFull = true;
    }
    
    // Update histogram
    updateHistogram(point.value);
    
    // Write to recording file if active
    if (viz.recordingEnabled) {
        writeDataPoint(point);
        viz.samplesRecorded++;
    }
}

void EntropyBeaconApp::processEntropyPoint(EntropyPoint& point) {
    // Detect anomalies
    detectAnomalies(point);
    
    // Update running statistics
    updateAnomalyStats(point.normalized);
}

void EntropyBeaconApp::calculateSampleInterval() {
    // Convert sample rate to microsecond interval
    sampleInterval = 1000000 / viz.sampleRate;
    
    // Clamp to reasonable limits
    sampleInterval = max(MIN_SAMPLE_INTERVAL, min(MAX_SAMPLE_INTERVAL, sampleInterval));
    
    debugLog("Sample interval set to: " + String(sampleInterval) + " us");
}

uint16_t EntropyBeaconApp::readEntropySource(uint8_t source) {
    // Read from analog pin with some processing
    uint16_t reading = analogRead(source);
    
    // Add system entropy for more randomness
    reading ^= (systemCore.getEntropyPool() & 0xFFF);
    
    return reading;
}

// ========================================
// VISUALIZATION METHODS
// ========================================

void EntropyBeaconApp::drawOscilloscope() {
    if (getBufferSize() < 2) return;
    
    // Draw grid if enabled
    if (viz.showGrid) {
        drawGrid();
    }
    
    // Draw trigger line
    drawTriggerLine();
    
    // Calculate display parameters
    uint16_t samplesPerPixel = max(1, getBufferSize() / GRAPH_WIDTH);
    int16_t centerY = GRAPH_Y + GRAPH_HEIGHT / 2;
    
    // Draw waveform
    displayManager.setFont(FONT_SMALL);
    
    for (int16_t x = 0; x < GRAPH_WIDTH - 1; x++) {
        uint16_t sampleIndex1 = (bufferIndex + x * samplesPerPixel) % ENTROPY_BUFFER_SIZE;
        uint16_t sampleIndex2 = (bufferIndex + (x + 1) * samplesPerPixel) % ENTROPY_BUFFER_SIZE;
        
        if (!bufferFull && (sampleIndex1 >= bufferIndex || sampleIndex2 >= bufferIndex)) {
            continue;
        }
        
        EntropyPoint& point1 = entropyBuffer[sampleIndex1];
        EntropyPoint& point2 = entropyBuffer[sampleIndex2];
        
        // Convert values to screen coordinates
        int16_t y1 = mapToGraph(point1.value, GRAPH_HEIGHT);
        int16_t y2 = mapToGraph(point2.value, GRAPH_HEIGHT);
        
        y1 = GRAPH_Y + GRAPH_HEIGHT - y1;
        y2 = GRAPH_Y + GRAPH_HEIGHT - y2;
        
        // Draw line between points
        uint16_t traceColor = viz.traceColors[0];
        
        // Highlight anomalies
        if (point1.anomaly || point2.anomaly) {
            traceColor = COLOR_RED_GLOW;
        }
        
        displayManager.drawLine(GRAPH_X + x, y1, GRAPH_X + x + 1, y2, traceColor);
        
        // Draw anomaly markers
        if (point1.anomaly) {
            displayManager.drawRetroCircle(GRAPH_X + x, y1, 2, COLOR_RED_GLOW, false);
        }
    }
}

void EntropyBeaconApp::drawSpectrum() {
    // Perform FFT on recent data
    performFFT();
    
    // Draw frequency bars
    int16_t barWidth = GRAPH_WIDTH / viz.spectrumBars;
    int16_t barSpacing = max(1, barWidth / 4);
    
    for (uint8_t i = 0; i < viz.spectrumBars; i++) {
        if (i >= FFT_SIZE/2) break;
        
        float magnitude = spectrumData[i].magnitude * viz.spectrumGain;
        int16_t barHeight = (int16_t)(magnitude * GRAPH_HEIGHT);
        barHeight = min(barHeight, GRAPH_HEIGHT);
        
        int16_t barX = GRAPH_X + i * (barWidth + barSpacing);
        int16_t barY = GRAPH_Y + GRAPH_HEIGHT - barHeight;
        
        // Color based on frequency
        uint16_t barColor = COLOR_GREEN_PHOS;
        if (i < viz.spectrumBars / 3) {
            barColor = COLOR_RED_GLOW; // Low frequencies
        } else if (i > 2 * viz.spectrumBars / 3) {
            barColor = COLOR_BLUE_CYBER; // High frequencies
        }
        
        displayManager.drawRetroRect(barX, barY, barWidth - barSpacing, barHeight, barColor, true);
    }
    
    // Draw frequency labels
    displayManager.setFont(FONT_SMALL);
    for (uint8_t i = 0; i < 4; i++) {
        int16_t labelX = GRAPH_X + i * GRAPH_WIDTH / 3;
        float frequency = (float)(i * viz.sampleRate) / 6.0f; // Rough approximation
        displayManager.drawText(labelX, GRAPH_Y + GRAPH_HEIGHT + 5, 
                               formatFrequency(frequency), COLOR_LIGHT_GRAY);
    }
}

void EntropyBeaconApp::drawWaterfall() {
    // Shift waterfall data up
    for (int16_t y = 0; y < WATERFALL_HEIGHT - 1; y++) {
        for (int16_t x = 0; x < GRAPH_WIDTH; x++) {
            waterfallData[y][x] = waterfallData[y + 1][x];
        }
    }
    
    // Add new line from current spectrum
    performFFT();
    for (int16_t x = 0; x < GRAPH_WIDTH; x++) {
        uint8_t spectrumIndex = (x * (FFT_SIZE/2)) / GRAPH_WIDTH;
        float magnitude = spectrumData[spectrumIndex].magnitude;
        waterfallData[WATERFALL_HEIGHT - 1][x] = (uint8_t)(magnitude * 255);
    }
    
    // Draw waterfall
    for (int16_t y = 0; y < min(WATERFALL_HEIGHT, GRAPH_HEIGHT); y++) {
        for (int16_t x = 0; x < GRAPH_WIDTH; x++) {
            uint8_t intensity = waterfallData[y][x];
            
            // Convert intensity to color
            uint16_t pixelColor = COLOR_BLACK;
            if (intensity > 200) {
                pixelColor = COLOR_WHITE;
            } else if (intensity > 150) {
                pixelColor = COLOR_RED_GLOW;
            } else if (intensity > 100) {
                pixelColor = COLOR_PURPLE_GLOW;
            } else if (intensity > 50) {
                pixelColor = COLOR_GREEN_PHOS;
            } else if (intensity > 25) {
                pixelColor = COLOR_DARK_GRAY;
            }
            
            if (pixelColor != COLOR_BLACK) {
                displayManager.drawPixel(GRAPH_X + x, GRAPH_Y + y, pixelColor);
            }
        }
    }
}

void EntropyBeaconApp::drawScatterPlot() {
    if (getBufferSize() < 2) return;
    
    // Draw X vs Y plot of consecutive samples
    for (uint16_t i = 0; i < getBufferSize() - 1; i++) {
        EntropyPoint& point1 = entropyBuffer[i];
        EntropyPoint& point2 = entropyBuffer[(i + 1) % ENTROPY_BUFFER_SIZE];
        
        // Map values to graph coordinates
        int16_t x = GRAPH_X + (point1.value * GRAPH_WIDTH) / 4095;
        int16_t y = GRAPH_Y + GRAPH_HEIGHT - (point2.value * GRAPH_HEIGHT) / 4095;
        
        // Clamp to graph area
        x = max(GRAPH_X, min(GRAPH_X + GRAPH_WIDTH - 1, x));
        y = max(GRAPH_Y, min(GRAPH_Y + GRAPH_HEIGHT - 1, y));
        
        uint16_t pointColor = point1.anomaly ? COLOR_RED_GLOW : COLOR_GREEN_PHOS;
        displayManager.drawPixel(x, y, pointColor);
    }
}

void EntropyBeaconApp::drawHistogram() {
    // Find maximum bin count for scaling
    uint16_t maxCount = 0;
    for (uint16_t i = 0; i < 256; i++) {
        maxCount = max(maxCount, histogramBins[i]);
    }
    
    if (maxCount == 0) return;
    
    // Draw histogram bars
    int16_t barWidth = GRAPH_WIDTH / 256;
    if (barWidth < 1) barWidth = 1;
    
    for (uint16_t i = 0; i < 256; i++) {
        int16_t barHeight = (histogramBins[i] * GRAPH_HEIGHT) / maxCount;
        int16_t barX = GRAPH_X + (i * GRAPH_WIDTH) / 256;
        int16_t barY = GRAPH_Y + GRAPH_HEIGHT - barHeight;
        
        if (barHeight > 0) {
            displayManager.drawRetroRect(barX, barY, barWidth, barHeight, COLOR_GREEN_PHOS, true);
        }
    }
    
    // Draw distribution statistics
    displayManager.setFont(FONT_SMALL);
    displayManager.drawText(GRAPH_X, GRAPH_Y - 15, 
                           "Mean: " + String(anomalyDetector.mean, 3), COLOR_WHITE);
    displayManager.drawText(GRAPH_X + 100, GRAPH_Y - 15,
                           "StdDev: " + String(getStandardDeviation(), 3), COLOR_WHITE);
}

void EntropyBeaconApp::drawAnomalyView() {
    // Draw recent anomalies timeline
    displayManager.setFont(FONT_MEDIUM);
    displayManager.drawText(GRAPH_X, GRAPH_Y - 20, "Anomaly Detection", COLOR_RED_GLOW);
    
    // Anomaly statistics
    displayManager.setFont(FONT_SMALL);
    displayManager.drawText(GRAPH_X, GRAPH_Y, 
                           "Total Anomalies: " + String(anomalyDetector.anomalyCount), COLOR_WHITE);
    displayManager.drawText(GRAPH_X, GRAPH_Y + 15,
                           "Threshold: " + String(anomalyDetector.threshold, 1) + " σ", COLOR_WHITE);
    displayManager.drawText(GRAPH_X, GRAPH_Y + 30,
                           "Current Value: " + String(getCurrentEntropy(), 3), COLOR_GREEN_PHOS);
    
    // Draw anomaly indicator
    float currentValue = getCurrentEntropy();
    bool isCurrentAnomaly = isAnomaly(currentValue);
    
    int16_t indicatorY = GRAPH_Y + 60;
    displayManager.drawRetroRect(GRAPH_X, indicatorY, 100, 20, 
                                isCurrentAnomaly ? COLOR_RED_GLOW : COLOR_GREEN_PHOS, true);
    
    String statusText = isCurrentAnomaly ? "ANOMALY" : "NORMAL";
    displayManager.drawTextCentered(GRAPH_X, indicatorY + 6, 100, statusText, COLOR_BLACK);
    
    // Draw recent anomaly timeline
    int16_t timelineY = GRAPH_Y + 100;
    displayManager.drawLine(GRAPH_X, timelineY, GRAPH_X + GRAPH_WIDTH, timelineY, COLOR_DARK_GRAY);
    
    // Mark recent anomalies on timeline
    unsigned long currentTime = millis();
    for (uint16_t i = 0; i < getBufferSize(); i++) {
        EntropyPoint& point = entropyBuffer[i];
        if (point.anomaly && (currentTime - point.timestamp) < 60000) { // Last minute
            int16_t timelineX = GRAPH_X + ((currentTime - point.timestamp) * GRAPH_WIDTH) / 60000;
            displayManager.drawLine(timelineX, timelineY - 5, timelineX, timelineY + 5, COLOR_RED_GLOW);
        }
    }
}

void EntropyBeaconApp::drawGrid() {
    // Draw horizontal grid lines
    for (int16_t i = 1; i < 4; i++) {
        int16_t y = GRAPH_Y + (i * GRAPH_HEIGHT) / 4;
        displayManager.drawLine(GRAPH_X, y, GRAPH_X + GRAPH_WIDTH, y, COLOR_DARK_GRAY);
    }
    
    // Draw vertical grid lines
    for (int16_t i = 1; i < 4; i++) {
        int16_t x = GRAPH_X + (i * GRAPH_WIDTH) / 4;
        displayManager.drawLine(x, GRAPH_Y, x, GRAPH_Y + GRAPH_HEIGHT, COLOR_DARK_GRAY);
    }
}

void EntropyBeaconApp::drawTriggerLine() {
    int16_t triggerY = GRAPH_Y + GRAPH_HEIGHT - (viz.triggerLevel * GRAPH_HEIGHT) / 255;
    displayManager.drawLine(GRAPH_X, triggerY, GRAPH_X + GRAPH_WIDTH, triggerY, COLOR_PURPLE_GLOW);
}

// ========================================
// ANALYSIS METHODS
// ========================================

void EntropyBeaconApp::performFFT() {
    // Simplified FFT implementation
    // In a real implementation, you'd use a proper FFT library
    
    uint16_t dataSize = min(FFT_SIZE, getBufferSize());
    if (dataSize < 8) return;
    
    // Copy data to working buffer
    float realData[FFT_SIZE];
    float imagData[FFT_SIZE];
    
    for (uint16_t i = 0; i < dataSize; i++) {
        uint16_t bufferIdx = (bufferIndex + i) % ENTROPY_BUFFER_SIZE;
        realData[i] = entropyBuffer[bufferIdx].normalized - 0.5f; // Center around zero
        imagData[i] = 0.0f;
    }
    
    // Zero pad if necessary
    for (uint16_t i = dataSize; i < FFT_SIZE; i++) {
        realData[i] = 0.0f;
        imagData[i] = 0.0f;
    }
    
    // Simple magnitude calculation (not a real FFT)
    for (uint16_t i = 0; i < FFT_SIZE/2; i++) {
        float sum = 0.0f;
        float frequency = (float)i * viz.sampleRate / FFT_SIZE;
        
        // Calculate correlation with sine wave at this frequency
        for (uint16_t j = 0; j < dataSize; j++) {
            float phase = 2.0f * PI * frequency * j / viz.sampleRate;
            sum += realData[j] * sin(phase);
        }
        
        spectrumData[i].frequency = frequency;
        spectrumData[i].magnitude = abs(sum) / dataSize;
        spectrumData[i].phase = 0.0f;
    }
    
    normalizeSpectrum();
}

void EntropyBeaconApp::normalizeSpectrum() {
    // Find maximum magnitude
    float maxMagnitude = 0.0f;
    for (uint16_t i = 0; i < FFT_SIZE/2; i++) {
        maxMagnitude = max(maxMagnitude, spectrumData[i].magnitude);
    }
    
    // Normalize to 0-1 range
    if (maxMagnitude > 0.0f) {
        for (uint16_t i = 0; i < FFT_SIZE/2; i++) {
            spectrumData[i].magnitude /= maxMagnitude;
        }
    }
}

void EntropyBeaconApp::initializeAnomalyDetector() {
    anomalyDetector.mean = 0.5f;
    anomalyDetector.variance = 0.1f;
    anomalyDetector.threshold = ANOMALY_THRESHOLD_DEFAULT;
    anomalyDetector.windowSize = 100;
    anomalyDetector.enabled = true;
    anomalyDetector.anomalyCount = 0;
}

void EntropyBeaconApp::detectAnomalies(EntropyPoint& point) {
    if (!anomalyDetector.enabled) return;
    
    point.anomaly = isAnomaly(point.normalized);
    
    if (point.anomaly) {
        anomalyDetector.anomalyCount++;
        logAnomaly(point);
    }
}

void EntropyBeaconApp::updateAnomalyStats(float value) {
    // Update running mean and variance using exponential moving average
    float alpha = 0.01f; // Learning rate
    
    float delta = value - anomalyDetector.mean;
    anomalyDetector.mean += alpha * delta;
    anomalyDetector.variance += alpha * (delta * delta - anomalyDetector.variance);
}

bool EntropyBeaconApp::isAnomaly(float value) {
    float standardDev = getStandardDeviation();
    float deviation = abs(value - anomalyDetector.mean);
    
    return deviation > (anomalyDetector.threshold * standardDev);
}

void EntropyBeaconApp::logAnomaly(EntropyPoint& point) {
    debugLog("ANOMALY detected: value=" + String(point.normalized, 4) + 
             " at time=" + String(point.timestamp));
}

void EntropyBeaconApp::updateHistogram(uint16_t value) {
    uint8_t binIndex = value >> 4; // Convert 12-bit to 8-bit for histogram
    if (binIndex < 256) {
        histogramBins[binIndex]++;
        
        // Prevent overflow by scaling down periodically
        if (histogramBins[binIndex] > 30000) {
            for (uint16_t i = 0; i < 256; i++) {
                histogramBins[i] /= 2;
            }
        }
    }
}

// ========================================
// DAC OUTPUT METHODS
// ========================================

void EntropyBeaconApp::updateDACOutput() {
    if (viz.dacMode == DAC_OFF || getBufferSize() == 0) return;
    
    // Get current entropy value
    EntropyPoint& currentPoint = entropyBuffer[(bufferIndex - 1 + ENTROPY_BUFFER_SIZE) % ENTROPY_BUFFER_SIZE];
    uint16_t outputValue = 0;
    
    switch (viz.dacMode) {
        case DAC_RAW:
            outputValue = currentPoint.value >> 4; // Convert 12-bit to 8-bit
            break;
            
        case DAC_FILTERED:
            // Simple low-pass filter
            outputValue = applyFilter(currentPoint.normalized, 0) * 255;
            break;
            
        case DAC_TONE:
            // Convert entropy to audio tone frequency
            generateDACWaveform();
            return;
            
        case DAC_MODULATED:
            // AM modulation
            outputValue = (currentPoint.value >> 4) * sin(2.0f * PI * millis() / 1000.0f) / 2 + 128;
            break;
            
        case DAC_PULSE:
            // Pulse train based on entropy
            outputValue = (currentPoint.value > 2048) ? 255 : 0;
            break;
            
        default:
            return;
    }
    
    outputToDAC(outputValue);
}

void EntropyBeaconApp::generateDACWaveform() {
    // Generate audio tone based on current entropy
    static unsigned long lastToneUpdate = 0;
    static float phase = 0.0f;
    
    if (millis() - lastToneUpdate < 1) return; // 1ms update rate
    
    if (getBufferSize() > 0) {
        EntropyPoint& currentPoint = entropyBuffer[(bufferIndex - 1 + ENTROPY_BUFFER_SIZE) % ENTROPY_BUFFER_SIZE];
        
        // Map entropy to frequency (100Hz - 2kHz)
        float frequency = 100.0f + (currentPoint.normalized * 1900.0f);
        
        // Generate sine wave
        float sampleRate = 8000.0f; // 8kHz DAC update rate
        phase += 2.0f * PI * frequency / sampleRate;
        if (phase > 2.0f * PI) {
            phase -= 2.0f * PI;
        }
        
        uint8_t outputValue = (sin(phase) + 1.0f) * 127.5f;
        outputToDAC(outputValue);
    }
    
    lastToneUpdate = millis();
}

void EntropyBeaconApp::outputToDAC(uint16_t value) {
    // Clamp to 8-bit range
    uint8_t dacValue = min(255, (int)value);
    dacWrite(dacPin, dacValue);
}

float EntropyBeaconApp::applyFilter(float input, uint8_t filterType) {
    static float lastOutput = 0.0f;
    
    switch (filterType) {
        case 0: // Low-pass
            lastOutput = lastOutput * 0.9f + input * 0.1f;
            return lastOutput;
        default:
            return input;
    }
}

// ========================================
// UI AND CONTROL METHODS
// ========================================

void EntropyBeaconApp::drawInterface() {
    // Draw title
    displayManager.setFont(FONT_MEDIUM);
    displayManager.drawText(5, 5, "Entropy Beacon", COLOR_RED_GLOW);
    
    // Draw mode indicator
    String modeNames[] = {"OSC", "SPEC", "FALL", "SCAT", "HIST", "ANOM"};
    displayManager.setFont(FONT_SMALL);
    displayManager.drawText(150, 8, modeNames[viz.mode], COLOR_GREEN_PHOS);
    
    // Draw sample rate
    displayManager.drawText(200, 8, String(viz.sampleRate) + "Hz", COLOR_WHITE);
}

void EntropyBeaconApp::drawControls() {
    // Mode buttons
    int16_t buttonY = 220;
    displayManager.drawButton(5, buttonY, 30, 16, "Mode");
    displayManager.drawButton(40, buttonY, 30, 16, "Rate");
    displayManager.drawButton(75, buttonY, 30, 16, "DAC");
    
    // Recording indicator
    if (viz.recordingEnabled) {
        displayManager.drawButton(110, buttonY, 40, 16, "REC", BUTTON_PRESSED, COLOR_RED_GLOW);
    } else {
        displayManager.drawButton(110, buttonY, 40, 16, "Rec");
    }
    
    // Export button
    displayManager.drawButton(155, buttonY, 40, 16, "Export");
}

void EntropyBeaconApp::drawStatusBar() {
    displayManager.setFont(FONT_SMALL);
    
    // Buffer status
    String bufferStatus = "Buf: " + String(getBufferSize()) + "/" + String(ENTROPY_BUFFER_SIZE);
    displayManager.drawText(5, 25, bufferStatus, COLOR_LIGHT_GRAY);
    
    // Current entropy value
    if (getBufferSize() > 0) {
        float currentEntropy = getCurrentEntropy();
        displayManager.drawText(100, 25, "Val: " + String(currentEntropy, 3), COLOR_WHITE);
    }
    
    // Anomaly count
    displayManager.drawText(200, 25, "Anom: " + String(anomalyDetector.anomalyCount), 
                           anomalyDetector.anomalyCount > 0 ? COLOR_RED_GLOW : COLOR_LIGHT_GRAY);
}

void EntropyBeaconApp::setupTouchZones() {
    // Mode button
    touchZones[0] = {5, 220, 30, 16, "mode", true};
    
    // Sample rate button
    touchZones[1] = {40, 220, 30, 16, "rate", true};
    
    // DAC button
    touchZones[2] = {75, 220, 30, 16, "dac", true};
    
    // Record button
    touchZones[3] = {110, 220, 40, 16, "record", true};
    
    // Export button
    touchZones[4] = {155, 220, 40, 16, "export", true};
    
    // Graph area for parameter adjustment
    touchZones[5] = {GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, "graph", true};
}

void EntropyBeaconApp::handleControlTouch(TouchPoint touch) {
    // Check touch zones
    for (uint8_t i = 0; i < 6; i++) {
        InteractionZone& zone = touchZones[i];
        if (zone.enabled && touchInterface.isPointInRect(touch, zone.x, zone.y, zone.w, zone.h)) {
            
            if (zone.function == "mode") {
                // Cycle through visualization modes
                viz.mode = (VisualizationMode)((viz.mode + 1) % 6);
                debugLog("Mode changed to: " + String(viz.mode));
                
            } else if (zone.function == "rate") {
                // Cycle through sample rates
                SampleRate rates[] = {RATE_100HZ, RATE_500HZ, RATE_1KHZ, RATE_2KHZ, RATE_5KHZ};
                for (uint8_t r = 0; r < 5; r++) {
                    if (viz.sampleRate == rates[r]) {
                        viz.sampleRate = rates[(r + 1) % 5];
                        break;
                    }
                }
                calculateSampleInterval();
                debugLog("Sample rate changed to: " + String(viz.sampleRate));
                
            } else if (zone.function == "dac") {
                // Cycle through DAC modes
                viz.dacMode = (DACMode)((viz.dacMode + 1) % 6);
                debugLog("DAC mode changed to: " + String(viz.dacMode));
                
            } else if (zone.function == "record") {
                // Toggle recording
                if (viz.recordingEnabled) {
                    stopDataRecording();
                } else {
                    startDataRecording();
                }
                
            } else if (zone.function == "export") {
                // Export current data
                exportData("entropy_" + String(millis()) + ".json", "json");
            }
            
            return;
        }
    }
}

// ========================================
// UTILITY METHODS
// ========================================

uint16_t EntropyBeaconApp::mapToGraph(uint16_t value, int16_t graphHeight) {
    return (value * graphHeight) / 4095;
}

float EntropyBeaconApp::getCurrentEntropy() const {
    if (getBufferSize() == 0) return 0.0f;
    
    uint16_t lastIndex = (bufferIndex - 1 + ENTROPY_BUFFER_SIZE) % ENTROPY_BUFFER_SIZE;
    return entropyBuffer[lastIndex].normalized;
}

float EntropyBeaconApp::getStandardDeviation() const {
    return sqrt(anomalyDetector.variance);
}

String EntropyBeaconApp::formatFrequency(float frequency) {
    if (frequency >= 1000.0f) {
        return String(frequency / 1000.0f, 1) + "k";
    } else {
        return String((int)frequency);
    }
}

void EntropyBeaconApp::setVisualizationMode(VisualizationMode mode) {
    viz.mode = mode;
    
    // Mode-specific initialization
    switch (mode) {
        case VIZ_SPECTRUM:
            performFFT();
            break;
        case VIZ_HISTOGRAM:
            // Clear histogram for fresh start
            memset(histogramBins, 0, sizeof(histogramBins));
            break;
        default:
            break;
    }
    
    debugLog("Visualization mode set to: " + String(mode));
}

bool EntropyBeaconApp::startDataRecording(String filename) {
    if (viz.recordingEnabled) return false;
    
    if (filename.isEmpty()) {
        filename = "entropy_" + String(millis()) + ".csv";
    }
    
    String fullPath = getAppDataPath() + "/" + filename;
    recordingFile = SD.open(fullPath, FILE_WRITE);
    
    if (!recordingFile) {
        debugLog("Failed to create recording file: " + fullPath);
        return false;
    }
    
    // Write CSV header
    recordingFile.println("timestamp,value,normalized,anomaly");
    
    viz.recordingEnabled = true;
    viz.recordStartTime = millis();
    viz.samplesRecorded = 0;
    
    debugLog("Recording started: " + filename);
    return true;
}

bool EntropyBeaconApp::stopDataRecording() {
    if (!viz.recordingEnabled) return false;
    
    recordingFile.close();
    viz.recordingEnabled = false;
    
    debugLog("Recording stopped. Samples recorded: " + String(viz.samplesRecorded));
    return true;
}

void EntropyBeaconApp::writeDataPoint(EntropyPoint& point) {
    if (!viz.recordingEnabled || !recordingFile) return;
    
    recordingFile.print(point.timestamp);
    recordingFile.print(",");
    recordingFile.print(point.value);
    recordingFile.print(",");
    recordingFile.print(point.normalized, 6);
    recordingFile.print(",");
    recordingFile.println(point.anomaly ? "1" : "0");
    
    // Flush periodically
    if (viz.samplesRecorded % 100 == 0) {
        recordingFile.flush();
    }
}

bool EntropyBeaconApp::exportData(String filename, String format) {
    String fullPath = getAppDataPath() + "/" + filename;
    
    File exportFile = SD.open(fullPath, FILE_WRITE);
    if (!exportFile) {
        debugLog("Failed to create export file: " + fullPath);
        return false;
    }
    
    if (format == "json") {
        // Export as JSON
        DynamicJsonDocument doc(8192);
        
        doc["export_time"] = millis();
        doc["sample_rate"] = viz.sampleRate;
        doc["buffer_size"] = getBufferSize();
        doc["anomaly_count"] = anomalyDetector.anomalyCount;
        doc["statistics"]["mean"] = anomalyDetector.mean;
        doc["statistics"]["variance"] = anomalyDetector.variance;
        doc["statistics"]["std_deviation"] = getStandardDeviation();
        
        JsonArray dataArray = doc.createNestedArray("data");
        for (uint16_t i = 0; i < getBufferSize(); i++) {
            JsonObject pointObj = dataArray.createNestedObject();
            EntropyPoint& point = entropyBuffer[i];
            pointObj["timestamp"] = point.timestamp;
            pointObj["value"] = point.value;
            pointObj["normalized"] = point.normalized;
            pointObj["anomaly"] = point.anomaly;
        }
        
        serializeJson(doc, exportFile);
    }
    
    exportFile.close();
    debugLog("Data exported: " + filename);
    return true;
}

EntropyPoint EntropyBeaconApp::getDataPoint(uint16_t index) const {
    if (index >= getBufferSize()) {
        return {0, 0.0f, 0, false}; // Return empty point
    }
    
    uint16_t bufferIdx = (bufferIndex + index) % ENTROPY_BUFFER_SIZE;
    return entropyBuffer[bufferIdx];
}

void EntropyBeaconApp::calibrateBaseline() {
    debugLog("Calibrating baseline...");
    
    // Reset statistics
    resetStatistics();
    
    // Collect baseline samples
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) { // 5 second calibration
        update();
        delay(1);
    }
    
    debugLog("Baseline calibration complete");
    debugLog("Mean: " + String(anomalyDetector.mean, 4));
    debugLog("StdDev: " + String(getStandardDeviation(), 4));
}

void EntropyBeaconApp::resetStatistics() {
    initializeAnomalyDetector();
    memset(histogramBins, 0, sizeof(histogramBins));
    viz.samplesRecorded = 0;
    debugLog("Statistics reset");
}