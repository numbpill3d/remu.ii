#include "DigitalPet.h"

// ========================================
// SPRITE DATA - 16x16 1-bit bitmaps
// ========================================

const uint8_t DigitalPetApp::PET_SPRITE_IDLE[32] = {
    0x00, 0x00, 0x07, 0xE0, 0x18, 0x18, 0x20, 0x04, 0x47, 0xE2, 0x4C, 0x32,
    0x4C, 0x32, 0x47, 0xE2, 0x40, 0x02, 0x20, 0x04, 0x18, 0x18, 0x07, 0xE0,
    0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

const uint8_t DigitalPetApp::PET_SPRITE_HAPPY[32] = {
    0x00, 0x00, 0x07, 0xE0, 0x18, 0x18, 0x20, 0x04, 0x47, 0xE2, 0x4C, 0x32,
    0x4C, 0x32, 0x47, 0xE2, 0x41, 0x82, 0x22, 0x44, 0x1C, 0x38, 0x07, 0xE0,
    0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

const uint8_t DigitalPetApp::PET_SPRITE_SAD[32] = {
    0x00, 0x00, 0x07, 0xE0, 0x18, 0x18, 0x20, 0x04, 0x47, 0xE2, 0x4C, 0x32,
    0x4C, 0x32, 0x47, 0xE2, 0x40, 0x02, 0x38, 0x1C, 0x44, 0x22, 0x82, 0x41,
    0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

const uint8_t DigitalPetApp::PET_SPRITE_SLEEPING[32] = {
    0x00, 0x00, 0x07, 0xE0, 0x18, 0x18, 0x20, 0x04, 0x40, 0x02, 0x40, 0x02,
    0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x20, 0x04, 0x18, 0x18, 0x07, 0xE0,
    0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

const uint8_t DigitalPetApp::ACCESSORY_HAT_SPRITE[32] = {
    0x0F, 0xF0, 0x18, 0x18, 0x30, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Animation frame definitions
AnimationFrame DigitalPetApp::idleAnimation[] = {
    {PET_SPRITE_IDLE, 1000},
    {PET_SPRITE_IDLE, 1000},
    {PET_SPRITE_IDLE, 500}
};

AnimationFrame DigitalPetApp::happyAnimation[] = {
    {PET_SPRITE_HAPPY, 500},
    {PET_SPRITE_IDLE, 500},
    {PET_SPRITE_HAPPY, 500}
};

// ========================================
// CONSTRUCTOR / DESTRUCTOR
// ========================================

DigitalPetApp::DigitalPetApp() :
    selectedAction(0),
    showStats(false),
    showCustomization(false),
    lastStatsUpdate(0),
    lastAnimation(0),
    currentAnimFrame(0),
    activeTouchZone(-1),
    currentAnimation(nullptr),
    animationFrameCount(0),
    animationLoop(true)
{
    // Set app metadata
    metadata.name = "DigitalPet";
    metadata.version = "1.0";
    metadata.author = "remu.ii";
    metadata.description = "AI-driven cyberpet companion";
    metadata.category = CATEGORY_GAMES;
    metadata.maxMemory = 15000; // 15KB
    metadata.requiresSD = true;
    metadata.requiresBLE = false;
    metadata.requiresWiFi = false;
    
    // Initialize save file path
    saveFilePath = "/apps/DigitalPet/pet_data.json";
    
    // Set colors
    backgroundColor = COLOR_BLACK;
    foregroundColor = COLOR_GREEN_PHOS;
    
    showBackButton = true;
    showStatusBar = true;
}

DigitalPetApp::~DigitalPetApp() {
    cleanup();
}

// ========================================
// MANDATORY BASEAPP METHODS
// ========================================

bool DigitalPetApp::initialize() {
    debugLog("DigitalPet initializing...");
    
    setState(APP_INITIALIZING);
    
    // Create app data directory
    if (!createAppDataDir()) {
        debugLog("WARNING: Could not create app data directory");
    }
    
    // Load pet data or create new pet
    if (!loadPetData()) {
        debugLog("Creating new pet...");
        createDefaultPet();
    }
    
    // Setup touch zones for interaction
    setupTouchZones();
    
    // Start with idle animation
    setAnimation(idleAnimation, 3, true);
    
    // Initialize timing
    lastStatsUpdate = millis();
    lastAnimation = millis();
    
    setState(APP_RUNNING);
    debugLog("DigitalPet initialized successfully");
    
    return true;
}

void DigitalPetApp::update() {
    if (currentState != APP_RUNNING) return;
    
    unsigned long currentTime = millis();
    
    // Update pet stats periodically
    if (currentTime - lastStatsUpdate >= 60000) { // Every minute
        updatePetStats();
        lastStatsUpdate = currentTime;
    }
    
    // Update animation
    if (currentTime - lastAnimation >= 100) { // Animation update rate
        updateAnimation();
        lastAnimation = currentTime;
    }
    
    // Apply entropy influence
    applyEntropyInfluence();
    
    // Check pet health
    checkPetHealth();
    
    frameCount++;
}

void DigitalPetApp::render() {
    if (currentState != APP_RUNNING) return;
    
    // Clear screen
    displayManager.clearScreen(backgroundColor);
    
    if (showCustomization) {
        drawCustomizationMenu();
    } else if (showStats) {
        drawStatsDisplay();
    } else {
        // Main pet view
        drawBackground();
        drawPetRoom();
        drawPet();
        drawMoodIndicator();
        drawInteractionButtons();
    }
    
    // Draw common UI elements
    drawCommonUI();
}

bool DigitalPetApp::handleTouch(TouchPoint touch) {
    // Handle common UI first (back button, etc.)
    if (handleCommonTouch(touch)) {
        return true;
    }
    
    if (!touch.isNewPress) return false;
    
    if (showCustomization) {
        // Handle customization menu touches
        if (touch.y > 200) {
            showCustomization = false;
        }
        return true;
    }
    
    if (showStats) {
        // Handle stats display touches
        if (touch.y > 200) {
            showStats = false;
        }
        return true;
    }
    
    // Check interaction zones
    int8_t touchedZone = getTouchedZone(touch);
    if (touchedZone >= 0) {
        handleZoneTouch(touchedZone);
        return true;
    }
    
    // Direct pet interaction (petting)
    if (touch.x >= 120 && touch.x <= 200 && touch.y >= 80 && touch.y <= 160) {
        petPet();
        return true;
    }
    
    return false;
}

void DigitalPetApp::cleanup() {
    // Save pet data
    savePetData();
    
    debugLog("DigitalPet cleanup complete");
}

const uint8_t* DigitalPetApp::getIcon() const {
    return PET_SPRITE_IDLE;
}

// ========================================
// OPTIONAL BASEAPP METHODS
// ========================================

void DigitalPetApp::onPause() {
    // Save state when pausing
    savePetData();
}

void DigitalPetApp::onResume() {
    // Update stats based on time away
    unsigned long currentTime = millis();
    unsigned long timeAway = currentTime - pet.lastUpdate;
    
    if (timeAway > 60000) { // More than 1 minute away
        // Apply accelerated decay
        uint8_t minutesAway = timeAway / 60000;
        pet.stats.hunger = max(0, (int)pet.stats.hunger - (HUNGER_DECAY_RATE * minutesAway));
        pet.stats.loneliness = min(100, (int)pet.stats.loneliness + (LONELINESS_DECAY_RATE * minutesAway));
        pet.stats.sleep = max(0, (int)pet.stats.sleep - (SLEEP_DECAY_RATE * minutesAway));
        
        calculateMood();
        calculateHappiness();
    }
    
    pet.lastUpdate = currentTime;
}

bool DigitalPetApp::saveState() {
    return savePetData();
}

bool DigitalPetApp::loadState() {
    return loadPetData();
}

String DigitalPetApp::getSettingName(uint8_t index) const {
    switch (index) {
        case 0: return "Rename Pet";
        case 1: return "Customize Pet";
        case 2: return "Reset Pet";
        case 3: return "Pet Info";
        default: return "";
    }
}

void DigitalPetApp::handleSetting(uint8_t index) {
    switch (index) {
        case 0: // Rename Pet
            // Would show text input dialog
            debugLog("Rename pet selected");
            break;
        case 1: // Customize Pet
            showCustomization = true;
            break;
        case 2: // Reset Pet
            createDefaultPet();
            break;
        case 3: // Pet Info
            showStats = true;
            break;
    }
}

// ========================================
// PET LOGIC IMPLEMENTATION
// ========================================

void DigitalPetApp::updatePetStats() {
    unsigned long currentTime = millis();
    pet.lastUpdate = currentTime;
    
    // Apply stat decay
    handleStatDecay();
    
    // Calculate derived stats
    calculateMood();
    calculateHappiness();
    
    // Update activity based on mood
    if (pet.stats.sleep < 30) {
        pet.currentActivity = ACTIVITY_SLEEPING;
        setAnimation(sleepingAnimation, 1, true);
    } else if (pet.stats.hunger < 20) {
        pet.currentMood = MOOD_SAD;
    } else if (pet.stats.happiness > 80) {
        pet.currentMood = MOOD_HAPPY;
        setAnimation(happyAnimation, 3, true);
    } else {
        pet.currentActivity = ACTIVITY_IDLE;
        setAnimation(idleAnimation, 3, true);
    }
    
    // Save periodically
    if (systemCore.getUptimeSeconds() % 300 == 0) { // Every 5 minutes
        savePetData();
    }
}

void DigitalPetApp::calculateMood() {
    // Mood calculation based on stats
    uint8_t avgStats = (pet.stats.hunger + (100 - pet.stats.loneliness) + 
                       pet.stats.sleep + pet.stats.stability) / 4;
    
    if (pet.stats.health < 30) {
        pet.currentMood = MOOD_SICK;
    } else if (pet.stats.sleep < 20) {
        pet.currentMood = MOOD_SLEEPING;
    } else if (pet.stats.entropy > 80) {
        pet.currentMood = MOOD_CHAOTIC;
    } else if (avgStats > 80) {
        pet.currentMood = MOOD_HAPPY;
    } else if (avgStats > 60) {
        pet.currentMood = MOOD_CONTENT;
    } else if (avgStats > 40) {
        pet.currentMood = MOOD_NEUTRAL;
    } else if (avgStats > 20) {
        pet.currentMood = MOOD_SAD;
    } else {
        pet.currentMood = MOOD_ANGRY;
    }
}

void DigitalPetApp::calculateHappiness() {
    // Happiness is a weighted average of key stats
    pet.stats.happiness = (pet.stats.hunger * 0.3 + 
                          (100 - pet.stats.loneliness) * 0.3 +
                          pet.stats.sleep * 0.2 +
                          pet.stats.stability * 0.2);
    
    // Health affects overall happiness
    pet.stats.health = (pet.stats.happiness + pet.stats.mood) / 2;
}

void DigitalPetApp::applyEntropyInfluence() {
    // Get system entropy and influence pet
    uint32_t entropyPool = systemCore.getEntropyPool();
    uint8_t entropyInfluence = (entropyPool >> 24) & 0xFF; // Use top byte
    
    // Map entropy to 0-100 scale
    pet.stats.entropy = map(entropyInfluence, 0, 255, 0, 100);
    
    // High entropy affects stability
    if (pet.stats.entropy > 75) {
        pet.stats.stability = max(0, (int)pet.stats.stability - 1);
    } else if (pet.stats.entropy < 25) {
        pet.stats.stability = min(100, (int)pet.stats.stability + 1);
    }
}

void DigitalPetApp::handleStatDecay() {
    // Natural stat decay over time
    pet.stats.hunger = max(0, (int)pet.stats.hunger - 1);
    pet.stats.sleep = max(0, (int)pet.stats.sleep - 1);
    pet.stats.loneliness = min(100, (int)pet.stats.loneliness + 1);
    
    // Mood decays slower
    if (pet.stats.mood > 50) {
        pet.stats.mood = max(50, (int)pet.stats.mood - 1);
    }
}

void DigitalPetApp::checkPetHealth() {
    // Check if pet is still alive
    if (pet.stats.health < 10 && pet.stats.hunger < 10 && pet.stats.sleep < 10) {
        pet.isAlive = false;
        pet.currentMood = MOOD_SICK;
        debugLog("Pet has died!");
    }
}

// ========================================
// INTERACTION METHODS
// ========================================

void DigitalPetApp::feedPet() {
    if (!pet.isAlive) return;
    
    pet.stats.hunger = min(100, (int)pet.stats.hunger + FEED_AMOUNT);
    pet.stats.mood = min(100, (int)pet.stats.mood + 5);
    pet.totalInteractions++;
    
    pet.currentActivity = ACTIVITY_EATING;
    // setAnimation(eatingAnimation, 3, false); // Play once
    
    debugLog("Fed pet - hunger now: " + String(pet.stats.hunger));
}

void DigitalPetApp::petPet() {
    if (!pet.isAlive) return;
    
    pet.stats.loneliness = max(0, (int)pet.stats.loneliness - PET_AMOUNT);
    pet.stats.mood = min(100, (int)pet.stats.mood + 3);
    pet.totalInteractions++;
    
    // Show happy animation
    setAnimation(happyAnimation, 3, false);
    
    debugLog("Pet petted - loneliness now: " + String(pet.stats.loneliness));
}

void DigitalPetApp::playWithPet() {
    if (!pet.isAlive) return;
    
    pet.stats.loneliness = max(0, (int)pet.stats.loneliness - PLAY_AMOUNT);
    pet.stats.mood = min(100, (int)pet.stats.mood + 10);
    pet.stats.sleep = max(0, (int)pet.stats.sleep - 5); // Playing tires pet
    pet.totalInteractions++;
    
    pet.currentActivity = ACTIVITY_PLAYING;
    
    debugLog("Played with pet");
}

void DigitalPetApp::putPetToSleep() {
    if (!pet.isAlive) return;
    
    pet.stats.sleep = min(100, (int)pet.stats.sleep + REST_AMOUNT);
    pet.currentActivity = ACTIVITY_SLEEPING;
    
    setAnimation(sleepingAnimation, 1, true);
    
    debugLog("Pet put to sleep - sleep now: " + String(pet.stats.sleep));
}

// ========================================
// RENDERING METHODS
// ========================================

void DigitalPetApp::drawPet() {
    // Draw pet sprite in center
    int16_t petX = SCREEN_WIDTH / 2 - 16;
    int16_t petY = SCREEN_HEIGHT / 2 - 16;
    
    drawAnimatedSprite(petX, petY);
    
    // Draw accessories
    if (pet.custom.accessories & ACCESSORY_HAT) {
        displayManager.drawIcon(petX, petY - 8, ACCESSORY_HAT_SPRITE, COLOR_RED_GLOW);
    }
    
    // Draw pet name
    displayManager.setFont(FONT_MEDIUM);
    displayManager.drawTextCentered(0, 40, SCREEN_WIDTH, pet.custom.name, COLOR_GREEN_PHOS);
}

void DigitalPetApp::drawAnimatedSprite(int16_t x, int16_t y) {
    if (!currentAnimation || animationFrameCount == 0) {
        // Fallback to idle sprite
        displayManager.drawIcon(x, y, PET_SPRITE_IDLE, COLOR_WHITE);
        return;
    }
    
    // Draw current animation frame
    const uint8_t* spriteData = currentAnimation[currentAnimFrame].spriteData;
    
    // Color based on mood
    uint16_t spriteColor = COLOR_WHITE;
    switch (pet.currentMood) {
        case MOOD_HAPPY: spriteColor = COLOR_GREEN_PHOS; break;
        case MOOD_SAD: spriteColor = COLOR_BLUE_CYBER; break;
        case MOOD_ANGRY: spriteColor = COLOR_RED_GLOW; break;
        case MOOD_SICK: spriteColor = COLOR_LIGHT_GRAY; break;
        case MOOD_CHAOTIC: spriteColor = COLOR_PURPLE_GLOW; break;
        default: spriteColor = COLOR_WHITE; break;
    }
    
    displayManager.drawIcon(x, y, spriteData, spriteColor);
}

void DigitalPetApp::drawMoodIndicator() {
    // Draw mood text
    displayManager.setFont(FONT_SMALL);
    String moodText = "Mood: ";
    
    switch (pet.currentMood) {
        case MOOD_HAPPY: moodText += "Happy"; break;
        case MOOD_CONTENT: moodText += "Content"; break;
        case MOOD_NEUTRAL: moodText += "Neutral"; break;
        case MOOD_SAD: moodText += "Sad"; break;
        case MOOD_ANGRY: moodText += "Angry"; break;
        case MOOD_SLEEPING: moodText += "Sleeping"; break;
        case MOOD_SICK: moodText += "Sick"; break;
        case MOOD_CHAOTIC: moodText += "Chaotic"; break;
    }
    
    displayManager.drawText(10, 220, moodText, COLOR_GREEN_PHOS);
    
    // Draw simple ASCII mood indicator
    drawASCIIMood(280, 220, pet.currentMood);
}

void DigitalPetApp::drawStatsDisplay() {
    displayManager.clearScreen(COLOR_BLACK);
    
    // Title
    displayManager.setFont(FONT_MEDIUM);
    displayManager.drawTextCentered(0, 10, SCREEN_WIDTH, "Pet Stats", COLOR_RED_GLOW);
    
    // Stats bars
    int16_t barY = 50;
    int16_t barHeight = 12;
    int16_t barSpacing = 20;
    
    displayManager.setFont(FONT_SMALL);
    
    // Hunger
    displayManager.drawText(10, barY, "Hunger:", COLOR_WHITE);
    displayManager.drawProgressBar(80, barY, 180, barHeight, pet.stats.hunger, COLOR_GREEN_PHOS);
    barY += barSpacing;
    
    // Loneliness (inverted display)
    displayManager.drawText(10, barY, "Lonely:", COLOR_WHITE);
    displayManager.drawProgressBar(80, barY, 180, barHeight, 100 - pet.stats.loneliness, COLOR_BLUE_CYBER);
    barY += barSpacing;
    
    // Sleep
    displayManager.drawText(10, barY, "Sleep:", COLOR_WHITE);
    displayManager.drawProgressBar(80, barY, 180, barHeight, pet.stats.sleep, COLOR_PURPLE_GLOW);
    barY += barSpacing;
    
    // Happiness
    displayManager.drawText(10, barY, "Happy:", COLOR_WHITE);
    displayManager.drawProgressBar(80, barY, 180, barHeight, pet.stats.happiness, COLOR_RED_GLOW);
    barY += barSpacing;
    
    // Health
    displayManager.drawText(10, barY, "Health:", COLOR_WHITE);
    displayManager.drawProgressBar(80, barY, 180, barHeight, pet.stats.health, 
                                  pet.stats.health > 50 ? COLOR_GREEN_PHOS : COLOR_RED_GLOW);
    barY += barSpacing;
    
    // Pet info
    displayManager.drawText(10, barY + 10, "Age: " + String(getPetAge()) + " hours", COLOR_LIGHT_GRAY);
    displayManager.drawText(10, barY + 25, "Interactions: " + String(pet.totalInteractions), COLOR_LIGHT_GRAY);
    
    // Close instruction
    displayManager.drawTextCentered(0, 210, SCREEN_WIDTH, "Touch to close", COLOR_LIGHT_GRAY);
}

void DigitalPetApp::drawInteractionButtons() {
    // Draw interaction buttons at bottom
    int16_t buttonY = 180;
    int16_t buttonW = 60;
    int16_t buttonH = 20;
    int16_t spacing = 5;
    
    displayManager.setFont(FONT_SMALL);
    
    // Feed button
    displayManager.drawButton(10, buttonY, buttonW, buttonH, "Feed");
    
    // Play button  
    displayManager.drawButton(10 + buttonW + spacing, buttonY, buttonW, buttonH, "Play");
    
    // Sleep button
    displayManager.drawButton(10 + 2 * (buttonW + spacing), buttonY, buttonW, buttonH, "Sleep");
    
    // Stats button
    displayManager.drawButton(10 + 3 * (buttonW + spacing), buttonY, buttonW, buttonH, "Stats");
}

void DigitalPetApp::drawBackground() {
    // Draw retro background pattern
    displayManager.drawASCIIBorder(5, 5, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 50, COLOR_DARK_GRAY);
}

void DigitalPetApp::drawPetRoom() {
    // Draw simple room elements
    displayManager.setFont(FONT_SMALL);
    
    // Floor line
    displayManager.drawLine(20, 150, SCREEN_WIDTH - 20, 150, COLOR_MID_GRAY);
    
    // Food bowl (if hungry)
    if (pet.stats.hunger < 50) {
        displayManager.drawRetroRect(250, 140, 16, 8, COLOR_MID_GRAY, true);
        displayManager.drawText(250, 130, "FOOD", COLOR_LIGHT_GRAY);
    }
    
    // Bed (if tired)
    if (pet.stats.sleep < 50) {
        displayManager.drawRetroRect(30, 135, 30, 12, COLOR_PURPLE_GLOW, true);
        displayManager.drawText(30, 125, "BED", COLOR_LIGHT_GRAY);
    }
}

void DigitalPetApp::drawASCIIMood(int16_t x, int16_t y, PetMood mood) {
    displayManager.setFont(FONT_SMALL);
    uint16_t color = COLOR_WHITE;
    String symbol = ":)";
    
    switch (mood) {
        case MOOD_HAPPY:
            symbol = ":D";
            color = COLOR_GREEN_PHOS;
            break;
        case MOOD_CONTENT:
            symbol = ":)";
            color = COLOR_GREEN_PHOS;
            break;
        case MOOD_NEUTRAL:
            symbol = ":|";
            color = COLOR_WHITE;
            break;
        case MOOD_SAD:
            symbol = ":(";
            color = COLOR_BLUE_CYBER;
            break;
        case MOOD_ANGRY:
            symbol = ">:(";
            color = COLOR_RED_GLOW;
            break;
        case MOOD_SLEEPING:
            symbol = "zzZ";
            color = COLOR_PURPLE_GLOW;
            break;
        case MOOD_SICK:
            symbol = "X_X";
            color = COLOR_LIGHT_GRAY;
            break;
        case MOOD_CHAOTIC:
            symbol = "@_@";
            color = COLOR_PURPLE_GLOW;
            break;
    }
    
    displayManager.drawText(x, y, symbol, color);
}

// ========================================
// TOUCH HANDLING
// ========================================

void DigitalPetApp::setupTouchZones() {
    // Feed button
    touchZones[0] = {10, 180, 60, 20, "feed", true};
    
    // Play button
    touchZones[1] = {75, 180, 60, 20, "play", true};
    
    // Sleep button
    touchZones[2] = {140, 180, 60, 20, "sleep", true};
    
    // Stats button
    touchZones[3] = {205, 180, 60, 20, "stats", true};
    
    // Pet area (for petting)
    touchZones[4] = {120, 80, 80, 80, "pet", true};
    
    // Settings area
    touchZones[5] = {270, 10, 40, 20, "settings", true};
}

int8_t DigitalPetApp::getTouchedZone(TouchPoint touch) {
    for (int8_t i = 0; i < 6; i++) {
        if (touchZones[i].enabled && 
            touchInterface.isPointInRect(touch, touchZones[i].x, touchZones[i].y, 
                                       touchZones[i].w, touchZones[i].h)) {
            return i;
        }
    }
    return -1;
}

void DigitalPetApp::handleZoneTouch(int8_t zone) {
    if (zone < 0 || zone >= 6) return;
    
    String action = touchZones[zone].action;
    
    if (action == "feed") {
        feedPet();
    } else if (action == "play") {
        playWithPet();
    } else if (action == "sleep") {
        putPetToSleep();
    } else if (action == "stats") {
        showStats = true;
    } else if (action == "pet") {
        petPet();
    } else if (action == "settings") {
        showCustomization = true;
    }
}

// ========================================
// ANIMATION SYSTEM
// ========================================

void DigitalPetApp::updateAnimation() {
    if (!currentAnimation || animationFrameCount == 0) return;
    
    unsigned long currentTime = millis();
    uint16_t frameDuration = currentAnimation[currentAnimFrame].duration;
    
    if (currentTime - lastAnimation >= frameDuration) {
        currentAnimFrame++;
        
        if (currentAnimFrame >= animationFrameCount) {
            if (animationLoop) {
                currentAnimFrame = 0;
            } else {
                // Animation finished, return to idle
                setAnimation(idleAnimation, 3, true);
            }
        }
        
        lastAnimation = currentTime;
    }
}

void DigitalPetApp::setAnimation(AnimationFrame* frames, uint8_t frameCount, bool loop) {
    currentAnimation = frames;
    animationFrameCount = frameCount;
    animationLoop = loop;
    currentAnimFrame = 0;
    lastAnimation = millis();
}

// ========================================
// FILE I/O SYSTEM
// ========================================

bool DigitalPetApp::loadPetData() {
    if (!SD.exists(saveFilePath)) {
        debugLog("No save file found at: " + saveFilePath);
        return false;
    }
    
    File file = SD.open(saveFilePath, FILE_READ);
    if (!file) {
        debugLog("Failed to open save file");
        return false;
    }
    
    // Read JSON data
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        debugLog("Failed to parse save file: " + String(error.c_str()));
        return false;
    }
    
    // Validate and load data
    if (!validateSaveData(doc)) {
        debugLog("Invalid save data");
        return false;
    }
    
    // Load pet stats
    pet.stats.mood = doc["stats"]["mood"];
    pet.stats.hunger = doc["stats"]["hunger"];
    pet.stats.loneliness = doc["stats"]["loneliness"];
    pet.stats.entropy = doc["stats"]["entropy"];
    pet.stats.sleep = doc["stats"]["sleep"];
    pet.stats.stability = doc["stats"]["stability"];
    pet.stats.happiness = doc["stats"]["happiness"];
    pet.stats.health = doc["stats"]["health"];
    
    // Load customization
    pet.custom.name = doc["custom"]["name"].as<String>();
    pet.custom.accessories = doc["custom"]["accessories"];
    pet.custom.colorScheme = doc["custom"]["colorScheme"];
    pet.custom.skinType = doc["custom"]["skinType"];
    
    // Load state
    pet.birthTime = doc["state"]["birthTime"];
    pet.totalInteractions = doc["state"]["totalInteractions"];
    pet.evolutionStage = doc["state"]["evolutionStage"];
    pet.isAlive = doc["state"]["isAlive"];
    pet.lastUpdate = millis();
    
    debugLog("Pet data loaded successfully");
    return true;
}

bool DigitalPetApp::savePetData() {
    // Create directory if it doesn't exist
    createAppDataDir();
    
    File file = SD.open(saveFilePath, FILE_WRITE);
    if (!file) {
        debugLog("Failed to open save file for writing");
        return false;
    }
    
    // Create JSON document
    DynamicJsonDocument doc(1024);
    
    // Save pet stats
    doc["stats"]["mood"] = pet.stats.mood;
    doc["stats"]["hunger"] = pet.stats.hunger;
    doc["stats"]["loneliness"] = pet.stats.loneliness;
    doc["stats"]["entropy"] = pet.stats.entropy;
    doc["stats"]["sleep"] = pet.stats.sleep;
    doc["stats"]["stability"] = pet.stats.stability;
    doc["stats"]["happiness"] = pet.stats.happiness;
    doc["stats"]["health"] = pet.stats.health;
    
    // Save customization
    doc["custom"]["name"] = pet.custom.name;
    doc["custom"]["accessories"] = pet.custom.accessories;
    doc["custom"]["colorScheme"] = pet.custom.colorScheme;
    doc["custom"]["skinType"] = pet.custom.skinType;
    
    // Save state
    doc["state"]["birthTime"] = pet.birthTime;
    doc["state"]["totalInteractions"] = pet.totalInteractions;
    doc["state"]["evolutionStage"] = pet.evolutionStage;
    doc["state"]["isAlive"] = pet.isAlive;
    doc["state"]["lastSave"] = millis();
    
    // Add metadata
    doc["version"] = "1.0";
    doc["saveTime"] = systemCore.getUptimeSeconds();
    
    // Write to file
    serializeJson(doc, file);
    file.close();
    
    debugLog("Pet data saved successfully");
    return true;
}

void DigitalPetApp::createDefaultPet() {
    // Initialize with default values
    pet.stats = {50, 75, 20, 0, 80, 75, 60, 70}; // mood, hunger, loneliness, entropy, sleep, stability, happiness, health
    pet.custom = {ACCESSORY_NONE, 0, "Cyber", 0};
    pet.currentMood = MOOD_CONTENT;
    pet.currentActivity = ACTIVITY_IDLE;
    pet.birthTime = millis();
    pet.lastUpdate = millis();
    pet.totalInteractions = 0;
    pet.evolutionStage = 0;
    pet.isAlive = true;
    
    debugLog("Created default pet: " + pet.custom.name);
}

bool DigitalPetApp::validateSaveData(JsonDocument& doc) {
    // Basic validation of save file structure
    return doc.containsKey("stats") && 
           doc.containsKey("custom") && 
           doc.containsKey("state") &&
           doc["stats"].containsKey("mood") &&
           doc["custom"].containsKey("name");
}

// ========================================
// UTILITY METHODS
// ========================================

unsigned long DigitalPetApp::getPetAge() const {
    return (millis() - pet.birthTime) / 3600000; // Age in hours
}

void DigitalPetApp::setPetName(String name) {
    if (name.length() > 0 && name.length() <= 12) {
        pet.custom.name = name;
        savePetData();
    }
}

void DigitalPetApp::drawCustomizationMenu() {
    displayManager.clearScreen(COLOR_BLACK);
    displayManager.setFont(FONT_MEDIUM);
    displayManager.drawTextCentered(0, 20, SCREEN_WIDTH, "Customize Pet", COLOR_RED_GLOW);
    
    displayManager.setFont(FONT_SMALL);
    displayManager.drawTextCentered(0, 200, SCREEN_WIDTH, "Touch to close", COLOR_LIGHT_GRAY);
    
    // Simple customization options would go here
    displayManager.drawText(20, 60, "Accessories:", COLOR_WHITE);
    displayManager.drawText(30, 80, "[ ] Hat", pet.custom.accessories & ACCESSORY_HAT ? COLOR_GREEN_PHOS : COLOR_LIGHT_GRAY);
    displayManager.drawText(30, 100, "[ ] Glasses", pet.custom.accessories & ACCESSORY_GLASSES ? COLOR_GREEN_PHOS : COLOR_LIGHT_GRAY);
}