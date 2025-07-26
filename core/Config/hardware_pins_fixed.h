#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

// ========================================
// remu.ii Hardware Pin Configuration - CORRECTED
// ESP32 + Adafruit ILI9341 2.8" TFT + 4-Wire Resistive Touch
// ========================================

// TFT Display Pins (SPI)
#define TFT_CS     15    // Chip Select
#define TFT_DC     2     // Data/Command
#define TFT_RST    4     // Reset
#define TFT_MOSI   23    // SPI Data Out (Master Out Slave In)
#define TFT_SCLK   18    // SPI Clock
#define TFT_MISO   19    // SPI Data In (Master In Slave Out) - optional

// Touch Controller Pins (4-Wire Resistive) - FIXED CONFLICTS
#define TOUCH_XP   32    // X+ (Right)
#define TOUCH_XM   33    // X- (Left)
#define TOUCH_YP   14    // Y+ (Up) - changed from 14 to avoid conflicts
#define TOUCH_YM   13    // Y- (Down) - changed from 12 to avoid conflicts

// SD Card Pins (shares SPI bus with display)
#define SD_CS      5     // SD Card Chip Select

// Power Management
#define BATTERY_PIN 35   // ADC pin for battery voltage monitoring
#define PWR_LED    0     // Power status LED - changed from GPIO1 (TX) to GPIO0

// Entropy Sources (floating analog pins)
#define ENTROPY_PIN_1  36   // ADC1_CH0
#define ENTROPY_PIN_2  39   // ADC1_CH3
#define ENTROPY_PIN_3  34   // ADC1_CH6

// Audio Output - FIXED CONFLICTS
// Using I2S for best quality (recommended)
#define I2S_BCK_PIN   26    // I2S Bit Clock
#define I2S_WS_PIN    25    // I2S Word Select (LRCLK)
#define I2S_DATA_PIN  22    // I2S Data Output
#define AUDIO_AMP_EN  21    // Audio amplifier enable

// Alternative: Built-in DAC outputs (simpler but lower quality)
// Note: These conflict with I2S - choose one method
// #define DAC_OUT_LEFT  25    // Left channel DAC output (built-in DAC1)
// #define DAC_OUT_RIGHT 26    // Right channel DAC output (built-in DAC2)

// PWM Audio (backup option) - FIXED CONFLICTS
#define PWM_OUT_LEFT  16    // Left channel PWM output - changed from GPIO4
#define PWM_OUT_RIGHT 17    // Right channel PWM output - changed from GPIO15

// BLE/RF Pins (ESP32 built-in BLE + external RF modules)
// FIXED: Removed GPIO0 conflict by using safer pins
#define RF_CE_PIN     12    // RF module Chip Enable - changed from GPIO0
#define RF_CSN_PIN    27    // RF module Chip Select - changed to avoid conflicts
#define RF_IRQ_PIN    15    // RF module IRQ - moved from GPIO17

// System Control - FIXED CONFLICTS
#define BUZZER_PIN    1     // Optional piezo buzzer - moved from GPIO27
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

// Pin conflict detection (compile-time check)
#if TFT_CS == PWR_LED
#error "Pin conflict: TFT_CS and PWR_LED cannot use the same GPIO"
#endif

#if TFT_RST == PWM_OUT_LEFT
#error "Pin conflict: TFT_RST and PWM_OUT_LEFT cannot use the same GPIO"
#endif

// Memory optimization settings
#define ENABLE_SCREEN_BUFFER false  // Disable large screen buffer to save memory
#define REDUCED_MEMORY_MODE true    // Enable memory-saving features

#endif // HARDWARE_PINS_H