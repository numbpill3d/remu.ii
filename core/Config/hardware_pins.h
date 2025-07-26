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
#define TOUCH_YP   14    // Y+ (Up) - moved to avoid buzzer conflict
#define TOUCH_YM   12    // Y- (Down) - moved to avoid other conflicts

// SD Card Pins (shares SPI bus with display)
#define SD_CS      5     // SD Card Chip Select

// Power Management
#define BATTERY_PIN 35   // ADC pin for battery voltage monitoring
#define PWR_LED    1     // Power status LED (TX0 pin - use with caution during programming)

// Entropy Sources (floating analog pins)
#define ENTROPY_PIN_1  36   // ADC1_CH0
#define ENTROPY_PIN_2  39   // ADC1_CH3
#define ENTROPY_PIN_3  34   // ADC1_CH6

// Audio Output (I2S DAC)
#define I2S_BCK_PIN   26    // I2S Bit Clock (moved to avoid conflict)
#define I2S_WS_PIN    25    // I2S Word Select (LRCLK, moved to avoid conflict)
#define I2S_DATA_PIN  22    // I2S Data Output (moved to avoid conflict)
#define AUDIO_AMP_EN  21    // Audio amplifier enable (moved to avoid conflict)

// Built-in DAC outputs (preferred for audio)
// Note: GPIO25/26 used for I2S when I2S mode is enabled
// ESP32 has built-in 8-bit DACs on GPIO25 and GPIO26
#define DAC_OUT_LEFT  25    // Left channel DAC output (built-in DAC1, conflicts with I2S)
#define DAC_OUT_RIGHT 26    // Right channel DAC output (built-in DAC2, conflicts with I2S)

// Alternative PWM audio (when I2S is used)
#define PWM_OUT_LEFT  4     // Left channel PWM output (shared with TFT_RST - choose one)
#define PWM_OUT_RIGHT 15    // Right channel PWM output (shared with TFT_CS - choose one)

// BLE/RF Pins (ESP32 built-in BLE + external RF modules)
// Note: ESP32 has built-in BLE, these are for external RF modules
// Using safer pins that don't conflict with boot sequence
#define RF_CE_PIN     0     // RF module Chip Enable (boot pin - use with pullup resistor)
#define RF_CSN_PIN    16    // RF module Chip Select (moved to avoid conflicts)
#define RF_IRQ_PIN    17    // RF module IRQ (moved to avoid conflicts)

// System Control
#define BUZZER_PIN    27    // Optional piezo buzzer (back to 27, touch moved)
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