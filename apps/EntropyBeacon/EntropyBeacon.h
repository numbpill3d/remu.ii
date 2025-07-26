#ifndef ENTROPY_BEACON_H
#define ENTROPY_BEACON_H

#include "../../core/AppManager/BaseApp.h"
#include "../../core/SystemCore/SystemCore.h"
#include <ArduinoJson.h>
#include <SD.h>

// ========================================
// EntropyBeacon - Real-time entropy visualization for remu.ii
// Oscilloscope-style display, spectrum analysis, DAC output, anomaly detection
// ========================================

// Visualization modes
enum VisualizationMode {
    VIZ_OSCILLOSCOPE,   // Time domain waveform
    VIZ_SPECTRUM,       // Frequency domain analysis
    VIZ_WATERFALL,      // Waterfall display
    VIZ_SCATTER,        // XY scatter plot
    VIZ_HISTOGRAM,      // Value distribution
    VIZ_ANOMALY         // Anomaly detection view
};

// Sample rates
enum SampleRate {
    RATE_100HZ = 100,
    RATE_500HZ = 500,
    RATE_1KHZ = 1000,
    RATE_2KHZ = 2000,
    RATE_5KHZ = 5000,
    RATE_10KHZ = 10000
};

// DAC output modes
enum DACMode {
    DAC_OFF,            // No output
    DAC_RAW,            // Raw entropy output
    DAC_FILTERED,       // Low-pass filtered
    DAC_TONE,           // Convert to audio tone
    DAC_MODULATED,      // AM/FM modulated
    DAC_PULSE           // Pulse train
};

// Buffer sizes
#define ENTROPY_BUFFER_SIZE 512
#define FFT_SIZE 256
#define WATERFALL_HEIGHT 64
#define ANOMALY_HISTORY 128

// Display configuration
#define GRAPH_WIDTH 280
#define GRAPH_HEIGHT 140
#define GRAPH_X 20
#define GRAPH_Y 40

// Entropy data point
struct EntropyPoint {
    uint16_t value;         // Raw ADC value (0-4095)
    float normalized;       // Normalized to 0.0-1.0
    unsigned long timestamp; // When sample was taken
    bool anomaly;           // Flagged as anomalous
};

// Frequency bin for spectrum analysis
struct FrequencyBin {
    float frequency;        // Frequency in Hz
    float magnitude;        // Magnitude
    float phase;           // Phase
};

// Anomaly detection
struct AnomalyDetector {
    float mean;            // Running mean
    float variance;        // Running variance
    float threshold;       // Anomaly threshold (stddev multiplier)
    uint16_t windowSize;   // Analysis window size
    bool enabled;          // Detection enabled
    uint32_t anomalyCount; // Total anomalies detected
};

// Visualization state
struct EntropyVisualization {
    VisualizationMode mode;
    SampleRate sampleRate;
    DACMode dacMode;
    
    // Display settings
    float timeScale;       // Time scale for oscilloscope
    float amplitudeScale;  // Amplitude scaling
    uint8_t triggerLevel;  // Trigger level (0-255)
    bool autoScale;        // Auto-scale amplitude
    bool showGrid;         // Show grid lines
    uint8_t persistence;   // Trace persistence (0-255)
    
    // Colors for different traces
    uint16_t traceColors[3];
    uint8_t activeTraces;  // Bitmask of active traces
    
    // Spectrum analyzer settings
    uint8_t spectrumBars;  // Number of frequency bars
    bool logScale;         // Logarithmic frequency scale
    float spectrumGain;    // Spectrum display gain
    
    // Export settings
    bool recordingEnabled; // Recording to SD card
    unsigned long recordStartTime;
    uint32_t samplesRecorded;
};

// Touch interaction zones
struct InteractionZone {
    int16_t x, y, w, h;
    String function;
    bool enabled;
};

class EntropyBeaconApp : public BaseApp {
private:
    // Data buffers
    EntropyPoint entropyBuffer[ENTROPY_BUFFER_SIZE];
    FrequencyBin spectrumData[FFT_SIZE/2];
    uint8_t waterfallData[WATERFALL_HEIGHT][GRAPH_WIDTH];
    uint16_t histogramBins[256];
    
    // Buffer management
    uint16_t bufferIndex;
    uint16_t bufferFull;
    
    // Sampling control
    unsigned long lastSampleTime;
    unsigned long sampleInterval; // Microseconds between samples
    
    // Visualization state
    EntropyVisualization viz;
    AnomalyDetector anomalyDetector;
    
    // DAC output
    bool dacEnabled;
    uint8_t dacPin;
    uint16_t dacBuffer[64]; // Small buffer for DAC output
    
    // Touch zones
    InteractionZone touchZones[8];
    int8_t selectedZone;
    
    // File recording
    File recordingFile;
    String recordingPath;
    
    // Private methods - Sampling
    void sampleEntropy();
    void processEntropyPoint(EntropyPoint& point);
    void calculateSampleInterval();
    uint16_t readEntropySource(uint8_t source);
    
    // Private methods - Analysis
    void performFFT();
    void updateSpectrum();
    void detectAnomalies(EntropyPoint& point);
    void updateHistogram(uint16_t value);
    void calculateStatistics();
    
    // Private methods - Visualization
    void drawOscilloscope();
    void drawSpectrum();
    void drawWaterfall();
    void drawScatterPlot();
    void drawHistogram();
    void drawAnomalyView();
    void drawGrid();
    void drawTriggerLine();
    void drawCursor(int16_t x, int16_t y);
    
    // Private methods - DAC Output
    void updateDACOutput();
    void generateDACWaveform();
    void outputToDAC(uint16_t value);
    void modulateSignal(uint16_t* buffer, uint16_t length);
    
    // Private methods - UI
    void drawInterface();
    void drawControls();
    void drawStatusBar();
    void drawModeSelector();
    void drawParameterControls();
    void setupTouchZones();
    void handleControlTouch(TouchPoint touch);
    
    // Private methods - Recording
    bool startRecording();
    void stopRecording();
    void writeDataPoint(EntropyPoint& point);
    bool exportData(String filename, String format);
    
    // Private methods - Signal Processing
    float applyFilter(float input, uint8_t filterType);
    void normalizeSpectrum();
    void applyWindow(float* data, uint16_t length, uint8_t windowType);
    float calculateEntropy(uint16_t* data, uint16_t length);
    
    // Private methods - Anomaly Detection
    void initializeAnomalyDetector();
    void updateAnomalyStats(float value);
    bool isAnomaly(float value);
    void logAnomaly(EntropyPoint& point);
    
    // Utility methods
    uint16_t mapToGraph(uint16_t value, int16_t graphHeight);
    int16_t valueToPixel(float value, float min, float max, int16_t pixels);
    float pixelToValue(int16_t pixel, float min, float max, int16_t pixels);
    String formatFrequency(float frequency);
    String formatDuration(unsigned long milliseconds);
    
    // Constants
    static const uint16_t MIN_SAMPLE_INTERVAL = 100;  // 100 microseconds (10kHz max)
    static const uint16_t MAX_SAMPLE_INTERVAL = 10000; // 10ms (100Hz min)
    static const float ANOMALY_THRESHOLD_DEFAULT = 3.0f; // 3 standard deviations

public:
    EntropyBeaconApp();
    virtual ~EntropyBeaconApp();
    
    // Mandatory BaseApp methods
    bool initialize() override;
    void update() override;
    void render() override;
    bool handleTouch(TouchPoint touch) override;
    void cleanup() override;
    String getName() const override { return "EntropyBeacon"; }
    const uint8_t* getIcon() const override;
    
    // Optional BaseApp methods
    void onPause() override;
    void onResume() override;
    bool saveState() override;
    bool loadState() override;
    bool handleMessage(AppMessage message, void* data = nullptr) override;
    
    // Settings menu support
    uint8_t getSettingsCount() const override { return 6; }
    String getSettingName(uint8_t index) const override;
    void handleSetting(uint8_t index) override;
    
    // EntropyBeacon-specific methods
    void setVisualizationMode(VisualizationMode mode);
    VisualizationMode getVisualizationMode() const { return viz.mode; }
    
    void setSampleRate(SampleRate rate);
    SampleRate getSampleRate() const { return viz.sampleRate; }
    
    void setDACMode(DACMode mode);
    DACMode getDACMode() const { return viz.dacMode; }
    
    // Data access
    uint16_t getBufferSize() const { return bufferFull ? ENTROPY_BUFFER_SIZE : bufferIndex; }
    EntropyPoint getDataPoint(uint16_t index) const;
    float getCurrentEntropy() const;
    uint32_t getAnomalyCount() const { return anomalyDetector.anomalyCount; }
    
    // Recording control
    bool isRecording() const { return viz.recordingEnabled; }
    bool startDataRecording(String filename = "");
    bool stopDataRecording();
    uint32_t getSamplesRecorded() const { return viz.samplesRecorded; }
    
    // Analysis results
    float getMeanValue() const { return anomalyDetector.mean; }
    float getVariance() const { return anomalyDetector.variance; }
    float getStandardDeviation() const;
    
    // Calibration and configuration
    void calibrateBaseline();
    void resetStatistics();
    void setAnomalyThreshold(float threshold);
    float getAnomalyThreshold() const { return anomalyDetector.threshold; }
    
    // Debug methods
    void debugPrintBuffer();
    void debugGenerateTestSignal();
    void debugTriggerAnomaly();
    void debugExportSpectrum();
    
    // Icon data
    static const uint8_t ENTROPY_ICON[32];
};

#endif // ENTROPY_BEACON_H