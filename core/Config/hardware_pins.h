#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

// ========================================
// remu.ii Hardware Pin Configuration - CORRECTED FOR ADAFRUIT ILI9341
// ESP32 + Adafruit ILI9341 2.8" TFT + XPT2046 Touch Controller
// ========================================

// TFT Display Pins (SPI)
#define TFT_CS     5     // Chip Select - CHANGED from 15
#define TFT_DC     2     // Data/Command
#define TFT_RST    16    // Reset - CHANGED from 4
#define TFT_MOSI   23    // SPI Data Out (Master Out Slave In)
#define TFT_SCLK   18    // SPI Clock
#define TFT_MISO   19    // SPI Data In (Master In Slave Out)

// XPT2046 Touch Controller Pins (SPI-based)
#define TOUCH_CS   15    // Touch Chip Select - CHANGED from analog pins
#define TOUCH_IRQ  22    // Touch Interrupt - CHANGED from analog pins
// Touch shares SPI bus with display (MOSI, MISO, SCLK)

// 4-Wire Resistive Touch Pins (alternative to XPT2046)
// These are used by the TouchInterface for direct resistive touch reading
#define TOUCH_XP   32    // X+ (analog input / digital output)
#define TOUCH_XM   33    // X- (digital output)
#define TOUCH_YP   27    // Y+ (analog input)
#define TOUCH_YM   14    // Y- (digital output)

// SD Card Pins (shares SPI bus with display and touch)
#define SD_CS      4     // SD Card Chip Select - CHANGED from 5

// Power Management
#define BATTERY_PIN 35   // ADC pin for battery voltage monitoring
#define PWR_LED    0     // Power status LED

// Entropy Sources (floating analog pins)
#define ENTROPY_PIN_1  36   // ADC1_CH0
#define ENTROPY_PIN_2  39   // ADC1_CH3
#define ENTROPY_PIN_3  34   // ADC1_CH6

// Audio Output - I2S (recommended for best quality)
#define I2S_BCK_PIN   26    // I2S Bit Clock
#define I2S_WS_PIN    25    // I2S Word Select (LRCLK)
#define I2S_DATA_PIN  21    // I2S Data Output
// NOTE: AUDIO_AMP_EN removed due to conflict with TOUCH_YP

// Alternative: PWM Audio (backup option)
// NOTE: PWM audio pins reassigned due to conflict with 4-wire touch
#define PWM_OUT_LEFT  26    // Left channel PWM output (shared with I2S_BCK)
#define PWM_OUT_RIGHT 25    // Right channel PWM output (shared with I2S_WS)

// BLE/RF Pins (for external RF modules if needed)
#define RF_CE_PIN     12    // RF module Chip Enable
// NOTE: RF_CSN_PIN removed due to conflict with TOUCH_YM
#define RF_IRQ_PIN    13    // RF module IRQ

// System Control
#define BUZZER_PIN    17    // Optional piezo buzzer - CHANGED from 1
#define DEBUG_LED     2     // Debug LED (shared with TFT_DC - use carefully)

// Display Configuration
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 1   // Landscape orientation

// Touch Configuration (for XPT2046, not 4-wire resistive)
#define PRESSURE_THRESHOLD  200   // Minimum pressure for valid touch
#define TOUCH_DEBOUNCE_MS   50    // Debounce delay in milliseconds

// SPI Configuration
#define SPI_FREQUENCY   40000000  // 40 MHz SPI clock
#define TFT_SPI_HOST    VSPI_HOST // Use VSPI for display

// Pin conflict detection (compile-time check)
#if TFT_CS == SD_CS
#error "Pin conflict: TFT_CS and SD_CS cannot use the same GPIO"
#endif

#if TFT_CS == TOUCH_CS  
#error "Pin conflict: TFT_CS and TOUCH_CS cannot use the same GPIO"
#endif

#if SD_CS == TOUCH_CS
#error "Pin conflict: SD_CS and TOUCH_CS cannot use the same GPIO" 
#endif

// Memory optimization settings
#define ENABLE_SCREEN_BUFFER false  // Disable large screen buffer to save memory
#define REDUCED_MEMORY_MODE true    // Enable memory-saving features

#endif // HARDWARE_PINS_H
