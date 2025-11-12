# remu.ii Installation Guide

Complete guide for setting up the development environment, compiling, and uploading firmware to your remu.ii device.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Software Installation](#software-installation)
   - [Arduino IDE Setup](#arduino-ide-setup)
   - [ESP32 Board Support](#esp32-board-support)
   - [Required Libraries](#required-libraries)
3. [Hardware Setup](#hardware-setup)
4. [Compiling the Firmware](#compiling-the-firmware)
5. [Uploading to Device](#uploading-to-device)
6. [First Boot Configuration](#first-boot-configuration)
7. [Troubleshooting](#troubleshooting)
8. [Advanced Configuration](#advanced-configuration)

---

## Prerequisites

### Hardware Requirements

- **remu.ii device** (assembled - see ASSEMBLY_GUIDE.md)
- **USB-C cable** (data-capable, not charge-only)
- **Computer** running Windows, macOS, or Linux
- **SD card** (4GB-512GB, FAT32 formatted)

### Software Requirements

- **Arduino IDE** 1.8.19 or newer (or Arduino IDE 2.x)
- **ESP32 Board Support** version 2.x
- **USB-to-Serial drivers** (usually auto-installed)
- **Git** (optional, for cloning repository)

### Knowledge Requirements

- Basic familiarity with Arduino IDE
- Understanding of USB device drivers
- Basic command line usage (optional)

---

## Software Installation

### Arduino IDE Setup

#### Windows

1. Download Arduino IDE from https://www.arduino.cc/en/software
2. Run the installer (`arduino-<version>-windows.exe`)
3. Follow installation wizard (default options recommended)
4. Launch Arduino IDE

#### macOS

1. Download Arduino IDE from https://www.arduino.cc/en/software
2. Open the `.dmg` file
3. Drag Arduino to Applications folder
4. Launch Arduino IDE (allow security permission if prompted)

#### Linux

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install arduino

# Or download from arduino.cc for latest version
wget https://downloads.arduino.cc/arduino-<version>-linux64.tar.xz
tar -xf arduino-<version>-linux64.tar.xz
cd arduino-<version>
sudo ./install.sh
```

### ESP32 Board Support

#### Method 1: Board Manager (Recommended)

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click **OK**
5. Go to **Tools → Board → Boards Manager**
6. Search for **"esp32"**
7. Install **"esp32 by Espressif Systems"** version **2.0.x** or newer
8. Wait for installation to complete (may take several minutes)

#### Method 2: Manual Installation

```bash
# Linux/macOS
cd ~/Arduino/hardware
mkdir -p espressif
cd espressif
git clone https://github.com/espressif/arduino-esp32.git esp32
cd esp32
git submodule update --init --recursive
cd tools
python3 get.py

# Windows (PowerShell)
cd $HOME\Documents\Arduino\hardware
mkdir espressif
cd espressif
git clone https://github.com/espressif/arduino-esp32.git esp32
cd esp32
git submodule update --init --recursive
cd tools
python get.py
```

### Required Libraries

Install these libraries via **Tools → Manage Libraries** (Library Manager):

#### Core Libraries

1. **Adafruit GFX Library**
   - Search: "Adafruit GFX"
   - Version: 1.11.0 or newer
   - Install all dependencies when prompted

2. **Adafruit ILI9341**
   - Search: "Adafruit ILI9341"
   - Version: 1.6.0 or newer
   - Install dependencies (Adafruit BusIO, etc.)

3. **ArduinoJson**
   - Search: "ArduinoJson"
   - Version: 6.21.0 or newer
   - **Important**: Use version 6.x, NOT 7.x

#### Additional Libraries (Already Included)

These are part of ESP32 Arduino Core:
- WiFi (built-in)
- BLE Device (built-in)
- SD (built-in)
- SPI (built-in)
- EEPROM (built-in)
- esp_task_wdt (built-in)

### Verifying Installation

1. Open Arduino IDE
2. Go to **Tools → Board**
3. You should see **ESP32 Arduino** section with board options
4. Go to **Sketch → Include Library**
5. Verify **Adafruit GFX**, **Adafruit ILI9341**, and **ArduinoJson** are listed

---

## Hardware Setup

### Connecting remu.ii to Computer

1. **Ensure device is powered off**
2. **Connect USB-C cable** from remu.ii to computer
3. **Wait for drivers to install** (Windows may take a minute)
4. **Device should appear as COM port** (Windows) or `/dev/ttyUSB*` (Linux) or `/dev/cu.usbserial*` (macOS)

### Checking Connection

#### Windows

1. Open **Device Manager**
2. Expand **Ports (COM & LPT)**
3. Look for **Silicon Labs CP210x** or **USB Serial Port (COMx)**
4. Note the COM port number (e.g., COM3)

#### macOS

```bash
ls /dev/cu.*
# Look for /dev/cu.usbserial-XXXXXXXX or /dev/cu.SLAB_USBtoUART
```

#### Linux

```bash
ls /dev/ttyUSB*
# Should show /dev/ttyUSB0 or similar

# If permission denied, add user to dialout group:
sudo usermod -a -G dialout $USER
# Then log out and log back in
```

### Installing USB Drivers (If Needed)

If device is not detected:

#### Windows

1. Download **CP210x USB to UART Bridge drivers** from:
   https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
2. Install and restart computer
3. Reconnect device

#### macOS

Usually auto-installed. If not:
```bash
# Install via Homebrew
brew tap homebrew/cask-drivers
brew install silicon-labs-vcp-driver
```

#### Linux

Usually works out of the box. If not:
```bash
# Ubuntu/Debian
sudo apt install python3-serial
```

---

## Compiling the Firmware

### Opening the Project

#### Option 1: Clone from GitHub

```bash
git clone https://github.com/numbpill3d/remu.ii.git
cd remu.ii
# Open remu_ii.ino in Arduino IDE
```

#### Option 2: Download ZIP

1. Download from GitHub: https://github.com/numbpill3d/remu.ii
2. Extract to a folder named **`remu.ii`**
3. Open **`remu.ii/remu_ii.ino`** in Arduino IDE

### Configuring Board Settings

1. **Select Board**: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
2. **Configure settings** as follows:

| Setting | Value |
|---------|-------|
| **Upload Speed** | 921600 |
| **CPU Frequency** | 240MHz (WiFi/BT) |
| **Flash Frequency** | 80MHz |
| **Flash Mode** | DIO |
| **Flash Size** | 4MB (32Mb) |
| **Partition Scheme** | Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS) |
| **Core Debug Level** | None (or "Info" for debugging) |
| **PSRAM** | Disabled (unless you have PSRAM module) |
| **Port** | Select your COM/ttyUSB port |

### Verifying Compilation

1. Click **Verify** button (✓) in Arduino IDE
2. Wait for compilation to complete (may take 1-3 minutes)
3. Check for success message: **"Done compiling"**
4. Compilation output should show:
   ```
   Sketch uses XXXXX bytes (XX%) of program storage space
   Global variables use XXXXX bytes (XX%) of dynamic memory
   ```

### Common Compilation Errors

#### "Adafruit_GFX.h: No such file or directory"
- **Solution**: Install Adafruit GFX Library via Library Manager

#### "ArduinoJson.h: No such file or directory"
- **Solution**: Install ArduinoJson version 6.x via Library Manager

#### "esp_task_wdt.h: No such file or directory"
- **Solution**: Ensure ESP32 board support is installed (version 2.x)

#### "Multiple libraries found for..."
- **Solution**: Remove duplicate library installations, keep only one version

#### Out of memory during compilation
- **Solution**: Close other programs, restart Arduino IDE, try again

---

## Uploading to Device

### Pre-Upload Checklist

- [ ] Device connected via USB-C
- [ ] Correct COM/serial port selected
- [ ] ESP32 Dev Module board selected
- [ ] Upload speed set to 921600
- [ ] Compilation successful (no errors)

### Upload Procedure

#### Standard Upload

1. **Click Upload button** (→) in Arduino IDE
2. Wait for "Connecting..." message
3. **If upload doesn't start**, press and hold **BOOT button** on ESP32, then press **EN button** briefly
4. Release BOOT button when "Uploading..." appears
5. Wait for upload to complete (30-60 seconds)
6. Device will automatically restart when done

#### Upload Progress

You should see:
```
Connecting.....
Chip is ESP32-D0WDQ6 (revision 1)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz
MAC: xx:xx:xx:xx:xx:xx
Uploading stub...
Running stub...
Stub running...
Writing at 0x00010000... (X %)
...
Hard resetting via RTS pin...
```

#### Successful Upload

- **LED will blink** during upload
- **Serial monitor** will show boot messages (if opened)
- **Device will restart** automatically
- **Display will show** remu.ii boot screen

### Upload Troubleshooting

#### "Failed to connect to ESP32"

**Solutions**:
1. Press and hold BOOT button during upload
2. Try lower upload speed (115200)
3. Check USB cable (must be data-capable)
4. Try different USB port
5. Restart device and try again

#### "A fatal error occurred: Timed out waiting for packet header"

**Solutions**:
1. Lower upload speed to 460800 or 115200
2. Ensure device is in bootloader mode (hold BOOT)
3. Check USB cable quality

#### "Serial port COMx not found"

**Solutions**:
1. Reconnect USB cable
2. Install CP210x drivers
3. Check Device Manager (Windows) or `dmesg` (Linux)
4. Try different USB port

#### "Permission denied" (Linux)

**Solutions**:
```bash
sudo usermod -a -G dialout $USER
# Log out and log back in
# Or temporarily:
sudo chmod 666 /dev/ttyUSB0
```

---

## First Boot Configuration

### Initial Boot Sequence

After successful upload:

1. **Device powers on** (LED lights up)
2. **Serial output** shows initialization:
   ```
   [SystemCore] Initializing...
   [DisplayManager] Display initialized
   [TouchInterface] Touch interface initialized
   [FileSystem] SD card mounted successfully
   [AppManager] Registered 7 applications
   [SystemCore] Initialization complete
   ```
3. **Display shows** boot screen, then app launcher
4. **Touch calibration** may be required on first boot

### Serial Monitor Setup

To view debug output:

1. **Open Serial Monitor**: **Tools → Serial Monitor**
2. **Set baud rate**: **115200** (bottom-right dropdown)
3. **Set line ending**: **Both NL & CR**
4. You should see system messages and logs

### SD Card Preparation

1. **Format SD card** as FAT32:
   - **Windows**: Right-click card → Format → FAT32
   - **macOS**: Disk Utility → Erase → MS-DOS (FAT)
   - **Linux**: `sudo mkfs.vfat -F 32 /dev/sdX1`

2. **Insert card** into remu.ii SD slot (under display)
3. **Restart device**
4. Device will auto-create directory structure:
   ```
   /apps/
   /data/
   /samples/
   /settings/
   /temp/
   /logs/
   ```

### Touch Calibration

If touch is inaccurate:

1. Navigate to **Settings** app (may be difficult if uncalibrated)
2. Select **Touch Calibration**
3. Tap calibration targets **precisely** with stylus
4. Calibration saved to EEPROM
5. Restart device

#### Manual Calibration (if UI inaccessible)

Via Serial Monitor:
```
Type: calibrate
```
Follow on-screen prompts.

---

## Troubleshooting

### Device Won't Boot

**Symptoms**: Black screen, no LED, no serial output

**Solutions**:
1. Check battery charge (connect USB-C)
2. Press EN button to reset
3. Re-upload firmware
4. Check power connections (if DIY assembly)

### Display Shows Garbage/Noise

**Symptoms**: Random pixels, incorrect colors

**Solutions**:
1. Check display connections (SPI pins)
2. Verify `hardware_pins.h` pin definitions match wiring
3. Try reducing SPI frequency in code
4. Check for loose wires

### Touch Not Working

**Symptoms**: No response to stylus input

**Solutions**:
1. Verify touch controller connections
2. Run touch calibration
3. Check `TOUCH_XP`, `TOUCH_XM`, `TOUCH_YP`, `TOUCH_YM` pin definitions
4. Ensure using stylus (not finger)

### SD Card Not Detected

**Symptoms**: File operations fail, "SD card not found"

**Solutions**:
1. Format card as FAT32 (not exFAT or NTFS)
2. Check SD_CS pin definition
3. Verify SD card inserted properly
4. Try different SD card (max 512GB)
5. Check SPI bus connections

### WiFi Not Working

**Symptoms**: Networks not found, scan fails

**Solutions**:
1. Check antenna connection (if external)
2. Verify ESP32 module has WiFi (not all do)
3. Check WiFi is enabled in code
4. Distance from router may be too far

### Out of Memory Errors

**Symptoms**: Apps crash, "Low heap warning"

**Solutions**:
1. Reduce `TARGET_FPS` in Config.h
2. Enable `REDUCED_MEMORY_MODE`
3. Disable screen buffer
4. Return to launcher before launching new app

### Compilation Errors After Update

**Solutions**:
1. Update all libraries to latest versions
2. Clear Arduino IDE cache:
   - **Windows**: Delete `C:\Users\<name>\AppData\Local\Temp\arduino_*`
   - **macOS**: Delete `~/Library/Arduino15/tmp/`
   - **Linux**: Delete `~/.arduino15/tmp/`
3. Restart Arduino IDE

---

## Advanced Configuration

### Customizing Pin Assignments

Edit `/core/Config/hardware_pins.h`:

```cpp
// Example: Change TFT chip select pin
#define TFT_CS     5     // Change to desired GPIO number
```

**Important**: After changing pins:
1. Verify no conflicts with other pins
2. Update wiring accordingly
3. Recompile and upload firmware

### Adjusting Memory Settings

Edit `/core/Config.h`:

```cpp
// Reduce frame rate to save memory
#define TARGET_FPS 20  // Lower = less memory usage

// Enable memory saving mode
#define REDUCED_MEMORY_MODE true

// Disable screen buffer
#define ENABLE_SCREEN_BUFFER false
```

### Enabling Debug Output

In Board Settings:
1. **Tools → Core Debug Level → "Info"** (or "Debug" for verbose)
2. Recompile and upload
3. Open Serial Monitor to view debug logs

In code:
```cpp
#define DEBUG_MODE true  // Add to Config.h
```

### Changing Partition Scheme

For more app storage:
1. **Tools → Partition Scheme → "Huge APP (3MB No OTA/1MB SPIFFS)"**
2. Recompile and upload (will erase all data!)

### OTA (Over-The-Air) Updates

Not currently implemented, but planned for future release.

---

## Building from Source

### Prerequisites

- **Git**: https://git-scm.com/
- **Python 3.7+**: https://www.python.org/
- **PlatformIO** (optional, advanced): https://platformio.org/

### Clone Repository

```bash
git clone https://github.com/numbpill3d/remu.ii.git
cd remu.ii
```

### Using PlatformIO (Advanced)

```bash
# Install PlatformIO
pip install platformio

# Build
pio run

# Upload
pio run --target upload

# Monitor
pio device monitor
```

`platformio.ini` configuration:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    adafruit/Adafruit GFX Library@^1.11.0
    adafruit/Adafruit ILI9341@^1.6.0
    bblanchon/ArduinoJson@^6.21.0
monitor_speed = 115200
upload_speed = 921600
```

---

## Next Steps

After successful installation:

1. **Read USER_MANUAL.md** for usage instructions
2. **Explore applications** in the app launcher
3. **Load samples** onto SD card for Sequencer app
4. **Configure WiFi** for network scanning
5. **Join the community** and share your projects!

---

## Getting Help

### Resources

- **Documentation**: `/Documentation/` folder on SD card
- **GitHub Issues**: https://github.com/numbpill3d/remu.ii/issues
- **Serial Monitor**: For real-time debugging
- **Community Forum**: (TBD)

### Reporting Bugs

When reporting issues, include:
- Arduino IDE version
- ESP32 board support version
- Library versions
- Full error message
- Serial monitor output
- Steps to reproduce

---

**Happy hacking!**

*If you encounter issues not covered here, please open an issue on GitHub.*

---

**Last Updated**: 2025-11-12
**Firmware Version**: 1.0
**Compatible Hardware**: ESP32 WROOM-32
