# remu.ii

**A handheld multifaceted modular device for hacking, music production, and metaphysical exploration.**

[![License](https://img.shields.io/badge/license-Open%20Source-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Status](https://img.shields.io/badge/status-Active-success.svg)]()

---

## 📖 What is remu.ii?

**remu.ii** is a stylus-based handheld device powered by an ESP32 microcontroller that combines:

- 🐾 **AI Cyberpet Companion** with mood tracking and entropy influence
- 🎵 **8-Track Music Sequencer** for beat-making and glitch music
- 📡 **WiFi Security Tools** for network scanning and security research
- 🔵 **BLE Scanner** with anomaly detection
- 📻 **RF Tools** for signal capture and analysis
- 📊 **FFT Spectrum Analyzer** for signal processing
- 🌀 **Entropy Visualizer** for metaphysical anomaly detection

All housed in a portable device with a 2.8" touchscreen, SD card storage, and retro "terminal glitch" aesthetic.

---

## ✨ Features

### Hardware

- **ESP32 WROOM-32** dual-core processor (240MHz)
- **2.8" ILI9341 TFT Display** (320x240 pixels, 16-bit color)
- **4-wire resistive touchscreen** (stylus-based input)
- **SD card support** (up to 512GB, FAT32)
- **LiPo battery** with USB-C charging
- **WiFi + Bluetooth LE** connectivity
- **I2S audio output**
- **True entropy sources** from floating analog pins

### Software

- **Modular architecture** - Apps load dynamically
- **Memory-optimized** - Runs on 320KB RAM
- **Persistent storage** - Save state to SD card
- **Touch gestures** - Tap, drag, swipe, long-press
- **Watchdog protection** - Auto-recovery from crashes
- **Battery monitoring** - Real-time power management
- **Retro UI** - Darknet terminal aesthetic

---

## 🚀 Quick Start

### For Users

1. **Get the hardware** - See [ASSEMBLY_GUIDE.md](ASSEMBLY_GUIDE.md) for build instructions
2. **Install firmware** - Follow [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md) for Arduino IDE setup
3. **Learn to use** - Read [USER_MANUAL.md](USER_MANUAL.md) for detailed usage instructions

### For Developers

1. **Clone the repository**:
   ```bash
   git clone https://github.com/numbpill3d/remu.ii.git
   cd remu.ii
   ```

2. **Install Arduino IDE** and ESP32 board support

3. **Install required libraries**:
   - Adafruit GFX Library (≥1.11.0)
   - Adafruit ILI9341 (≥1.6.0)
   - ArduinoJson (v6.x)

4. **Open `remu_ii.ino`** in Arduino IDE

5. **Select board**: ESP32 Dev Module

6. **Configure settings**:
   - Upload Speed: 921600
   - CPU Frequency: 240MHz
   - Flash Size: 4MB

7. **Upload firmware** to your ESP32

See [API_REFERENCE.md](API_REFERENCE.md) for development documentation.

---

## 📚 Documentation

- **[USER_MANUAL.md](USER_MANUAL.md)** - Complete user guide with app instructions
- **[INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md)** - Firmware installation and setup
- **[ASSEMBLY_GUIDE.md](ASSEMBLY_GUIDE.md)** - Hardware assembly instructions with wiring diagrams
- **[API_REFERENCE.md](API_REFERENCE.md)** - Developer API documentation
- **[HARDWARE_WIRING.md](HARDWARE_WIRING.md)** - Detailed pin connections
- **[COMPILATION_STATUS.md](COMPILATION_STATUS.md)** - Build configuration and status

---

## 🎮 Applications

### Digital Pet

AI-driven cyberpet companion with psychological mood system, personality traits, and archetype-based behavior. Your pet's mood is influenced by ambient entropy and your interactions.

**Features**: CALM/RESTLESS/OBSESSED/GLITCHED moods, LOVING/AGGRESSIVE/NEEDY/PARANOID traits, ORACLE/PARASITE/MIRROR archetypes

### Sequencer

8-track, 16-step drum machine and beat sequencer with glitch aesthetic.

**Features**: 16-step grid, 8 tracks, BPM control, swing adjustment, pitch shift, distortion/bitcrush/delay effects, pattern save/load

### WiFi Tools

Network security testing and analysis platform.

**Features**: WiFi scanner, SSID discovery, security type detection, client enumeration, deauth attacks, beacon spam, evil twin AP

⚠️ **For authorized security testing only**

### BLE Scanner

Bluetooth Low Energy device discovery with advanced anomaly detection.

**Features**: Device scanning, RSSI tracking, anomaly detection (MAC randomization, signal spoofing, timing irregularities), device labeling, statistics

### Car Cloner (RF Tools)

RF signal capture, analysis, and replay for security research.

**Features**: Multi-frequency support (315MHz, 433.92MHz, 868MHz, 915MHz), modulation detection (ASK/FSK/PSK/OOK), protocol analysis, signal library

⚠️ **For authorized security research only**

### Frequency Scanner

Real-time FFT spectrum analyzer with waterfall display.

**Features**: FFT processing (128-1024 samples), window functions, peak detection, spectrum smoothing, signal recording, signal generator

### Entropy Beacon

Real-time entropy visualization and anomaly detection from floating analog pins.

**Features**: Oscilloscope/spectrum visualization, multiple sample rates, anomaly detection, DAC output, entropy-driven waveforms

---

## 🛠️ Hardware Requirements

### Main Components

- ESP32 WROOM-32 development board
- 2.8" ILI9341 TFT display with SPI interface
- 4-wire resistive touchscreen
- Micro SD card slot (usually built into display)
- LiPo battery (3.7V, 1000-2000mAh)
- TP4056 USB-C charging module
- Stylus for resistive touch

### Optional Components

- I2S DAC or amplifier for audio
- Speaker or headphone jack
- RF modules for extended functionality
- Custom 3D-printed enclosure

See [ASSEMBLY_GUIDE.md](ASSEMBLY_GUIDE.md) for complete parts list and wiring.

---

## 🔧 Technical Specifications

| Aspect | Details |
|--------|---------|
| **Processor** | ESP32 WROOM-32 (dual-core Xtensa, 240MHz) |
| **RAM** | 320KB SRAM |
| **Flash** | 4MB |
| **Display** | ILI9341 2.8" TFT (320x240, 65K colors) |
| **Touch** | 4-wire resistive (XPT2046 compatible) |
| **Storage** | SD card (4GB-512GB, FAT32) |
| **Connectivity** | WiFi 802.11 b/g/n, Bluetooth LE |
| **Audio** | I2S DAC / Built-in DAC (22.05kHz) |
| **Power** | 3.7V LiPo, USB-C charging |
| **Frame Rate** | 20 FPS (memory-optimized) |
| **OS** | Custom firmware (Arduino framework) |

---

## 📂 Project Structure

```
remu.ii/
├── remu_ii.ino                 # Main entry point
├── core/                       # Core system modules
│   ├── SystemCore/             # Power, entropy, watchdog
│   ├── DisplayManager/         # ILI9341 control, UI rendering
│   ├── TouchInterface/         # 4-wire touch + gestures
│   ├── AppManager/             # Dynamic app loading
│   ├── Settings/               # JSON configuration
│   ├── FileSystem/             # SD card abstraction
│   └── Config/                 # Hardware pin definitions
├── apps/                       # Application modules
│   ├── DigitalPet/             # AI cyberpet companion
│   ├── Sequencer/              # Music production
│   ├── WifiTools/              # Network security
│   ├── BLEScanner/             # Bluetooth analysis
│   ├── CarCloner/              # RF tools
│   ├── FreqScanner/            # FFT spectrum analyzer
│   └── EntropyBeacon/          # Entropy visualization
├── Documentation/              # Additional docs
│   ├── USER_MANUAL.md
│   ├── INSTALLATION_GUIDE.md
│   ├── ASSEMBLY_GUIDE.md
│   ├── API_REFERENCE.md
│   └── HARDWARE_WIRING.md
└── README.md                   # This file
```

---

## 🔐 Security & Legal

remu.ii includes tools for **authorized security testing** and **educational purposes only**.

### Legal Notice

- **WiFi deauth, beacon spam**: May be illegal without authorization
- **RF transmission**: Requires proper licensing in many jurisdictions
- **Network testing**: Only test networks you own or have written permission
- **Responsible use**: Always comply with local laws and regulations

### Intended Use

- Authorized penetration testing
- Security research in controlled environments
- Educational purposes
- CTF competitions
- Personal creative projects

**Users assume all legal responsibility for their actions.**

---

## 🤝 Contributing

Contributions are welcome! Here's how:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Development Guidelines

- Follow existing code style
- Document public APIs
- Test on real hardware when possible
- Update documentation for user-facing changes
- Write clear commit messages

---

## 🐛 Bug Reports & Feature Requests

- **Bug reports**: Open an issue with detailed description and steps to reproduce
- **Feature requests**: Describe the feature and its use case
- **Questions**: Check documentation first, then open a discussion

---

## 📜 License

This project is open source. See [LICENSE](LICENSE) file for details.

---

## 🙏 Credits

### Hardware

- **ESP32** by Espressif Systems
- **ILI9341 Display** compatible with Adafruit modules
- **Arduino Framework** for ESP32

### Libraries

- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit ILI9341](https://github.com/adafruit/Adafruit_ILI9341)
- [ArduinoJson](https://arduinojson.org/)
- ESP32 Arduino Core

### Inspiration

- Retro handheld consoles
- Cyberdeck culture
- Security research tools
- Glitch aesthetics

---

## 🌟 Show Your Support

If you find remu.ii useful:
- ⭐ Star this repository
- 🐛 Report bugs and suggest features
- 🔧 Contribute code or documentation
- 🎨 Share your projects and mods
- 💬 Join the community discussions

---

## 📞 Contact

- **GitHub**: [@numbpill3d](https://github.com/numbpill3d)
- **Project**: [github.com/numbpill3d/remu.ii](https://github.com/numbpill3d/remu.ii)
- **Issues**: [github.com/numbpill3d/remu.ii/issues](https://github.com/numbpill3d/remu.ii/issues)

---

## 🗺️ Roadmap

### Completed

- ✅ Core system architecture
- ✅ Display and touch management
- ✅ App launcher and lifecycle
- ✅ Digital Pet with mood system
- ✅ WiFi/BLE security tools
- ✅ Sequencer framework
- ✅ Entropy system
- ✅ Comprehensive documentation

### In Progress

- 🚧 Complete sequencer audio implementation
- 🚧 Expand RF tools capabilities
- 🚧 Additional Digital Pet features

### Planned

- 📋 OTA firmware updates
- 📋 Web-based configuration interface
- 📋 Additional applications (IRC client, file manager, etc.)
- 📋 Custom PCB design
- 📋 Injection-molded enclosure
- 📋 Community app marketplace

---

## ❓ FAQ

### Q: Can I use this for actual security testing?
**A:** Yes, but only on networks/devices you own or have written authorization to test. Unauthorized testing is illegal.

### Q: What battery life can I expect?
**A:** Depends on usage. WiFi scanning drains faster (~2-4 hours). Normal use with BLE/display (~4-6 hours). Low-power mode extends runtime.

### Q: Can I add my own applications?
**A:** Absolutely! See [API_REFERENCE.md](API_REFERENCE.md) for developer documentation on creating custom apps.

### Q: Does it support capacitive touch?
**A:** No, it's designed for resistive touch (stylus-based). Capacitive screens require different hardware and drivers.

### Q: Can I use a different display?
**A:** Potentially, but you'll need to modify the display driver code. The ILI9341 is recommended for compatibility.

### Q: Is there a pre-built version I can buy?
**A:** Not currently. This is a DIY project. See [ASSEMBLY_GUIDE.md](ASSEMBLY_GUIDE.md) for build instructions.

---

**Built with curiosity, code, and a little chaos.**

*Stay weird. Hack responsibly.*

---

**Last Updated**: 2025-11-12
**Version**: 1.0
**Status**: Feature Complete, Documentation Complete
