#ifndef HARDWARE_PINS_FIXED_H
#define HARDWARE_PINS_FIXED_H

// ========================================
// remu.ii Hardware Pin Configuration - FIXED VERSION
// ESP32 + Adafruit ILI9341 2.8" TFT + 4-Wire Resistive Touch
// All pin conflicts resolved for Arduino IDE compilation
// ========================================

// TFT Display Pins (SPI)
#define TFT_CS     15    // Chip Select
#define TFT_DC     2     // Data/Command
#define TFT_RST    4     // Reset
#define TFT_MOSI   23    // SPI Data Out
#define TFT_SCLK   18    // SPI Clock
#define TFT_MISO   19    // SPI Data In (optional)

// Touch Controller Pins (4-Wire Resistive) - FIXED
#define TOUCH_XP   32    // X+ (Right)
#define TOUCH_XM   33    // X- (Left)
#define TOUCH_YP   14    // Y+ (Up)
#define TOUCH_YM   13    // Y- (Down)

// SD Card Pins (shares SPI bus with display)
#define SD_CS      5     // SD Card Chip Select

// Power Management
#define BATTERY_PIN 35   // ADC pin for battery voltage monitoring
#define PWR_LED    0     // Power status LED

// Entropy Sources (floating analog pins)
#define ENTROPY_PIN_1  36   // ADC1_CH0
#define ENTROPY_PIN_2  39   // ADC1_CH3
#define ENTROPY_PIN_3  34   // ADC1_CH6

// Audio Output - I2S (recommended)
#define I2S_BCK_PIN   26    // I2S Bit Clock
#define I2S_WS_PIN    25    // I2S Word Select
#define I2S_DATA_PIN  22    // I2S Data Output
#define AUDIO_AMP_EN  21    // Audio amplifier enable

// PWM Audio (backup option)
#define PWM_OUT_LEFT  16    // Left channel PWM output
#define PWM_OUT_RIGHT 17    // Right channel PWM output

// BLE/RF Pins (external RF modules)
#define RF_CE_PIN     12    // RF module Chip Enable
#define RF_CSN_PIN    27    // RF module Chip Select
#define RF_IRQ_PIN    16    // RF module IRQ (shared with PWM_OUT_LEFT)

// System Control
#define BUZZER_PIN    1     // Optional piezo buzzer
#define DEBUG_LED     2     // Debug LED (shared with TFT_DC)

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

// Memory optimization settings
#define ENABLE_SCREEN_BUFFER false  // Disable large screen buffer to save memory
#define REDUCED_MEMORY_MODE true    // Enable memory-saving features

#endif // HARDWARE_PINS_FIXED_H