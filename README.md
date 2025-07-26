**remu.ii – Handheld OS Feature Specification Document**

*Codename: remu.ii*
*A stylus-based ESP32 handheld system for autonomous hacking, anomaly interaction, and cyberpet companionship.*

---

## 🧠 Core System Overview

* **Device Name:** remu.ii
* **Platform:** ESP32 (WROOM preferred)
* **Screen:** Adafruit ILI9341 with resistive touch
* **Storage:** SD card (up to 512GB FAT32) via TFT slot
* **Input:** Stylus (resistive touch only, no joystick or encoder)
* **Power:** FuelRod lithium battery (USB-C), physical on/off switch
* **Boot:** From SD card, dynamic app loader with launcher
* **UI Theme:** Glitched, terminal-esque, stylus-driven OS with a touch menu to access modular apps

---

## 🗂️ Modular App Architecture

Each app is stored in its own folder on the SD card under `/apps/`. All apps are loaded dynamically to conserve memory. File structure includes a manifest, assets, and core code.

```
remu.ii/
├── remu.ii.ino
├── core/
│   ├── SystemCore/
│   │   ├── SystemCore.cpp
│   │   └── SystemCore.h
│   ├── DisplayManager/
│   │   ├── DisplayManager.cpp
│   │   └── DisplayManager.h
│   ├── TouchInterface/
│   │   ├── TouchInterface.cpp
│   │   └── TouchInterface.h
│   ├── Config/
│   │   └── Config.h
│   ├── Settings/
│   │   ├── Settings.cpp
│   │   └── Settings.h
│   ├── AppManager/
│   │   ├── AppManager.cpp
│   │   └── AppManager.h
├── apps/
│   ├── DigitalPet/
│   │   ├── DigitalPet.cpp
│   │   └── DigitalPet.h
│   ├── Sequencer/
│   │   ├── Sequencer.cpp
│   │   └── Sequencer.h
│   ├── WiFiTools/
│   │   ├── WiFiTools.cpp
│   │   └── WiFiTools.h
│   ├── BLEScanner/
│   │   ├── BLEScanner.cpp
│   │   └── BLEScanner.h
│   ├── CarCloner/
│   │   ├── CarCloner.cpp
│   │   └── CarCloner.h
│   ├── FreqScanner/
│   │   ├── FreqScanner.cpp
│   │   └── FreqScanner.h
│   ├── EntropyBeacon/
│   │   ├── EntropyBeacon.cpp
│   │   └── EntropyBeacon.h

```

---

## 🐾 Digital Pet (Entity Engine)

An AI-driven companion that lives in its own app container. Stylus-only interaction.

### Features:

* **Stats:** Mood, Hunger, Loneliness, Entropy, Sleep, Stability
* **Interaction:** Stylus taps to pet, feed, show
* **Entropy Influence:** Mood/appearance and UI fluctuates with ambient entropy
* **Learning AI:** Simple memory and behavior modeling based on user input history
* **Customization:**

  * Dress-up features (hats, accessories, skins, eyes, antennae)
  * Room decoration and furniture from unlockables
* **Persistence:** JSON state saving to `/apps/digital_pet/pet_data.json`

---

## 🎛️ Model\:Samples-Inspired Music Sequencer

A beat-making engine with glitch and ambient bias.

### Features:

* **Interface:** 6-track x 16-step grid (touch-select) effect knobs/toggles (popup or input based)
* **Sample System:** Load `.wav` files from `/sounds/`
* **Playback Engine:** Real-time, stylus-modulated tempo control
* **Preset Save/Load:** `/projects/*.json`
* **Visuals:** Pixel-glitch animations and BPM pulsing

---

## 🛠️ Hacking Utilities

### 1. **WiFi Tools**

* SSID scanner with RSSI, channel, and MAC
* SSID spammer (fake APs from list)
* Deauthentication (if supported by firmware)

### 2. **BLE Scanner**

* Nearby device discovery
* Displays name, MAC, and signal strength
* Logging to `/logs/ble/`

### 3. **Car Cloner / Signal Replayer**

* Passive RF signal sniffer (if external hardware connected)
* Record + replay function
* Logs saved to `/captures/car_keys/`

### 4. **Frequency Scanner**

* Signal waterfall or bar graph visualizer
* Logs anomalous spikes with timestamp

---

## 🧪 Entropy Beacon / NHI Anomaly Tool

Stylus-driven metaphysical contact UI for speculative signal interaction.

### Features:

* Reads entropy from floating analog pins
* Logs spikes and patterns
* DAC output pulses based on entropy curves
* Graphical representation of detected anomalies
* Stylus taps can modulate waveform (as user signal output)

---

## 🔧 System Utilities

### 1. **File Browser**

* Navigate SD file structure
* View text logs, browse folders
* Delete and copy functions

### 2. **Settings Manager**

* Adjust screen brightness, auto-sleep, volume
* Load/save settings from `/sys/config.json`

### 3. **Device Resuscitator**

* Uses USBtinyISP
* Flash Pro Micro, Trinkey, etc.
* Includes diagrams + known bootloader images

---

## 📁 File Structure Overview

```
remu.ii/
├── remu.ii.ino
├── core/
│   ├── SystemCore/
│   │   ├── SystemCore.cpp
│   │   └── SystemCore.h
│   ├── DisplayManager/
│   │   ├── DisplayManager.cpp
│   │   └── DisplayManager.h
│   ├── TouchInterface/
│   │   ├── TouchInterface.cpp
│   │   └── TouchInterface.h
│   ├── Config/
│   │   └── Config.h
│   ├── Settings/
│   │   ├── Settings.cpp
│   │   └── Settings.h
│   ├── AppManager/
│   │   ├── AppManager.cpp
│   │   └── AppManager.h
├── apps/
│   ├── DigitalPet/
│   │   ├── DigitalPet.cpp
│   │   └── DigitalPet.h
│   ├── Sequencer/
│   │   ├── Sequencer.cpp
│   │   └── Sequencer.h
│   ├── WiFiTools/
│   │   ├── WiFiTools.cpp
│   │   └── WiFiTools.h
│   ├── BLEScanner/
│   │   ├── BLEScanner.cpp
│   │   └── BLEScanner.h
│   ├── CarCloner/
│   │   ├── CarCloner.cpp
│   │   └── CarCloner.h
│   ├── FreqScanner/
│   │   ├── FreqScanner.cpp
│   │   └── FreqScanner.h
│   ├── EntropyBeacon/
│   │   ├── EntropyBeacon.cpp
│   │   └── EntropyBeacon.h

```
