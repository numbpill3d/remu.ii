#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

// ========================================
// remu.ii Hardware Pin Configuration
// ESP32 + Adafruit ILI9341 2.8" TFT + 4-Wire Resistive Touch
// ========================================

// TFT Display Pins (SPI)
#define TFT_CS     15    // Chip Select
#define TFT_DC     2     // Data/Command
#define TFT_RST    4     // Reset
#define TFT_MOSI   23    // SPI Data Out (Master Out Slave In)
#define TFT_SCLK   18    // SPI Clock
#define TFT_MISO   19    // SPI Data In (Master In Slave Out) - optional

// Touch Controller Pins (4-Wire Resistive)
#define TOUCH_XP   32    // X+ (Right)
#define TOUCH_XM   33    // X- (Left) 
#define TOUCH_YP   25    // Y+ (Up)
#define TOUCH_YM   26    // Y- (Down)

// SD Card Pins (shares SPI bus with display)
#define SD_CS      5     // SD Card Chip Select

// Power Management
#define BATTERY_PIN 35   // ADC pin for battery voltage monitoring
#define PWR_LED    13    // Power status LED

// Entropy Sources (floating analog pins)
#define ENTROPY_PIN_1  36   // ADC1_CH0
#define ENTROPY_PIN_2  39   // ADC1_CH3
#define ENTROPY_PIN_3  34   // ADC1_CH6

// System Control
#define BUZZER_PIN    27    // Optional piezo buzzer
#define DEBUG_LED     2     // Debug LED (shared with TFT_DC - use carefully)

// Display Configuration
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 1   // Landscape orientation

// Touch Configuration
#define PRESSURE_THRESHOLD  200   // Minimum pressure for valid touch
#define TOUCH_DEBOUNCE_MS   50    // Debounce delay in milliseconds

// SPI Configuration
#define SPI_FREQUENCY   40000000  // 40 MHz SPI clock
#define TFT_SPI_HOST    VSPI_HOST // Use VSPI for display

#endif // HARDWARE_PINS_H