# remu.ii User Manual

## Welcome to remu.ii

**remu.ii** is a handheld multifaceted modular device that combines entertainment, music production, security research, and metaphysical exploration into a single portable platform. Powered by an ESP32 microcontroller and featuring a stylus-based touchscreen interface, remu.ii is your companion for hacking, creativity, and anomaly detection.

---

## Table of Contents

1. [Device Overview](#device-overview)
2. [Getting Started](#getting-started)
3. [Interface Guide](#interface-guide)
4. [Applications](#applications)
   - [Digital Pet](#digital-pet)
   - [Sequencer](#sequencer)
   - [WiFi Tools](#wifi-tools)
   - [BLE Scanner](#ble-scanner)
   - [Car Cloner (RF Tools)](#car-cloner-rf-tools)
   - [Frequency Scanner](#frequency-scanner)
   - [Entropy Beacon](#entropy-beacon)
5. [System Settings](#system-settings)
6. [File Management](#file-management)
7. [Battery & Power Management](#battery--power-management)
8. [Troubleshooting](#troubleshooting)
9. [Safety & Legal Information](#safety--legal-information)

---

## Device Overview

### Hardware Specifications

- **Microcontroller**: ESP32 WROOM-32 (dual-core, 240MHz)
- **Display**: 2.8" TFT LCD (320x240 pixels, 16-bit color)
- **Input**: 4-wire resistive touchscreen (stylus-based)
- **Storage**: SD card (supports up to 512GB, FAT32 format)
- **Power**: 3.7V LiPo battery with USB-C charging
- **Connectivity**: WiFi 802.11 b/g/n, Bluetooth Low Energy
- **Audio**: I2S DAC or built-in DAC output
- **Entropy Sources**: 3 floating analog pins for true random number generation

### What's in the Box

- remu.ii device
- Stylus
- USB-C charging cable
- Quick start guide (this manual)

---

## Getting Started

### First Boot

1. **Charge the Device**: Connect remu.ii to a USB-C power source. The power LED will illuminate while charging.
2. **Insert SD Card**: Ensure your SD card is formatted as FAT32 and insert it into the SD card slot (located under the display).
3. **Power On**: Press and hold the power button for 2 seconds. The device will boot and display the app launcher.

### Initial Setup

On first boot, you'll see:
- **Boot screen** with system initialization messages
- **App Launcher** displaying all available applications
- **System status bar** showing battery level, time, and system state

### Navigating the Interface

- **Tap**: Use the stylus to tap buttons, icons, and UI elements
- **Drag**: Touch and drag to scroll lists or move sliders
- **Long Press**: Hold the stylus on an element for 800ms to access context menus
- **Swipe**: Quickly drag across the screen to navigate (left/right/up/down)
- **Double Tap**: Tap twice quickly for special actions

### App Launcher

The launcher displays all installed applications in a grid layout:
- **Tap an app icon** to launch the application
- **System apps** are marked with a system icon
- **Memory indicator** shows available RAM before launching

---

## Interface Guide

### Status Bar

Located at the top of the screen:
- **Battery Icon**: Shows charge level and charging status
- **Time**: Current uptime or time since boot
- **WiFi/BLE Icons**: Connection status indicators
- **Memory Indicator**: Available free heap memory

### Common UI Elements

- **Buttons**: Tap to activate. Buttons have visual feedback (press state)
- **Checkboxes**: Tap to toggle on/off
- **Sliders**: Drag left/right to adjust values
- **Scroll Bars**: Drag to scroll through long lists
- **Windows**: Draggable dialog boxes with close buttons
- **Progress Bars**: Show completion status of operations

### Gestures

| Gesture | Action | Duration |
|---------|--------|----------|
| **Tap** | Select/Activate | Instant |
| **Double Tap** | Quick action | <300ms between taps |
| **Long Press** | Context menu | >800ms |
| **Drag** | Move/Scroll | 10px threshold |
| **Swipe** | Navigate | >50px, >200px/s |

---

## Applications

### Digital Pet

**Category**: Entertainment / Companion
**Purpose**: AI-driven cyberpet with mood tracking and entropy influence

#### Features

- **Psychological Mood System**: Your pet has four mood states:
  - **CALM**: Low entropy, content and peaceful
  - **RESTLESS**: Moderate entropy, seeking attention
  - **OBSESSED**: Focused on specific behavioral patterns
  - **GLITCHED**: High entropy, corrupted and unpredictable

- **Personality Traits**: Your pet can have multiple traits:
  - **LOVING**: Responds well to affection
  - **AGGRESSIVE**: Reacts negatively to rough touch
  - **NEEDY**: Requires frequent interaction
  - **PARANOID**: Suspicious of your actions

- **Archetypes**: Choose one at creation:
  - **ORACLE**: Wise, entropy-reading, offers cryptic visions
  - **PARASITE**: Clingy, battery-draining, reacts to neglect
  - **MIRROR**: Mimics your interaction patterns with unsettling behavior

#### How to Play

1. **First Time**: Select your pet's archetype (this is permanent!)
2. **Interact**: Tap the pet sprite to interact (pet, feed, play)
3. **Feed**: Keep your pet satisfied by feeding regularly
4. **Observe**: Watch as your pet's mood changes based on entropy
5. **Stats**: View detailed statistics and interaction history

#### Touch Zones

- **Pet Area** (center): Interact with your pet
- **Feed Button**: Feed your pet to keep it happy
- **Play Button**: Engage in play activities
- **Stats Button**: View detailed statistics
- **Back Button**: Return to main pet view

#### Tips

- Your pet remembers recent interactions (up to 50 events)
- High ambient entropy causes corruption and glitching
- Neglecting your pet affects its mood and behavior
- Different archetypes react differently to the same actions
- The pet's room theme changes based on how you treat it

---

### Sequencer

**Category**: Media / Music Production
**Purpose**: 8-track, 16-step beat-making engine with glitch aesthetic

#### Features

- **16-Step Grid**: Classic drum machine-style step sequencer
- **8 Tracks**: Each track can play a different sample
- **Cell States**:
  - **OFF**: Step is silent
  - **ON**: Step plays sample at normal volume
  - **ACCENT**: Louder hit
  - **GHOST**: Quieter hit

- **Audio Engine**:
  - Sample rate: 22,050 Hz
  - Built-in samples: kick, snare, hihat, bass
  - Load custom samples from SD card

#### Controls

- **Grid**: Tap cells to toggle ON/OFF, hold for accent/ghost
- **Play/Stop**: Start/stop playback
- **Record**: Live recording mode
- **BPM**: Adjust tempo (40-300 BPM)
- **Swing**: Add groove (0-100%, 50% = no swing)
- **Pattern Length**: 1-16 steps

#### Track Controls

Each track has:
- **Volume** (0-127)
- **Pitch** (-12 to +12 semitones)
- **Pan** (0-127, 64 = center)
- **Mute/Solo** buttons

#### Effects

- **Distortion**: Add grit and saturation
- **Bitcrush**: Lo-fi sample-rate reduction
- **Delay**: Configurable delay time

#### Modes

- **PATTERN**: Edit grid patterns
- **SONG**: Chain patterns for full arrangements
- **PERFORM**: Live performance tweaks
- **SAMPLE**: Manage sample library

#### File Management

- **Save Pattern**: Patterns saved to `/projects/*.json`
- **Load Sample**: Samples loaded from `/sounds/`
- **Export**: Export patterns or full songs

---

### WiFi Tools

**Category**: Hacking Tools / Network Security Research
**Purpose**: Network scanning, analysis, and security testing

⚠️ **WARNING**: This tool is for authorized security testing and educational purposes only. Unauthorized use may be illegal in your jurisdiction.

#### Features

- **WiFi Scanner**: Discover networks on all channels (1-13)
- **RSSI Measurement**: Signal strength analysis
- **Security Detection**: Identify WEP, WPA, WPA2, WPA3, Open networks
- **Client Detection**: Discover connected devices
- **Network Sorting**: Sort by signal, channel, security type

#### Modes

1. **SCANNER**: Passive network discovery
2. **DEAUTH**: De-authentication attacks (if firmware supports)
3. **BEACON_SPAM**: Generate fake SSIDs
4. **MONITOR**: Raw packet monitoring
5. **AP_CLONE**: Evil twin access point
6. **HANDSHAKE**: Capture WPA handshakes

#### Network Information

For each network:
- **SSID**: Network name (or <hidden> for stealth networks)
- **BSSID**: MAC address of access point
- **RSSI**: Signal strength in dBm
- **Channel**: WiFi channel (1-13)
- **Security**: Encryption type
- **Clients**: Number of connected devices

#### Safety Features

- **Legal Disclaimer**: Displayed on startup (must acknowledge)
- **Transmission Limits**: Conservative defaults enforced
- **Activity Logging**: All actions logged to SD card for audit trail

#### Legal Notice

The use of WiFi deauthentication, beacon flooding, or any attack mode may be **illegal** without proper authorization. remu.ii is designed for:
- Authorized penetration testing engagements
- Security research in controlled environments
- Educational purposes in isolated networks
- CTF (Capture The Flag) competitions

**Always obtain written permission before testing networks you do not own.**

---

### BLE Scanner

**Category**: Security Tools / Device Discovery
**Purpose**: Bluetooth Low Energy monitoring and anomaly detection

#### Features

- **Device Scanning**: Discover BLE devices in range
- **RSSI Tracking**: Monitor signal strength over time (20-sample history)
- **Anomaly Detection**: Identify suspicious behavior:
  - Sudden RSSI changes
  - RSSI outliers
  - MAC address randomization
  - Timing irregularities
  - Signal spoofing attempts
  - Rapid appearance patterns

- **Device Labeling**: Assign custom names to known devices
- **Statistics**: Comprehensive scan statistics and trends

#### Views

1. **DEVICE_LIST**: All discovered devices (scrollable)
2. **DEVICE_DETAILS**: Full information for selected device
3. **ANOMALY_ALERTS**: Detected anomalies with descriptions
4. **STATISTICS**: Summary stats (total devices, anomalies, etc.)
5. **LABELING**: Device naming interface
6. **LOGS**: Event logging view

#### Device Information

For each device:
- **MAC Address**: Unique identifier
- **Device Name**: Advertised name (if available)
- **RSSI**: Current signal strength
- **RSSI History**: Statistical analysis (mean, variance, std dev)
- **First Seen / Last Seen**: Timestamps
- **Scan Count**: Number of times detected
- **Anomaly Status**: Normal, suspicious, or anomaly type

#### Configuration

- **Scan Duration**: How long to scan (seconds)
- **Scan Interval**: Time between scans
- **RSSI Threshold**: Minimum signal strength to log
- **Anomaly Sensitivity**: 0.0-1.0 (higher = more sensitive)
- **Auto-Label**: Automatically label known device types
- **SD Logging**: Enable/disable file logging
- **Device Timeout**: Remove devices not seen for N minutes

#### Files

- **Device Labels**: `/data/blescanner/labels.json`
- **Scan Log**: `/logs/ble_scan.log`
- **Anomaly Log**: `/logs/ble_anomalies.log`
- **Config**: `/settings/blescanner.cfg`

---

### Car Cloner (RF Tools)

**Category**: RF Security Research
**Purpose**: RF signal capture, analysis, and replay

⚠️ **WARNING**: This tool is for authorized security research and educational purposes only. Transmitting on certain frequencies may be illegal.

#### Features

- **RF Signal Capture**: Record signals from key fobs, remotes, etc.
- **Frequency Support**: 315MHz, 433.92MHz, 868MHz, 915MHz (+ custom)
- **Signal Analysis**:
  - Modulation detection (ASK, FSK, PSK, OOK, PWM, Manchester)
  - Pulse width analysis
  - Protocol detection (fixed code, rolling code)
  - Entropy scoring
  - Confidence rating

- **Signal Replay**: Transmit captured signals
- **Storage**: Save up to 32 signals

#### Capture Modes

- **Single**: Capture one signal
- **Continuous**: Capture multiple signals
- **Triggered**: Wait for signal above threshold
- **Timed**: Capture for specified duration

#### Views

1. **Main Menu**: Start capture or replay
2. **Capture Interface**: Frequency/power controls
3. **Signal Library**: Browse saved signals
4. **Replay Interface**: Select and transmit
5. **Analysis View**: Show modulation, timing, protocol
6. **Settings**: Configuration panel

#### Signal Information

For each captured signal:
- **Frequency**: Center frequency in MHz
- **Modulation**: Detected modulation type
- **Protocol**: Fixed code, rolling code, or PWM
- **Pulse Count**: Number of pulses detected
- **Signal Strength**: RSSI during capture
- **Timestamp**: When captured
- **File Path**: Where saved on SD card

#### Safety Features

- **Legal Warning**: Displayed for 10 seconds minimum on startup
- **Transmission Limits**: Max 30 seconds continuous transmission
- **Conservative Defaults**: Power set to 50/255
- **Activity Logging**: All transmissions logged
- **Legality Checks**: Warns before transmission

#### Legal Notice

Transmitting radio signals may require licensing or may be prohibited:
- **Always check local regulations** before transmitting
- **Only test on devices you own** or have authorization to test
- **Use in isolated/shielded environments** when possible
- **Understand the legal implications** of RF transmission

---

### Frequency Scanner

**Category**: Signal Analysis / DSP
**Purpose**: Real-time FFT spectrum analyzer

#### Features

- **FFT Processing**: 128, 256, 512, or 1024 sample sizes
- **Window Functions**: Rectangular, Hamming, Blackman, Hanning, Kaiser
- **Sample Rates**: 8kHz, 16kHz, 22.05kHz, 44.1kHz
- **Peak Detection**: Automatic spectral peak identification (up to 10 peaks)
- **Spectrum Smoothing**: Adjustable smoothing factor (0.0-1.0)

#### Display Modes

1. **SPECTRUM**: Real-time frequency domain display
2. **WATERFALL**: Time-frequency waterfall plot
3. **DUAL**: Split view (spectrum + waterfall)
4. **RECORDING**: Data capture interface
5. **GENERATOR**: Signal generation interface
6. **SETTINGS**: Configuration panel

#### Measurement Tools

- **Frequency Markers**: 2 independent movable markers
- **Cursor**: Dual-axis measurement cursor
- **Readouts**: Frequency and amplitude at markers
- **Delta**: Difference between marker positions

#### Signal Generator

Generate test signals:
- **Waveforms**: Sine, square, triangle, sawtooth, noise, sweep, custom
- **Modulation**: AM (Amplitude), FM (Frequency), PWM (Pulse Width)
- **Frequency**: Adjustable in Hz
- **Amplitude**: 0.0-1.0
- **Sweep**: Configurable start/end/duration

#### Recording

- **Format**: CSV or binary
- **Duration**: Up to 60 seconds (configurable)
- **Metadata**: Saved in JSON format
- **Storage**: `/data/freqscanner/recordings/`

#### Frequency Ranges

- **AUDIO_LOW**: 20Hz - 2kHz
- **AUDIO_MID**: 200Hz - 8kHz
- **AUDIO_FULL**: 20Hz - 20kHz
- **RF_LOW**: 1MHz - 30MHz
- **RF_HIGH**: 30MHz - 300MHz
- **CUSTOM**: User-defined range

---

### Entropy Beacon

**Category**: Metaphysical / Anomaly Detection
**Purpose**: Real-time entropy visualization and anomaly detection

#### Features

- **Entropy Sampling**: Read from 3 floating analog pins
- **Sample Rates**: 100Hz, 500Hz, 1kHz, 2kHz, 5kHz, 8kHz
- **Anomaly Detection**: Threshold-based outlier identification
- **Visualization**: Oscilloscope and spectrum modes
- **DAC Output**: Generate entropy-driven waveforms

#### Visualization Modes

1. **OSCILLOSCOPE**: Time-domain waveform trace
2. **SPECTRUM**: 32-bin FFT visualization

#### Configuration

- **Sample Rate**: Adjust sampling frequency
- **Amplitude Scaling**: Zoom in/out on signal
- **DAC Enable/Disable**: Toggle audio output
- **Recording**: Enable/disable SD card logging
- **Anomaly Threshold**: Sensitivity adjustment

#### Interpretation

- **Low Entropy**: Stable, predictable patterns
- **High Entropy**: Chaotic, random patterns
- **Anomalies**: Sudden spikes or unusual patterns
- **Correlation**: Look for patterns correlating with events

---

## System Settings

### Accessing Settings

From any app, return to the launcher and look for the **Settings** app (gear icon).

### Available Settings

- **Brightness**: Adjust display brightness (0-255)
- **Volume**: Set audio output volume (0-100)
- **Auto-Sleep**: Timeout before entering low-power mode (minutes)
- **Language**: Interface language (currently English only)
- **Touch Calibration**: Recalibrate the touchscreen
- **SD Card**: Format, eject, or check SD card health
- **WiFi**: Configure WiFi settings
- **System Info**: View system statistics and uptime

### Touch Calibration

If touch input becomes inaccurate:
1. Go to **Settings** → **Touch Calibration**
2. Tap the calibration targets precisely with the stylus
3. Calibration data is saved to EEPROM
4. Restart the device to apply changes

---

## File Management

### Directory Structure

remu.ii automatically creates this structure on your SD card:

```
/
├── apps/           - Application data
├── data/           - User data
│   ├── blescanner/ - BLE scan results
│   ├── carcloner/  - RF signal captures
│   └── freqscanner/- FFT recordings
├── samples/        - Audio samples for Sequencer
├── settings/       - Configuration files
│   └── config.json - System settings
├── temp/           - Temporary files
└── logs/           - Event logs
    ├── ble_scan.log
    ├── ble_anomalies.log
    └── carcloner.log
```

### File Formats

- **JSON**: Configuration and save files (human-readable)
- **CSV**: Data exports and recordings
- **Binary**: Raw signal data
- **WAV**: Audio samples (mono, 16-bit, 22050Hz recommended)

### Best Practices

- Use a high-quality SD card (Class 10 or UHS-I)
- Format as FAT32
- Keep at least 100MB free for system operations
- Regularly back up important data
- Eject SD card properly through Settings before removing

---

## Battery & Power Management

### Battery Monitoring

The status bar shows:
- **Battery Percentage**: Estimated charge level (0-100%)
- **Charging Icon**: Appears when plugged in
- **Power State**:
  - FULL: >75%
  - GOOD: 25-75%
  - LOW: 10-25%
  - CRITICAL: <10%

### Power States

- **FULL POWER**: All features enabled, 240MHz CPU
- **LOW POWER**: Reduced brightness, 80MHz CPU
- **CRITICAL**: Emergency mode, return to launcher, save all data

### Battery Tips

- Fully charge before first use
- Avoid deep discharge (<5%)
- WiFi/BLE scanning drains battery faster
- Lower brightness to extend runtime
- RF transmission uses significant power

### Charging

- Use the included USB-C cable
- Charging time: ~2-3 hours from empty
- Can use device while charging
- Power LED indicates charging status

---

## Troubleshooting

### Device Won't Turn On

- Check battery charge
- Try connecting to USB-C power
- Hold power button for 10 seconds to force restart

### Touchscreen Not Responding

- Recalibrate touch in Settings
- Ensure you're using a stylus (not finger)
- Clean the screen with a soft cloth
- Check for cracks or damage

### SD Card Not Detected

- Ensure card is formatted as FAT32
- Try reinserting the card
- Check card capacity (max 512GB)
- Test card in another device

### Apps Crashing or Freezing

- Check free memory in status bar
- Return to launcher and relaunch app
- Restart device if problem persists
- Update firmware if available

### WiFi/BLE Not Working

- Check WiFi is enabled in Settings
- Ensure you're within range of target devices
- Some features require specific ESP32 firmware
- Restart device to reset radio

### Audio Not Working

- Check volume setting
- Verify audio output connection
- Ensure samples are in correct format (WAV, mono, 16-bit)
- Check I2S DAC wiring if using external DAC

### Screen Glitches or Artifacts

- This may be intentional (corruption effects in Digital Pet)
- If persistent, check display connections
- Restart device
- Reduce brightness if overheating

---

## Safety & Legal Information

### General Safety

- Do not expose to water or extreme temperatures
- Do not disassemble device (no user-serviceable parts)
- Use only recommended batteries and chargers
- Keep away from children under 13
- Stylus is a choking hazard

### Electrical Safety

- Do not use if device is damaged
- Disconnect power if device overheats
- Do not short-circuit battery terminals
- Dispose of batteries according to local regulations

### Legal Compliance

remu.ii includes tools for security research and testing. **You are responsible for legal compliance in your jurisdiction.**

#### WiFi Tools

- **Deauthentication attacks** may be illegal without authorization
- **Beacon flooding** may violate local regulations
- **Packet injection** requires proper licensing in some areas
- **Only test networks you own or have written permission to test**

#### RF Tools (Car Cloner)

- **Transmitting on certain frequencies requires licensing**
- **Jamming or interfering with communications is illegal**
- **Only test devices you own**
- **Check local regulations before transmitting**

#### BLE Scanner

- **Passive scanning is generally legal**
- **MAC address collection may have privacy implications**
- **Be aware of local privacy laws**

### Intended Use

remu.ii is designed for:
- **Authorized security testing and penetration testing**
- **Educational purposes in controlled environments**
- **Personal creative projects (music, art, data visualization)**
- **CTF competitions and hacking challenges**
- **Security research with proper authorization**

### Not Intended For

- **Unauthorized access to systems or networks**
- **Malicious hacking or illegal activities**
- **Privacy violations or surveillance**
- **Jamming or disrupting critical communications**

### Disclaimer

The creators of remu.ii are not responsible for misuse of this device or illegal activities conducted with it. Users assume all legal risk and responsibility for their actions.

---

## Support & Community

### Getting Help

- **Documentation**: Check `/Documentation/` on SD card
- **Source Code**: https://github.com/numbpill3d/remu.ii
- **Issues**: Report bugs on GitHub
- **Firmware Updates**: Check releases page

### Contributing

remu.ii is open-source! Contributions welcome:
- Submit pull requests
- Report bugs
- Suggest new features
- Share your projects

### Credits

- **Hardware**: ESP32 WROOM-32, Adafruit ILI9341 display
- **Libraries**: Adafruit GFX, ArduinoJson, ESP32 Arduino Core
- **Firmware**: Custom remu.ii OS
- **Author**: numbpill3d

---

**Thank you for using remu.ii!**

*Stay curious. Stay legal. Stay weird.*

---

**Version**: 1.0
**Last Updated**: 2025-11-12
**License**: Open Source (see LICENSE file)
