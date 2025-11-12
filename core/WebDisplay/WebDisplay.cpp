#include "WebDisplay.h"

WebDisplay* WebDisplay::instance = nullptr;

WebDisplay::WebDisplay() : server(nullptr), wsServer(nullptr), initialized(false),
                           touchPending(false) {
    instance = this;
    lastTouch = {0, 0, false, 0};
}

WebDisplay::~WebDisplay() {
    shutdown();
}

bool WebDisplay::initialize(const char* ssid, const char* password) {
    Serial.println("[WebDisplay] Initializing web display mode...");

    // Start WiFi access point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("[WebDisplay] AP IP address: ");
    Serial.println(IP);
    Serial.printf("[WebDisplay] Connect to WiFi: %s\n", ssid);
    Serial.printf("[WebDisplay] Password: %s\n", password);
    Serial.printf("[WebDisplay] Then open browser to: http://%s\n", IP.toString().c_str());

    // Initialize web server
    server = new WebServer(80);
    server->on("/", [this]() { handleRoot(); });
    server->onNotFound([this]() { handleNotFound(); });
    server->begin();
    Serial.println("[WebDisplay] HTTP server started on port 80");

    // Initialize WebSocket server
    wsServer = new WebSocketsServer(81);
    wsServer->begin();
    wsServer->onEvent(webSocketEvent);
    Serial.println("[WebDisplay] WebSocket server started on port 81");

    initialized = true;
    return true;
}

void WebDisplay::update() {
    if (!initialized) return;

    server->handleClient();
    wsServer->loop();
}

void WebDisplay::shutdown() {
    if (server) {
        server->stop();
        delete server;
        server = nullptr;
    }
    if (wsServer) {
        wsServer->close();
        delete wsServer;
        wsServer = nullptr;
    }
    WiFi.softAPdisconnect(true);
    initialized = false;
    Serial.println("[WebDisplay] Shutdown complete");
}

void WebDisplay::webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (instance) {
        instance->handleWebSocketMessage(num, payload, length);
    }
}

void WebDisplay::handleWebSocketMessage(uint8_t clientNum, uint8_t* payload, size_t length) {
    String message = String((char*)payload);

    // Parse JSON message
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.println("[WebDisplay] Failed to parse WebSocket message");
        return;
    }

    String type = doc["type"] | "";

    if (type == "touch") {
        // Touch event from web client
        lastTouch.x = doc["x"] | 0;
        lastTouch.y = doc["y"] | 0;
        lastTouch.pressed = doc["pressed"] | false;
        lastTouch.timestamp = millis();
        touchPending = true;

        Serial.printf("[WebDisplay] Touch: (%d, %d) pressed=%d\n",
                     lastTouch.x, lastTouch.y, lastTouch.pressed);
    }
}

void WebDisplay::sendDisplayUpdate(String command) {
    if (!initialized || !wsServer) return;
    wsServer->broadcastTXT(command);
}

// Display methods - send commands to web client
void WebDisplay::clearScreen(uint16_t color) {
    DynamicJsonDocument doc(128);
    doc["cmd"] = "clear";
    doc["color"] = color;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

void WebDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
    DynamicJsonDocument doc(128);
    doc["cmd"] = "pixel";
    doc["x"] = x;
    doc["y"] = y;
    doc["color"] = color;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

void WebDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    DynamicJsonDocument doc(192);
    doc["cmd"] = "line";
    doc["x0"] = x0;
    doc["y0"] = y0;
    doc["x1"] = x1;
    doc["y1"] = y1;
    doc["color"] = color;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

void WebDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    DynamicJsonDocument doc(192);
    doc["cmd"] = "rect";
    doc["x"] = x;
    doc["y"] = y;
    doc["w"] = w;
    doc["h"] = h;
    doc["color"] = color;
    doc["fill"] = false;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

void WebDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    DynamicJsonDocument doc(192);
    doc["cmd"] = "rect";
    doc["x"] = x;
    doc["y"] = y;
    doc["w"] = w;
    doc["h"] = h;
    doc["color"] = color;
    doc["fill"] = true;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

void WebDisplay::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    DynamicJsonDocument doc(192);
    doc["cmd"] = "circle";
    doc["x"] = x;
    doc["y"] = y;
    doc["r"] = r;
    doc["color"] = color;
    doc["fill"] = false;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

void WebDisplay::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    DynamicJsonDocument doc(192);
    doc["cmd"] = "circle";
    doc["x"] = x;
    doc["y"] = y;
    doc["r"] = r;
    doc["color"] = color;
    doc["fill"] = true;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

void WebDisplay::drawText(int16_t x, int16_t y, String text, uint16_t color, uint8_t size) {
    DynamicJsonDocument doc(512);
    doc["cmd"] = "text";
    doc["x"] = x;
    doc["y"] = y;
    doc["text"] = text;
    doc["color"] = color;
    doc["size"] = size;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

void WebDisplay::drawButton(int16_t x, int16_t y, int16_t w, int16_t h,
                           String label, bool pressed, uint16_t color) {
    DynamicJsonDocument doc(512);
    doc["cmd"] = "button";
    doc["x"] = x;
    doc["y"] = y;
    doc["w"] = w;
    doc["h"] = h;
    doc["label"] = label;
    doc["pressed"] = pressed;
    doc["color"] = color;

    String output;
    serializeJson(doc, output);
    sendDisplayUpdate(output);
}

// Touch input
bool WebDisplay::hasTouchEvent() {
    return touchPending;
}

void WebDisplay::getTouchEvent(int16_t& x, int16_t& y, bool& pressed) {
    x = lastTouch.x;
    y = lastTouch.y;
    pressed = lastTouch.pressed;
}

void WebDisplay::clearTouchEvent() {
    touchPending = false;
}

String WebDisplay::getIPAddress() {
    return WiFi.softAPIP().toString();
}

bool WebDisplay::isClientConnected() {
    return wsServer && wsServer->connectedClients() > 0;
}

void WebDisplay::handleRoot() {
    // Serve the main HTML page (embedded)
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>remu.ii - Virtual Display</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }

        body {
            font-family: 'Courier New', monospace;
            background: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            padding: 20px;
        }

        .device {
            background: #ffffff;
            border: 8px solid #cc0000;
            border-radius: 24px;
            padding: 20px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.3);
            max-width: 400px;
        }

        .screen-bezel {
            background: #2a2a2a;
            border: 4px solid #cc0000;
            border-radius: 12px;
            padding: 12px;
            box-shadow: inset 0 0 20px rgba(0,0,0,0.8);
        }

        #display {
            width: 320px;
            height: 240px;
            background: #000000;
            border: 2px solid #550000;
            cursor: crosshair;
            image-rendering: pixelated;
            display: block;
        }

        .status-bar {
            margin-top: 15px;
            padding: 10px;
            background: #ffeeee;
            border: 2px solid #cc0000;
            border-radius: 8px;
            font-size: 11px;
            color: #cc0000;
        }

        .status-item {
            display: flex;
            justify-content: space-between;
            margin: 3px 0;
        }

        .status-label {
            font-weight: bold;
        }

        .dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            display: inline-block;
            margin-right: 5px;
        }

        .dot.on {
            background: #00ff00;
            box-shadow: 0 0 5px #00ff00;
        }

        .dot.off {
            background: #333333;
        }

        .title {
            text-align: center;
            margin-bottom: 15px;
            color: #cc0000;
            font-size: 18px;
            font-weight: bold;
            text-transform: uppercase;
            letter-spacing: 2px;
        }

        .subtitle {
            text-align: center;
            margin-bottom: 10px;
            color: #666666;
            font-size: 10px;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }

        .pulse {
            animation: pulse 1s infinite;
        }
    </style>
</head>
<body>
    <div class="device">
        <div class="title">remu.ii</div>
        <div class="subtitle">Virtual Display</div>

        <div class="screen-bezel">
            <canvas id="display" width="320" height="240"></canvas>
        </div>

        <div class="status-bar">
            <div class="status-item">
                <span class="status-label">Connection:</span>
                <span><span class="dot off" id="connDot"></span><span id="connStatus">Connecting...</span></span>
            </div>
            <div class="status-item">
                <span class="status-label">Touch:</span>
                <span id="touchPos">---</span>
            </div>
            <div class="status-item">
                <span class="status-label">FPS:</span>
                <span id="fps">0</span>
            </div>
        </div>
    </div>

    <script>
        const canvas = document.getElementById('display');
        const ctx = canvas.getContext('2d');
        let ws = null;
        let frameCount = 0;
        let lastFpsUpdate = Date.now();

        // Color conversion: RGB565 to RGB888
        function rgb565ToRgb(color) {
            const r = ((color >> 11) & 0x1F) << 3;
            const g = ((color >> 5) & 0x3F) << 2;
            const b = (color & 0x1F) << 3;
            return `rgb(${r},${g},${b})`;
        }

        // Initialize WebSocket
        function connectWebSocket() {
            ws = new WebSocket(`ws://${window.location.hostname}:81`);

            ws.onopen = () => {
                console.log('WebSocket connected');
                document.getElementById('connStatus').textContent = 'Connected';
                document.getElementById('connDot').classList.remove('off');
                document.getElementById('connDot').classList.add('on');
            };

            ws.onclose = () => {
                console.log('WebSocket disconnected');
                document.getElementById('connStatus').textContent = 'Disconnected';
                document.getElementById('connDot').classList.remove('on');
                document.getElementById('connDot').classList.add('off');
                setTimeout(connectWebSocket, 2000);
            };

            ws.onerror = (error) => {
                console.error('WebSocket error:', error);
            };

            ws.onmessage = (event) => {
                handleDisplayCommand(event.data);
                frameCount++;
            };
        }

        // Handle display commands from ESP32
        function handleDisplayCommand(data) {
            try {
                const cmd = JSON.parse(data);
                const color = rgb565ToRgb(cmd.color || 0);

                switch(cmd.cmd) {
                    case 'clear':
                        ctx.fillStyle = color;
                        ctx.fillRect(0, 0, 320, 240);
                        break;

                    case 'pixel':
                        ctx.fillStyle = color;
                        ctx.fillRect(cmd.x, cmd.y, 1, 1);
                        break;

                    case 'line':
                        ctx.strokeStyle = color;
                        ctx.lineWidth = 1;
                        ctx.beginPath();
                        ctx.moveTo(cmd.x0, cmd.y0);
                        ctx.lineTo(cmd.x1, cmd.y1);
                        ctx.stroke();
                        break;

                    case 'rect':
                        if (cmd.fill) {
                            ctx.fillStyle = color;
                            ctx.fillRect(cmd.x, cmd.y, cmd.w, cmd.h);
                        } else {
                            ctx.strokeStyle = color;
                            ctx.lineWidth = 1;
                            ctx.strokeRect(cmd.x, cmd.y, cmd.w, cmd.h);
                        }
                        break;

                    case 'circle':
                        ctx.beginPath();
                        ctx.arc(cmd.x, cmd.y, cmd.r, 0, 2 * Math.PI);
                        if (cmd.fill) {
                            ctx.fillStyle = color;
                            ctx.fill();
                        } else {
                            ctx.strokeStyle = color;
                            ctx.lineWidth = 1;
                            ctx.stroke();
                        }
                        break;

                    case 'text':
                        ctx.fillStyle = color;
                        ctx.font = `${cmd.size * 8}px "Courier New", monospace`;
                        ctx.fillText(cmd.text, cmd.x, cmd.y + (cmd.size * 8));
                        break;

                    case 'button':
                        // Draw button
                        const pressed = cmd.pressed || false;
                        const offset = pressed ? 2 : 0;

                        // Background
                        ctx.fillStyle = pressed ? rgb565ToRgb(0x2104) : color;
                        ctx.fillRect(cmd.x + offset, cmd.y + offset, cmd.w, cmd.h);

                        // Border
                        ctx.strokeStyle = color;
                        ctx.lineWidth = 2;
                        ctx.strokeRect(cmd.x + offset, cmd.y + offset, cmd.w, cmd.h);

                        // Label
                        ctx.fillStyle = '#ffffff';
                        ctx.font = '12px "Courier New", monospace';
                        ctx.textAlign = 'center';
                        ctx.textBaseline = 'middle';
                        ctx.fillText(cmd.label, cmd.x + cmd.w/2 + offset, cmd.y + cmd.h/2 + offset);
                        ctx.textAlign = 'left';
                        ctx.textBaseline = 'alphabetic';
                        break;
                }
            } catch (e) {
                console.error('Error parsing command:', e);
            }
        }

        // Handle mouse/touch input
        canvas.addEventListener('mousedown', handleTouch);
        canvas.addEventListener('touchstart', handleTouch);
        canvas.addEventListener('mouseup', handleRelease);
        canvas.addEventListener('touchend', handleRelease);

        function handleTouch(e) {
            e.preventDefault();
            const rect = canvas.getBoundingClientRect();
            const x = Math.floor((e.clientX || e.touches[0].clientX) - rect.left);
            const y = Math.floor((e.clientY || e.touches[0].clientY) - rect.top);

            document.getElementById('touchPos').textContent = `(${x}, ${y})`;

            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({type: 'touch', x: x, y: y, pressed: true}));
            }
        }

        function handleRelease(e) {
            e.preventDefault();
            document.getElementById('touchPos').textContent = '---';

            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({type: 'touch', x: 0, y: 0, pressed: false}));
            }
        }

        // FPS counter
        setInterval(() => {
            const now = Date.now();
            const elapsed = (now - lastFpsUpdate) / 1000;
            const fps = Math.round(frameCount / elapsed);
            document.getElementById('fps').textContent = fps;
            frameCount = 0;
            lastFpsUpdate = now;
        }, 1000);

        // Start
        connectWebSocket();

        // Clear screen to black on load
        ctx.fillStyle = '#000000';
        ctx.fillRect(0, 0, 320, 240);
    </script>
</body>
</html>
)rawliteral";

    server->send(200, "text/html", html);
}

void WebDisplay::handleNotFound() {
    server->send(404, "text/plain", "Not Found");
}
