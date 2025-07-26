#ifndef TOUCH_INTERFACE_H
#define TOUCH_INTERFACE_H

#include <Arduino.h>
#include "../Config/hardware_pins.h"

// ========================================
// TouchInterface - 4-wire resistive touch for remu.ii
// Stylus input processing with debouncing and calibration
// ========================================

// Touch point structure
struct TouchPoint {
    int16_t x, y;           // Screen coordinates
    uint16_t rawX, rawY;    // Raw ADC readings
    uint16_t pressure;      // Touch pressure (0-1023)
    bool isPressed;         // Current touch state
    bool wasPressed;        // Previous touch state
    bool isNewPress;        // Rising edge detection
    bool isNewRelease;      // Falling edge detection
    unsigned long timestamp; // When touch occurred
};

// Touch gesture types
enum TouchGesture {
    GESTURE_NONE,
    GESTURE_TAP,
    GESTURE_DOUBLE_TAP,
    GESTURE_LONG_PRESS,
    GESTURE_DRAG_START,
    GESTURE_DRAG_MOVE,
    GESTURE_DRAG_END,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN
};

// Touch gesture data
struct TouchGesture_t {
    TouchGesture type;
    TouchPoint startPoint;
    TouchPoint currentPoint;
    TouchPoint endPoint;
    int16_t deltaX, deltaY;
    unsigned long duration;
    float velocity;
};

// Calibration data structure
struct TouchCalibration {
    int16_t xMin, xMax;     // Raw X range
    int16_t yMin, yMax;     // Raw Y range
    int16_t offsetX, offsetY; // Screen offset correction
    float scaleX, scaleY;   // Scaling factors
    bool isCalibrated;      // Calibration status
};

// Touch configuration constants
#define TOUCH_SAMPLES         4      // Number of samples to average
#define DEBOUNCE_DELAY       50      // Debounce time in milliseconds
#define LONG_PRESS_TIME     800      // Long press threshold in ms
#define DOUBLE_TAP_TIME     300      // Double tap window in ms
#define DRAG_THRESHOLD       10      // Minimum movement for drag in pixels
#define SWIPE_THRESHOLD      50      // Minimum movement for swipe in pixels
#define SWIPE_MIN_VELOCITY   200     // Minimum velocity for swipe detection

class TouchInterface {
private:
    // Touch state
    TouchPoint currentTouch;
    TouchPoint lastTouch;
    TouchGesture_t currentGesture;
    
    // Timing variables
    unsigned long lastReadTime;
    unsigned long lastPressTime;
    unsigned long lastReleaseTime;
    unsigned long gestureStartTime;
    
    // Calibration data
    TouchCalibration calibration;
    
    // State tracking
    bool touchActive;
    bool gestureActive;
    uint8_t tapCount;
    
    // Private methods - 4-wire resistive touch reading
    uint16_t readTouchX();
    uint16_t readTouchY();
    uint16_t readTouchPressure();
    bool isTouchPressed();
    
    // Touch processing
    void sampleTouch();
    TouchPoint averageReadings(TouchPoint* samples, uint8_t count);
    void processTouch();
    void detectGestures();
    
    // Coordinate transformation
    int16_t mapX(uint16_t rawX);
    int16_t mapY(uint16_t rawY);
    void applyCalibration(TouchPoint& point);
    
    // Gesture detection helpers
    float calculateVelocity(TouchPoint start, TouchPoint end, unsigned long timeMs);
    float calculateDistance(TouchPoint p1, TouchPoint p2);
    TouchGesture detectSwipeDirection(int16_t deltaX, int16_t deltaY);

public:
    TouchInterface();
    ~TouchInterface();
    
    // Core initialization and lifecycle
    bool initialize();
    void update();
    void shutdown();
    
    // Touch reading
    TouchPoint getCurrentTouch();
    TouchPoint getLastTouch() const { return lastTouch; }
    bool isTouched() const { return currentTouch.isPressed; }
    bool wasNewPress() const { return currentTouch.isNewPress; }
    bool wasNewRelease() const { return currentTouch.isNewRelease; }
    
    // Gesture recognition
    TouchGesture_t getCurrentGesture() const { return currentGesture; }
    bool hasNewGesture() const;
    TouchGesture getLastGestureType() const { return currentGesture.type; }
    void clearGesture();
    
    // Calibration
    bool startCalibration();
    bool calibratePoint(int16_t screenX, int16_t screenY);
    bool finishCalibration();
    void loadCalibration();
    void saveCalibration();
    void resetCalibration();
    bool isCalibrated() const { return calibration.isCalibrated; }
    TouchCalibration getCalibration() const { return calibration; }
    
    // Coordinate utilities
    bool isPointInRect(TouchPoint point, int16_t x, int16_t y, int16_t w, int16_t h);
    bool isPointInCircle(TouchPoint point, int16_t centerX, int16_t centerY, int16_t radius);
    
    // Touch zones and UI helpers
    bool isTouchInButton(int16_t x, int16_t y, int16_t w, int16_t h);
    uint8_t getTouchGridPosition(uint8_t gridCols, uint8_t gridRows); // Returns 0-255, 255 = no touch
    
    // Configuration
    void setPressureThreshold(uint16_t threshold);
    void setDebounceTime(unsigned long timeMs);
    void setLongPressTime(unsigned long timeMs);
    uint16_t getPressureThreshold() const;
    
    // Diagnostics and debugging
    void printTouchInfo();
    void printCalibrationInfo();
    void runTouchTest(); // Interactive touch test mode
    String getTouchStatusString();
    
    // Raw touch access (for debugging/calibration)
    uint16_t getRawX() { return readTouchX(); }
    uint16_t getRawY() { return readTouchY(); }
    uint16_t getRawPressure() { return readTouchPressure(); }
};

// Global touch interface instance
extern TouchInterface touchInterface;

#endif // TOUCH_INTERFACE_H