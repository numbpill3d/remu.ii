#include "DisplayManager.h"
#include "../SystemCore/SystemCore.h"

// Global instance
DisplayManager displayManager;

DisplayManager::DisplayManager() : 
    tft(nullptr),
    initialized(false),
    brightness(255),
    currentFont(FONT_MEDIUM),
    screenBuffer(nullptr),
    bufferEnabled(false),
    backgroundColor(COLOR_BLACK),
    foregroundColor(COLOR_WHITE)
{
}

DisplayManager::~DisplayManager() {
    shutdown();
}

bool DisplayManager::initialize() {
    Serial.println("[DisplayManager] Initializing ILI9341 display...");
    
    // Initialize TFT display
    tft = new Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
    
    if (!tft) {
        Serial.println("[DisplayManager] ERROR: Failed to create TFT instance");
        return false;
    }
    
    // Initialize SPI and display
    tft->begin();
    tft->setRotation(SCREEN_ROTATION);
    tft->fillScreen(COLOR_BLACK);
    
    // Set initial font
    setFont(FONT_MEDIUM);
    
    // Display boot logo
    drawBootLogo();
    
    initialized = true;
    Serial.println("[DisplayManager] Display initialized successfully");
    Serial.printf("[DisplayManager] Resolution: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    
    return true;
}

void DisplayManager::update() {
    // Nothing to update continuously for now
    // Could add screen saver, brightness adjustment, etc.
}

void DisplayManager::shutdown() {
    if (tft) {
        tft->fillScreen(COLOR_BLACK);
        delete tft;
        tft = nullptr;
    }
    
    if (screenBuffer) {
        free(screenBuffer);
        screenBuffer = nullptr;
    }
    
    initialized = false;
    Serial.println("[DisplayManager] Display shutdown complete");
}

void DisplayManager::clearScreen(uint16_t color) {
    if (!initialized || !tft) return;
    tft->fillScreen(color);
    backgroundColor = color;
}

void DisplayManager::setBrightness(uint8_t level) {
    brightness = level;
    // Note: ILI9341 doesn't have built-in brightness control
    // This would require PWM on the backlight pin or external circuit
}

void DisplayManager::setRotation(uint8_t rotation) {
    if (!initialized || !tft) return;
    tft->setRotation(rotation);
}

uint16_t DisplayManager::rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void DisplayManager::setFont(uint8_t font) {
    if (!initialized || !tft) return;
    
    currentFont = font;
    switch (font) {
        case FONT_SMALL:
            tft->setTextSize(1);
            break;
        case FONT_MEDIUM:
            tft->setTextSize(2);
            break;
        case FONT_LARGE:
            tft->setTextSize(3);
            break;
        default:
            tft->setTextSize(2);
            break;
    }
}

void DisplayManager::drawText(int16_t x, int16_t y, String text, uint16_t color) {
    if (!initialized || !tft) return;
    
    tft->setCursor(x, y);
    tft->setTextColor(color);
    tft->print(text);
}

void DisplayManager::drawTextCentered(int16_t x, int16_t y, int16_t w, String text, uint16_t color) {
    if (!initialized || !tft) return;
    
    int16_t textWidth = getTextWidth(text);
    int16_t centeredX = x + (w - textWidth) / 2;
    drawText(centeredX, y, text, color);
}

void DisplayManager::drawTerminalText(int16_t x, int16_t y, String text, uint16_t color) {
    // Terminal text with monospace appearance
    setFont(FONT_SMALL);
    drawText(x, y, text, color);
}

int16_t DisplayManager::getTextWidth(String text) {
    if (!initialized || !tft) return 0;
    
    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    return w;
}

int16_t DisplayManager::getTextHeight() {
    if (!initialized || !tft) return 0;
    
    switch (currentFont) {
        case FONT_SMALL: return 8;
        case FONT_MEDIUM: return 16;
        case FONT_LARGE: return 24;
        default: return 16;
    }
}

void DisplayManager::drawButton(Button& button) {
    drawButton(button.x, button.y, button.w, button.h, button.text, button.state, button.color);
}

void DisplayManager::drawButton(int16_t x, int16_t y, int16_t w, int16_t h, String text, ButtonState state, uint16_t color) {
    if (!initialized || !tft) return;
    
    // Draw button background
    tft->fillRect(x, y, w, h, color);
    
    // Draw 3D border effect
    bool inset = (state == BUTTON_PRESSED);
    drawBorder3D(x, y, w, h, inset);
    
    // Draw button text
    uint16_t textColor = COLOR_WHITE;
    if (state == BUTTON_DISABLED) {
        textColor = COLOR_LIGHT_GRAY;
    } else if (state == BUTTON_HIGHLIGHTED) {
        textColor = COLOR_RED_GLOW;
    }
    
    setFont(FONT_MEDIUM);
    drawTextCentered(x, y + (h - getTextHeight()) / 2, w, text, textColor);
    
    // Add glow effect for highlighted buttons
    if (state == BUTTON_HIGHLIGHTED) {
        drawGlowEffect(x, y, w, h, COLOR_RED_GLOW);
    }
}

void DisplayManager::drawWindow(Window& window) {
    drawWindow(window.x, window.y, window.w, window.h, window.title, window.type);
}

void DisplayManager::drawWindow(int16_t x, int16_t y, int16_t w, int16_t h, String title, WindowType type) {
    if (!initialized || !tft) return;
    
    // Window background
    uint16_t bgColor = COLOR_BLACK;
    uint16_t borderColor = COLOR_DARK_GRAY;
    
    switch (type) {
        case WINDOW_TERMINAL:
            bgColor = COLOR_BLACK;
            borderColor = COLOR_GREEN_PHOS;
            break;
        case WINDOW_DIALOG:
            bgColor = COLOR_DARK_GRAY;
            borderColor = COLOR_RED_GLOW;
            break;
        case WINDOW_POPUP:
            bgColor = COLOR_MID_GRAY;
            borderColor = COLOR_PURPLE_GLOW;
            break;
        default:
            break;
    }
    
    // Fill window background
    tft->fillRect(x, y, w, h, bgColor);
    
    // Draw border
    tft->drawRect(x, y, w, h, borderColor);
    tft->drawRect(x + 1, y + 1, w - 2, h - 2, borderColor);
    
    // Draw title bar if title provided
    if (title.length() > 0) {
        tft->fillRect(x + 2, y + 2, w - 4, TITLE_BAR_HEIGHT, COLOR_MID_GRAY);
        drawBorder3D(x + 2, y + 2, w - 4, TITLE_BAR_HEIGHT, false);
        
        setFont(FONT_SMALL);
        drawText(x + 6, y + 6, title, COLOR_WHITE);
    }
}

void DisplayManager::drawProgressBar(ProgressBar& progressBar) {
    drawProgressBar(progressBar.x, progressBar.y, progressBar.w, progressBar.h, 
                   progressBar.progress, progressBar.fillColor, progressBar.bgColor);
}

void DisplayManager::drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t progress, uint16_t fillColor, uint16_t bgColor) {
    if (!initialized || !tft) return;
    
    // Background
    tft->fillRect(x, y, w, h, bgColor);
    
    // Border
    tft->drawRect(x, y, w, h, COLOR_DARK_GRAY);
    
    // Progress fill
    int16_t fillWidth = (w - 4) * progress / 100;
    if (fillWidth > 0) {
        tft->fillRect(x + 2, y + 2, fillWidth, h - 4, fillColor);
        
        // Add glow effect to progress
        if (fillWidth > 2) {
            tft->drawFastHLine(x + 2, y + 2, fillWidth, COLOR_WHITE);
        }
    }
    
    // Progress text
    setFont(FONT_SMALL);
    String progressText = String(progress) + "%";
    drawTextCentered(x, y + (h - 8) / 2, w, progressText, COLOR_WHITE);
}

void DisplayManager::drawBorder3D(int16_t x, int16_t y, int16_t w, int16_t h, bool inset) {
    if (!initialized || !tft) return;
    
    uint16_t lightColor = inset ? COLOR_DARK_GRAY : COLOR_WHITE;
    uint16_t darkColor = inset ? COLOR_WHITE : COLOR_DARK_GRAY;
    
    // Top and left edges (light)
    tft->drawFastHLine(x, y, w - 1, lightColor);
    tft->drawFastVLine(x, y, h - 1, lightColor);
    
    // Bottom and right edges (dark)
    tft->drawFastHLine(x + 1, y + h - 1, w - 1, darkColor);
    tft->drawFastVLine(x + w - 1, y + 1, h - 1, darkColor);
}

void DisplayManager::drawGlowEffect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!initialized || !tft) return;
    
    // Simple glow effect - draw faded border
    uint8_t alpha = 128;
    for (int i = 0; i < 3; i++) {
        tft->drawRect(x - i, y - i, w + 2 * i, h + 2 * i, color);
        alpha /= 2;
    }
}

void DisplayManager::drawScrollbar(int16_t x, int16_t y, int16_t h, uint8_t position, uint8_t size) {
    if (!initialized || !tft) return;
    
    // Scrollbar background
    tft->fillRect(x, y, SCROLL_BAR_WIDTH, h, COLOR_DARK_GRAY);
    tft->drawRect(x, y, SCROLL_BAR_WIDTH, h, COLOR_MID_GRAY);
    
    // Scrollbar thumb
    int16_t thumbHeight = h * size / 100;
    int16_t thumbY = y + (h - thumbHeight) * position / 100;
    
    tft->fillRect(x + 1, thumbY, SCROLL_BAR_WIDTH - 2, thumbHeight, COLOR_MID_GRAY);
    drawBorder3D(x + 1, thumbY, SCROLL_BAR_WIDTH - 2, thumbHeight, false);
}

void DisplayManager::drawCheckbox(int16_t x, int16_t y, bool checked, String label) {
    if (!initialized || !tft) return;
    
    // Checkbox square
    tft->fillRect(x, y, 12, 12, COLOR_WHITE);
    tft->drawRect(x, y, 12, 12, COLOR_DARK_GRAY);
    drawBorder3D(x, y, 12, 12, true);
    
    // Check mark
    if (checked) {
        tft->drawLine(x + 2, y + 6, x + 5, y + 9, COLOR_RED_GLOW);
        tft->drawLine(x + 5, y + 9, x + 10, y + 3, COLOR_RED_GLOW);
    }
    
    // Label
    if (label.length() > 0) {
        setFont(FONT_MEDIUM);
        drawText(x + 16, y - 2, label, COLOR_WHITE);
    }
}

void DisplayManager::drawGlitch(int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!initialized || !tft) return;
    
    // Random glitch lines
    for (int i = 0; i < 5; i++) {
        int16_t glitchY = y + systemCore.getRandomByte() % h;
        int16_t glitchW = systemCore.getRandomByte() % w;
        uint16_t glitchColor = (systemCore.getRandomByte() % 2) ? COLOR_RED_GLOW : COLOR_PURPLE_GLOW;
        
        tft->drawFastHLine(x, glitchY, glitchW, glitchColor);
    }
}

void DisplayManager::drawScanlines(int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!initialized || !tft) return;
    
    // Draw horizontal scanlines
    for (int16_t i = y; i < y + h; i += 2) {
        tft->drawFastHLine(x, i, w, COLOR_DARK_GRAY);
    }
}

void DisplayManager::drawNoise(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t intensity) {
    if (!initialized || !tft) return;
    
    for (int i = 0; i < intensity; i++) {
        int16_t noiseX = x + systemCore.getRandomByte() % w;
        int16_t noiseY = y + systemCore.getRandomByte() % h;
        uint16_t noiseColor = systemCore.getRandomByte() % 2 ? COLOR_WHITE : COLOR_BLACK;
        
        tft->drawPixel(noiseX, noiseY, noiseColor);
    }
}

void DisplayManager::drawASCIIBorder(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!initialized || !tft) return;
    
    setFont(FONT_SMALL);
    
    // Top border
    drawText(x, y, "+", color);
    for (int i = 1; i < w / 6 - 1; i++) {
        drawText(x + i * 6, y, "-", color);
    }
    drawText(x + w - 6, y, "+", color);
    
    // Bottom border
    drawText(x, y + h - 8, "+", color);
    for (int i = 1; i < w / 6 - 1; i++) {
        drawText(x + i * 6, y + h - 8, "-", color);
    }
    drawText(x + w - 6, y + h - 8, "+", color);
    
    // Side borders
    for (int i = 1; i < h / 8 - 1; i++) {
        drawText(x, y + i * 8, "|", color);
        drawText(x + w - 6, y + i * 8, "|", color);
    }
}

void DisplayManager::drawBootLogo() {
    if (!initialized || !tft) return;
    
    clearScreen(COLOR_BLACK);
    
    // ASCII art logo
    setFont(FONT_MEDIUM);
    
    String logo[] = {
        "  ┌─────────────────┐",
        "  │   remu.ii v1.0  │",
        "  │                 │", 
        "  │  ░░░░░░░░░░░░░  │",
        "  │  ░ ANTI-PHONE ░  │",
        "  │  ░  FRAMEWORK ░  │",
        "  │  ░░░░░░░░░░░░░  │",
        "  │                 │",
        "  │ Loading system.. │",
        "  └─────────────────┘"
    };
    
    int16_t startY = 60;
    for (int i = 0; i < 10; i++) {
        drawText(20, startY + i * 16, logo[i], COLOR_GREEN_PHOS);
        delay(100); // Animated loading effect
    }
    
    // Loading bar
    for (int i = 0; i <= 100; i += 5) {
        drawProgressBar(50, 220, 220, 16, i, COLOR_RED_GLOW, COLOR_DARK_GRAY);
        delay(50);
    }
    
    delay(1000);
    clearScreen(COLOR_BLACK);
}

void DisplayManager::drawTestPattern() {
    if (!initialized || !tft) return;
    
    clearScreen(COLOR_BLACK);
    
    // Color bars
    int16_t barHeight = SCREEN_HEIGHT / 6;
    uint16_t colors[] = {COLOR_WHITE, COLOR_RED_GLOW, COLOR_PURPLE_GLOW, 
                        COLOR_GREEN_PHOS, COLOR_BLUE_CYBER, COLOR_DARK_GRAY};
    
    for (int i = 0; i < 6; i++) {
        tft->fillRect(0, i * barHeight, SCREEN_WIDTH, barHeight, colors[i]);
    }
    
    // Test text
    setFont(FONT_LARGE);
    drawTextCentered(0, SCREEN_HEIGHT / 2 - 12, SCREEN_WIDTH, "TEST PATTERN", COLOR_BLACK);
}

void DisplayManager::enableBuffer(bool enable) {
    if (enable && !screenBuffer) {
        screenBuffer = (uint16_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 2);
        if (screenBuffer) {
            bufferEnabled = true;
            Serial.println("[DisplayManager] Screen buffer enabled");
        }
    } else if (!enable && screenBuffer) {
        free(screenBuffer);
        screenBuffer = nullptr;
        bufferEnabled = false;
        Serial.println("[DisplayManager] Screen buffer disabled");
    }
}