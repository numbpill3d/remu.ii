# remu.ii Web Mode Setup Guide

**Run remu.ii in your web browser - no display, touch screen, or SD card needed!**

Web Mode allows you to run remu.ii on just an ESP32 connected to your computer, with the interface displayed in a web browser. Perfect for development, testing, and running remu.ii without building the full hardware.

---

## 🌐 What is Web Mode?

Web Mode turns your ESP32 into a WiFi access point that serves a virtual remu.ii display to your web browser. The ESP32 runs all the firmware code, and your browser displays a Tamagotchi-style retro interface in white and red.

### Features

- ✅ **No hardware assembly required** - Just an ESP32 and USB cable
- ✅ **Browser-based display** - 320x240 virtual screen in retro style
- ✅ **Mouse/touch input** - Click on the virtual screen to interact
- ✅ **Real-time updates** - WebSocket-based communication
- ✅ **Same firmware** - Runs actual remu.ii applications
- ✅ **Portable** - Works on any computer with a browser

---

## 🛠️ Hardware Requirements

### Minimum (For Web Mode)

- **ESP32 board** (any variant: WROOM-32, DevKit V1, etc.)
- **USB cable** (data-capable, not charge-only)
- **Computer** with USB port and web browser

### That's it! No display, touchscreen, SD card, or battery needed.

---

## 📥 Installation

### Step 1: Install Arduino IDE and Libraries

Follow the standard installation from [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md):

1. **Install Arduino IDE** (1.8.19 or newer)
2. **Add ESP32 board support**
3. **Install required libraries**:
   - ArduinoJson (v6.x)
   - WebSockets by Markus Sattler
   - ESPAsyncWebServer (optional, but recommended)

#### Installing WebSockets Library

1. Open Arduino IDE
2. Go to **Tools → Manage Libraries**
3. Search for **"WebSockets by Markus Sattler"**
4. Click **Install**

---

### Step 2: Choose Your Build Method

You have two options:

#### **Option A: Simple Example (Recommended for Testing)**

Use the pre-made web mode example:

1. Navigate to `examples/web_mode/`
2. Open `remu_ii_web.ino` in Arduino IDE
3. This includes a simple demo app to test web mode

#### **Option B: Full remu.ii with Web Mode**

Enable web mode in the main firmware:

1. Open `/core/Config.h`
2. Find the "BUILD MODE CONFIGURATION" section (around line 13)
3. **Comment out** `#define HARDWARE_MODE`
4. **Uncomment** `#define WEB_MODE`

```cpp
// Build Mode Selection
// Uncomment ONE of these modes:
#define WEB_MODE          // Run in web browser (no hardware needed)
//#define HARDWARE_MODE   // Run on physical hardware (TFT, touch, SD card)
```

---

### Step 3: Configure Board Settings

1. **Connect ESP32** to computer via USB
2. **Select board**: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
3. **Select port**: **Tools → Port → (your ESP32 port)**
4. **Configure settings**:

| Setting | Value |
|---------|-------|
| **Upload Speed** | 921600 |
| **CPU Frequency** | 240MHz (WiFi/BT) |
| **Flash Frequency** | 80MHz |
| **Flash Mode** | DIO |
| **Flash Size** | 4MB (32Mb) |
| **Partition Scheme** | Default 4MB with spiffs |

---

### Step 4: Compile and Upload

1. Click **Verify** (✓) to compile
2. Wait for compilation (may take 1-2 minutes)
3. Click **Upload** (→) to flash firmware
4. Wait for upload to complete

**Troubleshooting**: If upload fails, press and hold **BOOT** button on ESP32, then press **EN** button briefly, then release BOOT.

---

## 🚀 Running Web Mode

### Step 1: Open Serial Monitor

1. Click **Tools → Serial Monitor**
2. Set baud rate to **115200**
3. You should see:

```
========================================
  remu.ii - Web Mode
========================================

[Setup] Initializing system core...
[Setup] Initializing web display...
[WebDisplay] AP IP address: 192.168.4.1
[WebDisplay] Connect to WiFi: remu.ii
[WebDisplay] Password: remuiiweb
[WebDisplay] HTTP server started on port 80
[WebDisplay] WebSocket server started on port 81

========================================
  SETUP COMPLETE!
========================================

INSTRUCTIONS:
1. Connect to WiFi network: remu.ii
2. Password: remuiiweb
3. Open browser to: http://192.168.4.1

The virtual remu.ii display will appear in your browser!
========================================
```

### Step 2: Connect to WiFi

On your computer:

1. **Open WiFi settings**
2. **Look for network**: `remu.ii`
3. **Connect** to it
4. **Enter password**: `remuiiweb`

### Step 3: Open Web Interface

1. **Open web browser** (Chrome, Firefox, Safari, Edge)
2. **Navigate to**: `http://192.168.4.1`
3. **You should see**: The virtual remu.ii device in retro Tamagotchi style!

---

## 🎮 Using the Web Interface

### The Virtual Display

The web interface shows:

```
┌─────────────────────────────────────┐
│           remu.ii                   │  ← Device branding
│        Virtual Display              │
│                                     │
│  ┌───────────────────────────────┐ │
│  │                               │ │
│  │    320x240 Canvas Display     │ │  ← Interactive screen
│  │     (Click to interact)       │ │
│  │                               │ │
│  └───────────────────────────────┘ │
│                                     │
│  Connection: ● Connected            │  ← Status indicators
│  Touch: (160, 120)                  │
│  FPS: 20                            │
└─────────────────────────────────────┘
```

### Interacting

- **Click** on the virtual screen to simulate stylus touch
- **Drag** to simulate dragging
- **Release** to end touch
- Touch coordinates are displayed in real-time

### Visual Style

The interface uses a **retro Tamagotchi aesthetic**:
- **White and red** color scheme
- **Pixelated display** with scanline effect
- **3D device bezel** with rounded corners
- **Status indicators** with LED-style dots
- **Monospace font** for authentic retro feel

---

## 🔧 Configuration

### Changing WiFi Credentials

Edit the configuration in your sketch:

**For simple example** (`remu_ii_web.ino`):
```cpp
#define WEB_MODE_SSID "remu.ii"
#define WEB_MODE_PASSWORD "remuiiweb"
```

**For full firmware** (`Config.h`):
```cpp
#ifdef WEB_MODE
#define WEB_MODE_SSID "remu.ii"
#define WEB_MODE_PASSWORD "remuiiweb"
#define WEB_SERVER_PORT 80
#define WEBSOCKET_PORT 81
#endif
```

### Connecting to Existing WiFi

To connect to your existing WiFi instead of creating an access point:

Modify `WebDisplay::initialize()` in `WebDisplay.cpp`:

```cpp
// Change from:
WiFi.mode(WIFI_AP);
WiFi.softAP(ssid, password);

// To:
WiFi.mode(WIFI_STA);
WiFi.begin("YourWiFiSSID", "YourWiFiPassword");
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
Serial.println("\nConnected!");
Serial.print("IP: ");
Serial.println(WiFi.localIP());
```

Then connect to the displayed IP address.

---

## 🧪 Testing Applications

### Simple Demo (Included)

The example sketch includes a simple counter demo:
- Displays a counter
- Has a clickable button
- Updates in real-time
- Shows connection status

### Running Full Apps

To run actual remu.ii applications in web mode:

1. Enable **WEB_MODE** in `Config.h`
2. Applications will automatically use `WebDisplay` instead of `DisplayManager`
3. Touch events from browser are translated to `TouchPoint` structures
4. All display commands are sent via WebSocket

**Note**: Some apps may require modifications to work without SD card or other hardware.

---

## 📊 Performance

### Expected Performance

- **Frame Rate**: 15-20 FPS (depending on app complexity)
- **Latency**: 50-100ms touch response
- **Network**: Works on 2.4GHz WiFi (ESP32 limitation)
- **Range**: ~30 feet from ESP32

### Optimizing Performance

**Reduce WebSocket traffic**:
```cpp
// Batch drawing commands
webDisplay.clearScreen(COLOR_BLACK);
// Multiple draw calls...
// All sent together
```

**Lower frame rate** if needed:
```cpp
unsigned long lastUpdate = 0;
if (millis() - lastUpdate >= 100) {  // 10 FPS
    // Render
    lastUpdate = millis();
}
```

---

## 🐛 Troubleshooting

### Can't Connect to WiFi Network

**Problem**: "remu.ii" network not appearing

**Solutions**:
1. Check serial monitor - is ESP32 starting correctly?
2. ESP32 must be powered and running (LED should be on)
3. Try restarting ESP32 (press EN button)
4. Check ESP32 antenna (some modules have internal, some external)

---

### Can't Access Web Page

**Problem**: Browser shows "Can't reach this page"

**Solutions**:
1. Verify you're connected to "remu.ii" WiFi network
2. Try `http://192.168.4.1` exactly (not https://)
3. Check serial monitor for actual IP address
4. Disable VPN if active
5. Try different browser (Chrome recommended)
6. Clear browser cache and cookies

---

### Display Not Updating

**Problem**: Web page loads but screen is black or frozen

**Solutions**:
1. Check browser console for errors (F12 → Console)
2. Verify WebSocket connection (look for green "Connected" status)
3. Refresh page (F5)
4. Check serial monitor for errors
5. Restart ESP32

---

### Touch Not Working

**Problem**: Clicks don't register

**Solutions**:
1. Verify WebSocket is connected (check status indicator)
2. Look for touch coordinates updating when you click
3. Check browser console (F12) for JavaScript errors
4. Ensure clicking on canvas area, not outside it
5. Try different browser

---

### Slow Performance

**Problem**: Display updates slowly or lags

**Solutions**:
1. Move closer to ESP32 (reduce WiFi distance)
2. Close other WiFi-heavy applications
3. Reduce drawing commands per frame
4. Lower frame rate in code
5. Use Chrome browser (best WebSocket performance)

---

## 💻 Supported Browsers

| Browser | Support | Notes |
|---------|---------|-------|
| **Chrome** | ✅ Excellent | Best performance, recommended |
| **Firefox** | ✅ Good | Fully supported |
| **Safari** | ✅ Good | Works on macOS and iOS |
| **Edge** | ✅ Good | Chromium-based, same as Chrome |
| **Opera** | ✅ Good | Chromium-based |
| **Mobile** | ⚠️ Limited | Touch events may differ |

---

## 📱 Mobile Access

You can access web mode from smartphones and tablets:

1. Connect mobile device to "remu.ii" WiFi
2. Open mobile browser
3. Navigate to `http://192.168.4.1`
4. Use finger instead of mouse for touch

**Note**: Mobile browser touch handling may differ slightly from desktop.

---

## 🔒 Security Considerations

### Default Security

- **WiFi is password-protected** (default: `remuiiweb`)
- **Local network only** (no internet exposure)
- **No data collection** (all processing on ESP32)

### Recommended Changes

For production use:
1. **Change default password** in config
2. **Use strong password** (12+ characters)
3. **Don't expose to internet** (keep local only)

---

## 🎨 Customizing the Interface

### Changing Colors

Edit the HTML in `WebDisplay.cpp`, find the CSS section:

```css
.device {
    border: 8px solid #cc0000;  /* Red border */
}

.status-bar {
    background: #ffeeee;  /* Light pink background */
    color: #cc0000;  /* Red text */
}
```

Change hex color codes to your preference.

### Changing Size

Modify canvas dimensions:

```javascript
<canvas id="display" width="320" height="240"></canvas>
```

And update CSS:
```css
#display {
    width: 320px;
    height: 240px;
}
```

---

## 🚀 Advanced Usage

### Running Multiple Instances

You can run multiple ESP32s with web mode:

1. Change SSID for each device:
   ```cpp
   webDisplay.initialize("remu.ii-1", "password1");
   webDisplay.initialize("remu.ii-2", "password2");
   ```

2. Each will have its own IP address
3. Connect to different networks for each device

### Recording Display

To record the web display:

1. Use browser screen recording (Chrome: Extensions)
2. Or use OS screen recording (OBS, QuickTime, etc.)
3. Record the browser window with remu.ii interface

### Headless Mode

Run ESP32 without serial monitor:

1. Upload firmware
2. Power ESP32 from USB power adapter
3. ESP32 will start WiFi automatically
4. Connect from any device on network

---

## 🔄 Switching Between Modes

### From Hardware to Web Mode

1. Open `Config.h`
2. Comment `#define HARDWARE_MODE`
3. Uncomment `#define WEB_MODE`
4. Recompile and upload

### From Web to Hardware Mode

1. Open `Config.h`
2. Comment `#define WEB_MODE`
3. Uncomment `#define HARDWARE_MODE`
4. Recompile and upload
5. Ensure hardware is connected (display, touch, etc.)

---

## 📚 Example Applications for Web Mode

### Counter App (Included)

Simple counter with button - demonstrates:
- Drawing text
- Drawing buttons
- Handling touch events
- Real-time updates

### Digital Pet (Requires Adaptation)

To run Digital Pet in web mode:
1. Remove SD card dependencies
2. Use in-memory storage instead of files
3. Keep all core pet logic
4. Render to WebDisplay instead of DisplayManager

### Sequencer (Requires Audio Handling)

Audio in web mode requires:
1. Streaming audio data to browser
2. Using Web Audio API
3. Additional WebSocket messages
4. More complex implementation

---

## 🌟 Benefits of Web Mode

### For Development

- ✅ **Rapid iteration** - No hardware assembly needed
- ✅ **Easy debugging** - Serial monitor + browser console
- ✅ **Portable testing** - Run on any computer
- ✅ **Shared demos** - Anyone can connect and view

### For Users

- ✅ **Low cost** - Only need ESP32 ($5-15)
- ✅ **No soldering** - No technical skills required
- ✅ **Instant setup** - Running in minutes
- ✅ **Safe testing** - No risk of hardware damage

### For Education

- ✅ **Classroom friendly** - Students don't need full hardware
- ✅ **Easy deployment** - One ESP32 per class
- ✅ **Collaborative** - Multiple students can view one device
- ✅ **Learn concepts** - Understand remu.ii without hardware investment

---

## 📖 Additional Resources

- **[USER_MANUAL.md](USER_MANUAL.md)** - Full application documentation
- **[INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md)** - Detailed Arduino IDE setup
- **[API_REFERENCE.md](API_REFERENCE.md)** - Developer documentation
- **[README.md](README.md)** - Project overview

---

## 🤝 Contributing

Want to improve web mode?

Ideas:
- Better UI styling
- Audio support in browser
- WebRTC for lower latency
- Mobile app wrapper
- Multiple simultaneous clients
- Recording/playback functionality

Submit pull requests on GitHub!

---

## ❓ FAQ

### Q: Does web mode support all features?
**A:** Most features work, but some require hardware (SD card, battery monitoring, RF tools). Apps can be adapted to work without these.

### Q: Can I use this over the internet?
**A:** Yes, but requires port forwarding and proper security. Not recommended without VPN.

### Q: Is web mode slower than hardware mode?
**A:** Slightly. Network latency adds 50-100ms, but it's still very usable.

### Q: Can multiple people connect at once?
**A:** Yes! WebSocket supports multiple clients. Each person sees the same display and can interact.

### Q: Do I need an ESP32 with more RAM for web mode?
**A:** No. Web mode actually uses *less* RAM than hardware mode (no display buffer).

### Q: Can I use ESP8266 instead of ESP32?
**A:** Possibly, but not tested. ESP32 is strongly recommended for RAM and dual-core processing.

---

**Enjoy running remu.ii in your browser!**

*Web mode - because not everyone has a soldering iron.* ⚡🌐

---

**Last Updated**: 2025-11-12
**Version**: 1.0
**Tested On**: ESP32 WROOM-32, Chrome/Firefox/Safari
