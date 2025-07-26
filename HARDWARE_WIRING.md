# remu.ii Hardware Wiring Guide

## Components Required

### Core Components
- **ESP32 WROOM-32** development board
- **Adafruit ILI9341 2.8" TFT Display** with 4-wire resistive touch
- **MicroSD card** (up to 512GB, FAT32 formatted)
- **LiPo battery** (3.7V, 1000mAh+ recommended)
- **USB-C charging module** (optional, for FuelRod compatibility)

### Additional Components
- **Resistors**: 10kΩ (x4), 1kΩ (x2)
- **Capacitors**: 100nF ceramic (x4), 10µF electrolytic (x2)
- **Tactile switch** (power on/off)
- **LED** (3mm, power indicator)
- **Piezo buzzer** (optional, for audio feedback)
- **Breadboard or PCB** for prototyping
- **Jumper wires** (male-to-male, male-to-female)

## Pin Connections

### ESP32 to ILI9341 TFT Display (SPI)

| ESP32 Pin | ILI9341 Pin | Function | Notes |
|-----------|-------------|----------|-------|
| GPIO 23   | MOSI/SDI    | SPI Data Out | |
| GPIO 18   | SCK/CLK     | SPI Clock | |
| GPIO 19   | MISO/SDO    | SPI Data In | Optional for display |
| GPIO 15   | CS          | Chip Select | TFT_CS |
| GPIO 2    | DC/RS       | Data/Command | TFT_DC |
| GPIO 4    | RST/RESET   | Reset | TFT_RST |
| 3.3V      | VCC         | Power | |
| GND       | GND         | Ground | |
| 3.3V      | LED+        | Backlight + | Through 100Ω resistor |
| GND       | LED-        | Backlight - | |

### 4-Wire Resistive Touch Screen

| ESP32 Pin | Touch Pin | Function | Notes |
|-----------|-----------|----------|-------|
| GPIO 32   | X+        | X+ (Right) | TOUCH_XP |
| GPIO 33   | X-        | X- (Left) | TOUCH_XM |
| GPIO 14   | Y+        | Y+ (Up) | TOUCH_YP |
| GPIO 12   | Y-        | Y- (Down) | TOUCH_YM |

**Touch Wiring Notes:**
- X+ and X- are the horizontal electrodes
- Y+ and Y- are the vertical electrodes
- No external components needed for basic operation
- Add 1kΩ resistors in series for noise reduction (optional)

### SD Card (Shares SPI with Display)

| ESP32 Pin | SD Card Pin | Function | Notes |
|-----------|-------------|----------|-------|
| GPIO 23   | MOSI        | SPI Data Out | Shared with display |
| GPIO 18   | SCK         | SPI Clock | Shared with display |
| GPIO 19   | MISO        | SPI Data In | Shared with display |
| GPIO 5    | CS          | Chip Select | SD_CS |
| 3.3V      | VCC         | Power | |
| GND       | GND         | Ground | |

### Power Management

| ESP32 Pin | Component | Function | Notes |
|-----------|-----------|----------|-------|
| GPIO 35   | Battery +  | Battery Monitor | Through voltage divider |
| GPIO 1    | LED        | Power LED | PWR_LED (use with caution) |
| EN        | Switch     | Power Switch | Pull to GND to disable |

**Battery Monitoring Circuit:**
```
Battery + ----[10kΩ]---- GPIO 35 ----[10kΩ]---- GND
```
This creates a 2:1 voltage divider for safe ADC reading.

### Entropy Sources (Floating Analog Pins)

| ESP32 Pin | Function | Notes |
|-----------|----------|-------|
| GPIO 36   | Entropy 1 | Leave floating (no connection) |
| GPIO 39   | Entropy 2 | Leave floating (no connection) |
| GPIO 34   | Entropy 3 | Leave floating (no connection) |

### Audio Output (Choose ONE method)

#### Method 1: I2S DAC (Recommended for quality)
| ESP32 Pin | I2S DAC Pin | Function | Notes |
|-----------|-------------|----------|-------|
| GPIO 26   | BCK         | Bit Clock | I2S_BCK_PIN |
| GPIO 25   | WS/LRCLK    | Word Select | I2S_WS_PIN |
| GPIO 22   | DATA        | Data Output | I2S_DATA_PIN |
| GPIO 21   | EN          | Amplifier Enable | AUDIO_AMP_EN |

#### Method 2: Built-in DAC (Simpler)
| ESP32 Pin | Function | Notes |
|-----------|----------|-------|
| GPIO 25   | Left Audio | Built-in DAC1 |
| GPIO 26   | Right Audio | Built-in DAC2 |

Add RC filter: 1kΩ resistor + 10µF capacitor to ground for each channel.

#### Method 3: PWM Audio (Fallback)
| ESP32 Pin | Function | Notes |
|-----------|----------|-------|
| GPIO 4    | Left PWM | Conflicts with TFT_RST |
| GPIO 15   | Right PWM | Conflicts with TFT_CS |

**Note:** PWM audio conflicts with display pins. Choose I2S or built-in DAC.

### Optional Components

#### Piezo Buzzer
| ESP32 Pin | Component | Function | Notes |
|-----------|-----------|----------|-------|
| GPIO 27   | Buzzer +  | Buzzer Control | BUZZER_PIN |
| GND       | Buzzer -  | Ground | |

#### Debug LED
| ESP32 Pin | Component | Function | Notes |
|-----------|-----------|----------|-------|
| GPIO 2    | LED       | Debug LED | Shared with TFT_DC |

#### RF Module (Optional, for advanced features)
| ESP32 Pin | RF Module Pin | Function | Notes |
|-----------|---------------|----------|-------|
| GPIO 0    | CE            | Chip Enable | RF_CE_PIN (boot pin) |
| GPIO 16   | CSN           | Chip Select | RF_CSN_PIN |
| GPIO 17   | IRQ           | Interrupt | RF_IRQ_PIN |

## Wiring Diagrams

### Basic Breadboard Layout
```
ESP32 WROOM-32
     ┌─────────────────┐
     │  ┌─────────────┐│
     │  │    ESP32    ││
     │  │             ││
     │  │ GPIO 23 ────┼┼──── MOSI (Display & SD)
     │  │ GPIO 18 ────┼┼──── SCK  (Display & SD)
     │  │ GPIO 19 ────┼┼──── MISO (Display & SD)
     │  │ GPIO 15 ────┼┼──── CS   (Display)
     │  │ GPIO 5  ────┼┼──── CS   (SD Card)
     │  │ GPIO 2  ────┼┼──── DC   (Display)
     │  │ GPIO 4  ────┼┼──── RST  (Display)
     │  │             ││
     │  │ GPIO 32 ────┼┼──── X+   (Touch)
     │  │ GPIO 33 ────┼┼──── X-   (Touch)
     │  │ GPIO 14 ────┼┼──── Y+   (Touch)
     │  │ GPIO 12 ────┼┼──── Y-   (Touch)
     │  │             ││
     │  │ GPIO 35 ────┼┼──── Battery Monitor
     │  │ GPIO 36 ────┼┼──── (Floating - Entropy)
     │  │ GPIO 39 ────┼┼──── (Floating - Entropy)
     │  │ GPIO 34 ────┼┼──── (Floating - Entropy)
     │  │             ││
     │  │ GPIO 25 ────┼┼──── Audio Left (DAC)
     │  │ GPIO 26 ────┼┼──── Audio Right (DAC)
     │  │ GPIO 27 ────┼┼──── Buzzer
     │  └─────────────┘│
     └─────────────────┘
```

### Power Circuit
```
Battery + ──┬── ESP32 VIN
            │
            ├── [10kΩ] ── GPIO 35 ── [10kΩ] ── GND
            │
            └── Power Switch ── EN Pin

USB-C ──── Charging Module ──── Battery
```

### Touch Screen Connection Detail
```
Touch Screen (4-wire resistive)
     ┌─────────────────┐
     │                 │
  X- │ ●             ● │ X+
     │                 │
     │     Screen      │
     │                 │
  Y- │ ●             ● │ Y+
     │                 │
     └─────────────────┘
     
X- (GPIO 33) ────────────● Left electrode
X+ (GPIO 32) ────────────● Right electrode  
Y- (GPIO 12) ────────────● Bottom electrode
Y+ (GPIO 14) ────────────● Top electrode
```

## Assembly Steps

### 1. Prepare the ESP32
- Flash the remu.ii firmware before wiring
- Test basic functionality with serial monitor

### 2. Wire the Display
1. Connect power (3.3V, GND) first
2. Wire SPI connections (MOSI, SCK, MISO)
3. Connect control pins (CS, DC, RST)
4. Test display with basic graphics

### 3. Add Touch Screen
1. Identify touch electrodes on display
2. Connect 4-wire resistive touch pins
3. Test touch calibration

### 4. Install SD Card
1. Wire SD card using shared SPI bus
2. Use separate CS pin (GPIO 5)
3. Test file operations

### 5. Power System
1. Install battery monitoring circuit
2. Add power switch to EN pin
3. Connect battery and charging circuit
4. Test power management

### 6. Audio (Optional)
1. Choose audio method (I2S recommended)
2. Wire according to chosen method
3. Add output filtering if needed
4. Test audio output

### 7. Final Assembly
1. Secure all connections
2. Test all functionality
3. Calibrate touch screen
4. Load apps to SD card

## Troubleshooting

### Display Issues
- **Blank screen**: Check power and SPI connections
- **Garbled display**: Verify SPI wiring and pin assignments
- **No backlight**: Check LED+ connection and current limiting resistor

### Touch Issues
- **No touch response**: Verify 4-wire connections
- **Inaccurate touch**: Run touch calibration
- **Noisy touch**: Add series resistors to touch lines

### SD Card Issues
- **Card not detected**: Check CS pin and SPI sharing
- **File errors**: Ensure FAT32 format and proper power

### Power Issues
- **Won't boot**: Check power switch and EN pin
- **Battery not charging**: Verify charging circuit
- **Incorrect battery reading**: Check voltage divider values

### Audio Issues
- **No sound**: Verify audio method and pin assignments
- **Distorted audio**: Check filtering and grounding
- **I2S not working**: Ensure proper clock and data timing

## Safety Notes

⚠️ **Important Safety Information:**

1. **Power**: Use only 3.3V for ESP32 I/O pins
2. **Battery**: Use proper LiPo charging circuit with protection
3. **Static**: Use anti-static precautions when handling components
4. **Connections**: Double-check all wiring before powering on
5. **Heat**: Ensure adequate ventilation for continuous operation

## Performance Tips

1. **SPI Speed**: Use 40MHz SPI for optimal display performance
2. **Power**: Enable deep sleep for battery conservation
3. **Memory**: Monitor heap usage with multiple apps
4. **Touch**: Adjust pressure threshold for stylus sensitivity
5. **Audio**: Use I2S for best audio quality and CPU efficiency

## Next Steps

After successful hardware assembly:

1. Flash the complete remu.ii firmware
2. Run hardware tests using serial commands
3. Calibrate the touch screen
4. Load applications to SD card
5. Configure system settings
6. Test all applications and features

For software setup and app development, see the main README.md file.