# remu.ii Assembly Guide

Complete step-by-step instructions for assembling your remu.ii handheld device from components.

---

## Table of Contents

1. [Safety Warnings](#safety-warnings)
2. [Required Components](#required-components)
3. [Required Tools](#required-tools)
4. [Component Overview](#component-overview)
5. [Assembly Steps](#assembly-steps)
   - [Step 1: Prepare Components](#step-1-prepare-components)
   - [Step 2: Wire the Display](#step-2-wire-the-display)
   - [Step 3: Wire the Touch Controller](#step-3-wire-the-touch-controller)
   - [Step 4: Wire the SD Card](#step-4-wire-the-sd-card)
   - [Step 5: Wire Power System](#step-5-wire-power-system)
   - [Step 6: Wire Audio Output](#step-6-wire-audio-output)
   - [Step 7: Wire Entropy Sources](#step-7-wire-entropy-sources)
   - [Step 8: Optional Components](#step-8-optional-components)
   - [Step 9: Test Connections](#step-9-test-connections)
   - [Step 10: Final Assembly](#step-10-final-assembly)
6. [Troubleshooting](#troubleshooting)
7. [Appendix: Pin Reference](#appendix-pin-reference)

---

## Safety Warnings

⚠️ **READ BEFORE STARTING**

- **Electrical Shock**: Always disconnect power before working
- **Short Circuits**: Double-check connections before powering on
- **Battery Safety**: LiPo batteries can catch fire if damaged or short-circuited
- **Soldering**: Use proper ventilation and eye protection
- **Static Discharge**: Use ESD wrist strap when handling components
- **Sharp Objects**: Stylus and wire cutters are sharp - handle carefully

**If you are not comfortable with electronics assembly, seek assistance from an experienced person.**

---

## Required Components

### Main Components

| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| **ESP32 WROOM-32** | 4MB Flash, WiFi+BT | 1 | DevKit V1 or similar |
| **ILI9341 TFT Display** | 2.8", 320x240, SPI | 1 | Adafruit #1770 or compatible |
| **4-Wire Resistive Touch** | 2.8", matches display | 1 | Often included with display |
| **SD Card Slot** | Micro SD, SPI | 1 | Often built into display |
| **LiPo Battery** | 3.7V, 1000-2000mAh | 1 | JST connector |
| **USB-C Charging Module** | TP4056 or similar | 1 | With protection circuit |
| **Stylus** | For resistive touch | 1 | DS/3DS stylus works well |

### Additional Components

| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| **Voltage Divider Resistors** | 10kΩ, 20kΩ | 2 | For battery monitoring |
| **Capacitors (optional)** | 100nF ceramic | 3-5 | Power supply filtering |
| **Power Switch** | SPST toggle | 1 | For EN pin |
| **LED** | 3mm, any color | 1 | Power indicator |
| **Resistor for LED** | 220Ω | 1 | LED current limiting |
| **Audio Amplifier (optional)** | PAM8403 or MAX98357A | 1 | For audio output |
| **Speaker (optional)** | 8Ω, 0.5-1W | 1 | Or headphone jack |
| **Enclosure** | Custom 3D printed | 1 | Or project box |
| **Wire** | 22-28 AWG | ~2 meters | Multiple colors recommended |
| **Heat Shrink Tubing** | Assorted sizes | - | For insulation |
| **Solder** | Lead-free recommended | - | |

### Optional RF Components

| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| **nRF24L01+ Module** | 2.4GHz transceiver | 1 | For RF scanning (optional) |
| **433MHz RF Module** | RX/TX pair | 1 | For car cloner feature |
| **Antenna** | 2.4GHz or 433MHz | 1-2 | Improves RF range |

---

## Required Tools

### Essential Tools

- **Soldering Iron** (temperature controlled, 300-350°C)
- **Solder** (lead-free, 0.8mm or thinner)
- **Wire Strippers** (22-28 AWG)
- **Wire Cutters** (flush cut)
- **Multimeter** (for continuity testing)
- **Helping Hands** or PCB holder
- **Tweezers** (fine point)

### Recommended Tools

- **Hot Glue Gun** (for strain relief)
- **Heat Gun** (for heat shrink tubing)
- **Desoldering Pump** (for fixing mistakes)
- **Flux Pen** (improves solder joints)
- **ESD Wrist Strap** (prevents static damage)
- **Magnifying Glass** (for small solder joints)
- **Label Maker** (for wire identification)

### Safety Equipment

- **Safety Glasses**
- **Well-ventilated Area** or fume extractor
- **Fire Extinguisher** (for LiPo battery safety)
- **Non-conductive Work Surface**

---

## Component Overview

### ESP32 WROOM-32 Pinout

```
                    ESP32 DevKit V1
                  ┌─────────────────┐
            3V3 ──┤1              38├── GND
             EN ──┤2              37├── GPIO23 (MOSI)
         GPIO36 ──┤3  (ENTROPY1)  36├── GPIO22 (TOUCH_IRQ)
         GPIO39 ──┤4  (ENTROPY2)  35├── TXD0
         GPIO34 ──┤5  (ENTROPY3)  34├── RXD0
         GPIO35 ──┤6  (BATTERY)   33├── GPIO21 (I2S_DATA)
         GPIO32 ──┤7  (TOUCH_XP)  32├── GND
         GPIO33 ──┤8  (TOUCH_XM)  31├── GPIO19 (MISO)
         GPIO25 ──┤9  (I2S_WS)    30├── GPIO18 (SCLK)
         GPIO26 ──┤10 (I2S_BCK)   29├── GPIO5  (TFT_CS)
         GPIO27 ──┤11 (TOUCH_YP)  28├── GPIO17 (BUZZER)
         GPIO14 ──┤12 (TOUCH_YM)  27├── GPIO16 (TFT_RST)
         GPIO12 ──┤13 (RF_CE)     26├── GPIO4  (SD_CS)
            GND ──┤14             25├── GPIO0  (PWR_LED)
         GPIO13 ──┤15 (RF_IRQ)    24├── GPIO2  (TFT_DC)
          SD2   ──┤16             23├── GPIO15 (TOUCH_CS)
          SD3   ──┤17             22├── 3V3
          CMD   ──┤18             21├── GND
            5V ──┤19             20├── GPIO23 (duplicate)
                  └─────────────────┘
```

### Display Pin Functions

ILI9341 TFT pins:
- **VCC**: 3.3V power
- **GND**: Ground
- **CS**: Chip Select (GPIO5)
- **RESET**: Reset (GPIO16)
- **DC**: Data/Command (GPIO2)
- **SDI (MOSI)**: SPI Data In (GPIO23)
- **SCK**: SPI Clock (GPIO18)
- **LED**: Backlight (3.3V or PWM)
- **SDO (MISO)**: SPI Data Out (GPIO19)

### Touch Controller Pins

4-wire resistive touch:
- **X+**: TOUCH_XP (GPIO32)
- **X-**: TOUCH_XM (GPIO33)
- **Y+**: TOUCH_YP (GPIO27)
- **Y-**: TOUCH_YM (GPIO14)

*Note: These are analog pins that will be switched between input and output modes.*

---

## Assembly Steps

### Step 1: Prepare Components

#### 1.1 Inventory Check

Lay out all components and verify against the parts list. Check for:
- Damaged components
- Correct values (resistors, capacitors)
- All pins present on modules

#### 1.2 Pre-Test ESP32

Before assembly, test ESP32:

1. Connect ESP32 to computer via USB
2. Upload blink sketch to verify functionality
3. Check voltage at 3V3 pin (should be 3.2-3.4V)
4. Disconnect USB

#### 1.3 Prepare Wires

Cut wires to appropriate lengths:
- **Power/Ground**: Red/Black, ~10cm each (multiple)
- **SPI Bus**: 6 wires, ~8cm each (can use ribbon cable)
- **Touch**: 4 wires, ~6cm each
- **Misc**: Various colors, various lengths

Strip ~3mm from each end. Pre-tin ends if desired.

#### 1.4 Label Pins (Optional)

Use masking tape and marker to label:
- GPIO numbers on ESP32
- Pin functions on display module
- Wire purposes (e.g., "MOSI", "GND")

---

### Step 2: Wire the Display

⚠️ **IMPORTANT**: Double-check connections before powering on. Incorrect wiring can damage components.

#### 2.1 SPI Bus Connections

Connect display to ESP32 using the SPI bus:

| Display Pin | ESP32 GPIO | Wire Color (suggested) |
|-------------|------------|------------------------|
| **VCC** | 3V3 | Red |
| **GND** | GND | Black |
| **CS** | GPIO5 (TFT_CS) | Orange |
| **RESET** | GPIO16 (TFT_RST) | Yellow |
| **DC** | GPIO2 (TFT_DC) | Green |
| **SDI (MOSI)** | GPIO23 | Blue |
| **SCK** | GPIO18 | Purple |
| **LED** | 3V3 | Red (or PWM pin for dimming) |
| **SDO (MISO)** | GPIO19 | Gray |

#### 2.2 Soldering Tips

1. **Secure components** in helping hands or vise
2. **Heat pad and pin** simultaneously
3. **Apply solder** to joint (not iron tip)
4. **Remove iron** and let cool without movement
5. **Inspect joint**: Should be shiny and cone-shaped
6. **Test continuity** with multimeter

#### 2.3 Strain Relief

Apply hot glue or use heat shrink tubing at solder joints to prevent wires from breaking.

#### 2.4 Test Display (Optional)

Before continuing, you can test the display:
1. Upload test sketch with display initialization
2. Display should show test pattern
3. If not working, check connections and solder joints

---

### Step 3: Wire the Touch Controller

#### 3.1 Identify Touch Pins

The 4-wire resistive touch screen has 4 pins:
- **X+**: Usually marked or leftmost
- **X-**: Opposite side from X+
- **Y+**: Top or bottom edge
- **Y-**: Opposite from Y+

*Note: If unmarked, you may need to test with multimeter to identify.*

#### 3.2 Touch Connections

| Touch Pin | ESP32 GPIO | Wire Color |
|-----------|------------|------------|
| **X+** | GPIO32 (TOUCH_XP) | White |
| **X-** | GPIO33 (TOUCH_XM) | Brown |
| **Y+** | GPIO27 (TOUCH_YP) | Pink |
| **Y-** | GPIO14 (TOUCH_YM) | Gray |

#### 3.3 Soldering Touch Pins

1. Touch pins are usually small and fragile
2. Use fine solder (0.5mm recommended)
3. Apply minimal heat to avoid damaging touch film
4. Test continuity after soldering

---

### Step 4: Wire the SD Card

Most ILI9341 displays have built-in SD card slots that share the SPI bus.

#### 4.1 SD Card Connections

| SD Card Pin | ESP32 GPIO | Wire Color |
|-------------|------------|------------|
| **CS** | GPIO4 (SD_CS) | Brown |
| **MOSI** | GPIO23 | Blue (shared with display) |
| **MISO** | GPIO19 | Gray (shared with display) |
| **SCK** | GPIO18 | Purple (shared with display) |
| **VCC** | 3V3 | Red (shared) |
| **GND** | GND | Black (shared) |

#### 4.2 Notes

- **SD card shares SPI bus** with display, so only CS pin needs separate wire
- Some displays have SD on separate pins - check your module
- Ensure connections are solid for reliable SD card access

---

### Step 5: Wire Power System

⚠️ **BATTERY SAFETY**:
- Never short LiPo battery terminals
- Use protection circuit (TP4056 module recommended)
- Keep away from heat and puncture
- Store in fireproof bag

#### 5.1 Battery Charging Circuit

**Using TP4056 Module**:

1. **Connect Battery to TP4056**:
   - Battery **+** → TP4056 **B+**
   - Battery **-** → TP4056 **B-**

2. **Connect USB-C to TP4056**:
   - USB-C **+5V** → TP4056 **IN+**
   - USB-C **GND** → TP4056 **IN-**

3. **Connect Output to ESP32**:
   - TP4056 **OUT+** → Power Switch → ESP32 **5V** or **3V3** pin
   - TP4056 **OUT-** → ESP32 **GND**

*Note: Some TP4056 modules output ~4.2V (battery voltage). You can connect to ESP32 5V pin (has regulator) or use a 3.3V regulator.*

#### 5.2 Power Switch Wiring

1. **Connect switch** between TP4056 OUT+ and ESP32 power:
   ```
   TP4056 OUT+ → Switch Pin 1
   Switch Pin 2 → ESP32 5V (or EN pin for soft power)
   ```

2. **Alternative (EN pin method)**:
   ```
   Connect 10kΩ pullup resistor: ESP32 EN → 3V3
   Connect switch: ESP32 EN → GND (pulls low to disable)
   ```

#### 5.3 Battery Monitor Circuit

To monitor battery voltage, we need a voltage divider (ESP32 ADC max is 3.3V):

**Components**:
- R1: 20kΩ resistor
- R2: 10kΩ resistor

**Wiring**:
```
Battery + → R1 (20kΩ) → ESP32 GPIO35 (BATTERY_PIN) → R2 (10kΩ) → GND
```

This divides the ~4.2V battery voltage to ~1.4V (safe for ADC).

#### 5.4 Power LED

**Wiring**:
```
ESP32 GPIO0 (PWR_LED) → 220Ω Resistor → LED Anode (+)
LED Cathode (-) → GND
```

---

### Step 6: Wire Audio Output

Two options: I2S DAC (better quality) or PWM (simple).

#### Option A: I2S DAC (MAX98357A or similar)

| MAX98357A Pin | ESP32 GPIO | Notes |
|---------------|------------|-------|
| **LRC (WS)** | GPIO25 (I2S_WS) | Word select |
| **BCLK** | GPIO26 (I2S_BCK) | Bit clock |
| **DIN** | GPIO21 (I2S_DATA) | Data in |
| **GAIN** | GND | Sets gain (see datasheet) |
| **SD** | 3V3 | Shutdown (high = enabled) |
| **VIN** | 5V | Power (or 3V3) |
| **GND** | GND | Ground |
| **Speaker+** | → Speaker + | |
| **Speaker-** | → Speaker - | |

#### Option B: PWM Audio (Simple)

Use the built-in ESP32 DAC (GPIO25 and GPIO26):

**For Mono**:
```
ESP32 GPIO25 → 100μF Capacitor → 10kΩ Resistor → 3.5mm Jack Tip
                                                  Jack Sleeve → GND
```

**For Stereo** (not currently used):
```
Left:  GPIO25 → Capacitor/Resistor → Jack Left
Right: GPIO26 → Capacitor/Resistor → Jack Right
```

#### 6.3 Speaker Connection

**8Ω Speaker**:
- Connect speaker to amplifier output (Option A)
- Or connect to DAC output through capacitor (Option B)
- Add volume potentiometer if desired (between DAC and speaker)

---

### Step 7: Wire Entropy Sources

The entropy system uses 3 floating (unconnected) analog pins to generate random numbers.

#### 7.1 Entropy Pin Connections

**These pins should NOT be connected to anything!**

| ESP32 GPIO | Function | Notes |
|------------|----------|-------|
| **GPIO36** | ENTROPY_PIN_1 | Leave floating (no wire) |
| **GPIO39** | ENTROPY_PIN_2 | Leave floating |
| **GPIO34** | ENTROPY_PIN_3 | Leave floating |

#### 7.2 Optional: Enhance Randomness

For better entropy collection (optional):
1. Attach short (~5cm) wire antenna to each pin (acts as noise antenna)
2. Keep wires separated to pick up different noise sources
3. **Do not connect to any voltage source or ground!**

---

### Step 8: Optional Components

#### 8.1 Buzzer (Optional)

For audio feedback/alerts:

**Wiring**:
```
ESP32 GPIO17 (BUZZER_PIN) → Buzzer +
Buzzer - → GND
```

Use a passive buzzer (driven by PWM), not active buzzer.

#### 8.2 RF Module (Optional)

For car cloner / RF tools feature:

**nRF24L01+ Wiring** (if using for RF scanning):

| nRF24L01+ Pin | ESP32 GPIO |
|---------------|------------|
| **CE** | GPIO12 (RF_CE_PIN) |
| **CSN** | Not available due to conflict - reassign |
| **SCK** | GPIO18 (shared SPI) |
| **MOSI** | GPIO23 (shared SPI) |
| **MISO** | GPIO19 (shared SPI) |
| **IRQ** | GPIO13 (RF_IRQ_PIN) |
| **VCC** | 3V3 |
| **GND** | GND |

*Note: Due to pin conflicts with touch, RF module support may require reassigning pins.*

#### 8.3 External Antenna (Optional)

For better WiFi/BLE range:
- Connect external 2.4GHz antenna to ESP32 U.FL connector (if available)
- Or use ESP32 module with external antenna support

---

### Step 9: Test Connections

Before final assembly, test all connections:

#### 9.1 Continuity Tests

Use multimeter in continuity mode:
1. Test all GND connections (should beep)
2. Test all 3V3 connections (should beep)
3. **Verify NO continuity** between 3V3 and GND (should NOT beep!)

#### 9.2 Voltage Tests

With battery connected (or USB power):
1. Measure 3V3 pin: Should be 3.2-3.4V
2. Measure 5V pin (if using): Should be 4.8-5.2V
3. Measure battery voltage at GPIO35: Should be ~1.0-1.4V (divided)

#### 9.3 Visual Inspection

Check for:
- Cold solder joints (dull, grainy appearance)
- Solder bridges between adjacent pins
- Loose wires
- Exposed conductors that might short

#### 9.4 Initial Power-On Test

1. **Disconnect battery**
2. **Connect USB-C** to computer
3. **Power LED should light** (if wired)
4. **Upload test firmware** (display test)
5. **Verify display works**
6. **Test touch** (calibration will be needed)
7. **Test SD card** (insert and check detection)

#### 9.5 Battery Test

1. **Disconnect USB**
2. **Connect battery** (if not already connected)
3. **Turn on power switch**
4. **Device should boot** from battery power
5. **Connect USB** to test charging (charging LED should light)

---

### Step 10: Final Assembly

#### 10.1 Secure Components

**Using Hot Glue**:
1. Apply hot glue to back of display module
2. Press onto mounting surface or enclosure
3. Glue ESP32 in position
4. Glue battery compartment (allow for replacement)
5. **Do not glue battery directly** - use velcro or holder

**Using Mounting Holes**:
1. Use M2.5 or M3 screws for ESP32 DevKit
2. Use appropriate screws for display (usually M2)
3. Use standoffs to prevent shorts

#### 10.2 Cable Management

1. **Bundle SPI wires** together with zip ties or sleeve
2. **Separate power wires** from signal wires (reduce noise)
3. **Secure loose wires** with hot glue or cable clips
4. **Label important wires** for future maintenance

#### 10.3 Enclosure

**3D Printed Case** (recommended):
1. Design or download STL files for remu.ii case
2. Print in PETG or ABS for durability
3. Include:
   - Display window
   - SD card access
   - USB-C port access
   - Power switch access
   - Speaker grille (if using audio)
   - Stylus holder

**Project Box** (alternative):
1. Drill holes for display, USB, switch, etc.
2. Mount components with screws or glue
3. Cut display window (or use transparent lid)

#### 10.4 Final Checks

Before closing enclosure:
- [ ] All connections secure
- [ ] No loose wires that could short
- [ ] Battery properly secured
- [ ] SD card accessible
- [ ] USB-C port accessible
- [ ] Power switch accessible
- [ ] Display visible through window
- [ ] Touch area accessible with stylus

---

## Troubleshooting

### Display Issues

#### Display shows nothing (black screen)

**Possible Causes**:
- No power to display (check VCC and GND)
- Backlight not connected (check LED pin)
- Wrong pin configuration (verify pin definitions)
- Bad solder joint on SPI pins

**Solutions**:
1. Check continuity on all display connections
2. Measure voltage at display VCC (should be 3.3V)
3. Test backlight separately (connect LED pin to 3.3V)
4. Verify SPI pins match code configuration

#### Display shows garbage/random pixels

**Possible Causes**:
- SPI wiring incorrect (swapped MOSI/MISO)
- Loose connection on SPI bus
- Wrong SPI frequency (too high)

**Solutions**:
1. Verify MOSI goes to SDI, MISO goes to SDO
2. Check all SPI connections for cold solder joints
3. Reduce SPI frequency in code temporarily

#### Colors are wrong

**Possible Causes**:
- Color mode mismatch in code
- Display variant difference (ILI9341 vs ILI9340)

**Solutions**:
1. Try different color modes in display initialization
2. Check display chip version

### Touch Issues

#### Touch not responding at all

**Possible Causes**:
- Touch pins not connected
- Wrong pin configuration
- Touch controller damaged

**Solutions**:
1. Verify all 4 touch pins are connected
2. Check pin definitions match wiring
3. Test touch controller with multimeter (resistance between X+/X- and Y+/Y- should change when pressed)

#### Touch inaccurate/offset

**Possible Causes**:
- Not calibrated
- Wrong touch pin configuration
- Interference from display

**Solutions**:
1. Run touch calibration routine
2. Verify X+, X-, Y+, Y- are connected correctly
3. Add 100nF capacitors between touch pins and GND

### Power Issues

#### Device won't turn on

**Possible Causes**:
- Battery dead
- Power switch off
- Bad connection to ESP32
- Short circuit (fuse blown in TP4056)

**Solutions**:
1. Charge battery via USB-C
2. Check switch position
3. Bypass switch temporarily (connect directly)
4. Check for short circuits with multimeter
5. Measure battery voltage (should be >3.0V)

#### Battery drains too fast

**Possible Causes**:
- WiFi/BLE constantly scanning
- Display brightness too high
- Short circuit (parasitic drain)

**Solutions**:
1. Disable WiFi/BLE when not in use
2. Lower display brightness
3. Measure current draw with multimeter (should be <200mA idle)
4. Check for shorts or LEDs left on

#### Device won't charge

**Possible Causes**:
- USB cable is charge-only (no data)
- TP4056 module damaged
- Battery protection circuit tripped

**Solutions**:
1. Try different USB cable
2. Check voltage at TP4056 input (should be ~5V)
3. Measure battery voltage directly (if <2.5V, protection may be tripped)

### SD Card Issues

#### SD card not detected

**Possible Causes**:
- SD card not formatted as FAT32
- SD_CS pin incorrect
- SPI bus conflict
- Bad SD card

**Solutions**:
1. Format SD card as FAT32
2. Verify SD_CS pin definition
3. Test SD card in computer
4. Try different SD card
5. Check SPI bus wiring

### Audio Issues

#### No audio output

**Possible Causes**:
- Amplifier not powered
- Speaker not connected
- Audio sample not loaded

**Solutions**:
1. Check amplifier power (should have 3.3V or 5V)
2. Test speaker with battery directly (should click)
3. Verify I2S pin connections
4. Check SD card has audio samples

---

## Appendix: Pin Reference

### Complete Pin Assignment Table

| GPIO | Function | Direction | Connection |
|------|----------|-----------|------------|
| 0 | PWR_LED | Output | LED via 220Ω resistor |
| 2 | TFT_DC | Output | Display D/C pin |
| 4 | SD_CS | Output | SD card chip select |
| 5 | TFT_CS | Output | Display chip select |
| 12 | RF_CE_PIN | Output | RF module CE (optional) |
| 13 | RF_IRQ_PIN | Input | RF module IRQ (optional) |
| 14 | TOUCH_YM | I/O | Touch Y- |
| 15 | TOUCH_CS | Output | Touch chip select (XPT2046) |
| 16 | TFT_RST | Output | Display reset |
| 17 | BUZZER_PIN | Output | Buzzer (optional) |
| 18 | SCLK | Output | SPI clock (shared) |
| 19 | MISO | Input | SPI data in (shared) |
| 21 | I2S_DATA_PIN | Output | I2S audio data |
| 22 | TOUCH_IRQ | Input | Touch interrupt (XPT2046) |
| 23 | MOSI | Output | SPI data out (shared) |
| 25 | I2S_WS_PIN | Output | I2S word select |
| 26 | I2S_BCK_PIN | Output | I2S bit clock |
| 27 | TOUCH_YP | I/O | Touch Y+ |
| 32 | TOUCH_XP | I/O | Touch X+ |
| 33 | TOUCH_XM | I/O | Touch X- |
| 34 | ENTROPY_PIN_3 | Input | Floating (entropy source) |
| 35 | BATTERY_PIN | Input | Battery voltage monitor |
| 36 | ENTROPY_PIN_1 | Input | Floating (entropy source) |
| 39 | ENTROPY_PIN_2 | Input | Floating (entropy source) |

### Power Pins

| Pin | Voltage | Purpose |
|-----|---------|---------|
| 3V3 | 3.3V | Power for display, SD, touch, sensors |
| 5V | 5V | Input from USB or battery (via regulator) |
| GND | 0V | Common ground |
| EN | 3.3V | Enable (pulled high to run, low to reset) |

---

## Maintenance and Upgrades

### Routine Maintenance

- **Check solder joints** periodically for cracks
- **Clean display** with microfiber cloth
- **Test battery** capacity every 6 months
- **Backup SD card** data regularly

### Possible Upgrades

- **Larger battery** for extended runtime
- **Better speaker** or headphone amp
- **External antenna** for WiFi/BLE
- **Haptic feedback** motor
- **RGB LED** for status indication
- **Custom enclosure** with ergonomic design

---

## Next Steps

After assembly:

1. **Upload firmware** (see INSTALLATION_GUIDE.md)
2. **Calibrate touch** screen
3. **Test all features**:
   - Display graphics
   - Touch response
   - SD card access
   - WiFi scanning
   - BLE scanning
   - Audio output
   - Battery monitoring
4. **Read USER_MANUAL.md** for usage instructions

---

## Getting Help

If you encounter assembly issues:

- **Documentation**: Check `/Documentation/` folder
- **GitHub Issues**: https://github.com/numbpill3d/remu.ii/issues
- **Community Forum**: (TBD)

When asking for help, include:
- Photos of your wiring
- Multimeter measurements
- Error messages from serial monitor
- Component specifications

---

**Congratulations on building your remu.ii device!**

*Now go forth and hack responsibly.*

---

**Last Updated**: 2025-11-12
**Hardware Version**: 1.0
**Assembly Difficulty**: Intermediate to Advanced
