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
    showStats(false),
    showPetSelection(false),
    firstBoot(true),
    lastEntropyUpdate(0),
    lastMoodUpdate(0),
    lastAnimation(0),
    currentAnimFrame(0),
    currentRoomTheme(THEME_LOVING),
    activeTouchZone(-1),
    currentAnimation(nullptr),
    animationFrameCount(0),
    animationLoop(true)
{
    // Set app metadata
    metadata.name = "DigitalPet";
    metadata.version = "2.0";
    metadata.author = "remu.ii";
    metadata.description = "Entropy-driven cyberpet companion with memory";
    metadata.category = CATEGORY_GAMES;
    metadata.maxMemory = 20000; // 20KB for memory buffer
    metadata.requiresSD = true;
    metadata.requiresBLE = false;
    metadata.requiresWiFi = false;
    
    // Initialize save file paths
    saveFilePath = "/apps/DigitalPet/pet_data.json";
    petTypeFilePath = "/apps/DigitalPet/pet_type.txt";
    
    // Set colors
    backgroundColor = COLOR_BLACK;
    foregroundColor = COLOR_GREEN_PHOS;
    
    showBackButton = true;
    showStatusBar = true;
    
    // Initialize pet state with safe defaults
    pet.mood = CALM;
    pet.traits.clear();
    pet.corruptionLevel = 0.0f;
    pet.isAwake = true;
    pet.isObservingUser = false;
    pet.memory.clear();
    pet.personalitySeed = random(0xFFFFFFFF);
    pet.archetype = ORACLE; // Default, will be overridden
    pet.name = "???";
    pet.birthTime = millis();
    pet.lastUpdate = millis();
    pet.totalInteractions = 0;
    pet.isAlive = true;
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
    
    // Check if this is first boot (no pet type saved)
    if (!loadPetType()) {
        debugLog("First boot - showing pet selection screen");
        firstBoot = true;
        showPetSelection = true;
    } else {
        firstBoot = false;
        showPetSelection = false;
        
        // Load existing pet data or create new pet with saved archetype
        if (!loadPetData()) {
            debugLog("Creating new pet with saved archetype...");
            createDefaultPet(pet.archetype);
        }
    }
    
    // Setup touch zones for interaction
    setupTouchZones();
    
    // Start with idle animation
    setAnimation(idleAnimation, 3, true);
    
    // Initialize timing
    lastEntropyUpdate = millis();
    lastMoodUpdate = millis();
    lastAnimation = millis();
    
    setState(APP_RUNNING);
    debugLog("DigitalPet initialized successfully");
    
    return true;
}

void DigitalPetApp::update() {
    if (currentState != APP_RUNNING) return;
    
    unsigned long currentTime = millis();
    
    // Skip pet updates if in selection screen
    if (showPetSelection) {
        frameCount++;
        return;
    }
    
    // Update entropy influence periodically
    if (currentTime - lastEntropyUpdate >= ENTROPY_SAMPLE_INTERVAL) {
        applyEntropyInfluence();
        updateCorruption();
        lastEntropyUpdate = currentTime;
    }
    
    // Update mood and archetype behavior periodically
    if (currentTime - lastMoodUpdate >= 5000) { // Every 5 seconds
        updateMood();
        updateArchetypeBehavior();
        updateMemoryBuffer();
        lastMoodUpdate = currentTime;
    }
    
    // Update animation
    if (currentTime - lastAnimation >= 100) { // Animation update rate
        updateAnimation();
        lastAnimation = currentTime;
    }
    
    // Process corruption effects
    processCorruptionEffects();
    
    // Update pet state
    pet.lastUpdate = currentTime;
    
    // Save periodically
    if (systemCore.getUptimeSeconds() % 300 == 0) { // Every 5 minutes
        savePetData();
    }
    
    frameCount++;
}

void DigitalPetApp::render() {
    if (currentState != APP_RUNNING) return;
    
    // Clear screen
    displayManager.clearScreen(backgroundColor);
    
    if (showPetSelection) {
        showPetSelectionScreen();
    } else if (showStats) {
        drawStatsDisplay();
    } else {
        // Main pet view
        drawReactiveRoom();
        drawPet();
        drawMoodIndicator();
        drawInteractionButtons();
        
        // Draw corruption effects if present
        if (isCorrupted()) {
            drawCorruptionOverlay();
        }
        
        // Draw glitch effects for highly corrupted pets
        if (isHighlyCorrupted()) {
            drawGlitchEffects();
        }
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
    
    // Handle pet selection screen
    if (showPetSelection) {
        return handlePetSelection(touch);
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
        interactWithPet();
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
// MEMORY SYSTEM IMPLEMENTATION
// ========================================

void DigitalPetApp::recordAction(String action, float intensity) {
    PetMemory memory;
    memory.action = action;
    memory.timestamp = millis();
    memory.intensity = intensity;
    
    // Add to memory buffer
    pet.memory.push_back(memory);
    
    // Limit memory buffer size
    while (pet.memory.size() > MAX_MEMORY_ENTRIES) {
        pet.memory.pop_front();
    }
    
    debugLog("Recorded memory: " + action + " (intensity: " + String(intensity) + ")");
}

void DigitalPetApp::updateMemoryBuffer() {
    // Clean up old memories (older than 1 hour)
    clearOldMemories();
    
    // Check for neglect (no interactions in last 10 minutes)
    if (pet.memory.empty() || (millis() - pet.memory.back().timestamp) > 600000) {
        recordAction("neglect", 0.5f);
    }
}

bool DigitalPetApp::recentNeglect(unsigned long timeWindowMs) {
    unsigned long currentTime = millis();
    
    for (auto it = pet.memory.rbegin(); it != pet.memory.rend(); ++it) {
        if (currentTime - it->timestamp > timeWindowMs) break;
        if (it->action == "neglect") return true;
    }
    return false;
}

bool DigitalPetApp::wasRecentlyPunished(unsigned long timeWindowMs) {
    unsigned long currentTime = millis();
    
    for (auto it = pet.memory.rbegin(); it != pet.memory.rend(); ++it) {
        if (currentTime - it->timestamp > timeWindowMs) break;
        if (it->action == "punish" || it->action == "aggressive_touch") return true;
    }
    return false;
}

float DigitalPetApp::getMemoryInfluence(String actionType, unsigned long timeWindowMs) {
    unsigned long currentTime = millis();
    float totalInfluence = 0.0f;
    int count = 0;
    
    for (auto it = pet.memory.rbegin(); it != pet.memory.rend(); ++it) {
        if (currentTime - it->timestamp > timeWindowMs) break;
        if (it->action == actionType) {
            totalInfluence += it->intensity;
            count++;
        }
    }
    
    return count > 0 ? totalInfluence / count : 0.0f;
}

void DigitalPetApp::clearOldMemories() {
    unsigned long currentTime = millis();
    unsigned long oneHour = 3600000; // 1 hour in ms
    
    while (!pet.memory.empty() && (currentTime - pet.memory.front().timestamp) > oneHour) {
        pet.memory.pop_front();
    }
}

// ========================================
// MOOD & CORRUPTION SYSTEM
// ========================================

void DigitalPetApp::updateMood() {
    float entropy = getCurrentEntropy();
    
    // Base mood calculation on entropy levels
    if (entropy > 0.9f) {
        pet.mood = GLITCHED;
        // Add paranoid trait if high entropy persists
        if (std::find(pet.traits.begin(), pet.traits.end(), PARANOID) == pet.traits.end()) {
            pet.traits.push_back(PARANOID);
        }
    } else if (entropy > 0.6f) {
        pet.mood = RESTLESS;
    } else if (recentNeglect()) {
        pet.mood = OBSESSED;
        // Add needy trait after neglect
        if (std::find(pet.traits.begin(), pet.traits.end(), NEEDY) == pet.traits.end()) {
            pet.traits.push_back(NEEDY);
        }
    } else if (wasRecentlyPunished()) {
        pet.mood = RESTLESS;
        // Add aggressive trait after punishment
        if (std::find(pet.traits.begin(), pet.traits.end(), AGGRESSIVE) == pet.traits.end()) {
            pet.traits.push_back(AGGRESSIVE);
        }
    } else {
        pet.mood = CALM;
        // Add loving trait for good treatment
        float loveInfluence = getMemoryInfluence("pet", 600000) + getMemoryInfluence("feed", 600000);
        if (loveInfluence > 2.0f && std::find(pet.traits.begin(), pet.traits.end(), LOVING) == pet.traits.end()) {
            pet.traits.push_back(LOVING);
        }
    }
    
    // Limit traits to prevent overflow
    if (pet.traits.size() > 3) {
        pet.traits.erase(pet.traits.begin());
    }
}

void DigitalPetApp::updateCorruption() {
    float entropy = getCurrentEntropy();
    
    // Increase corruption based on high entropy
    if (entropy > 0.8f) {
        pet.corruptionLevel = min(1.0f, pet.corruptionLevel + 0.01f);
    } else if (entropy < 0.2f) {
        // Slowly heal corruption with low entropy
        pet.corruptionLevel = max(0.0f, pet.corruptionLevel - 0.005f);
    }
    
    // Memory influences corruption
    if (wasRecentlyPunished()) {
        pet.corruptionLevel = min(1.0f, pet.corruptionLevel + 0.02f);
    }
    
    if (getMemoryInfluence("pet", 300000) > 1.0f) {
        pet.corruptionLevel = max(0.0f, pet.corruptionLevel - 0.01f);
    }
}

float DigitalPetApp::getCurrentEntropy() {
    // Get system entropy and normalize to 0.0-1.0
    uint32_t entropyPool = systemCore.getEntropyPool();
    uint8_t entropyByte = (entropyPool >> 24) & 0xFF;
    return entropyByte / 255.0f;
}

void DigitalPetApp::processCorruptionEffects() {
    if (pet.corruptionLevel > CORRUPTION_THRESHOLD_HIGH) {
        // High corruption: pet may lie about its state
        if (random(100) < 10) { // 10% chance per update
            // Corrupt some memory entries
            if (!pet.memory.empty()) {
                int randomIndex = random(pet.memory.size());
                auto it = pet.memory.begin();
                std::advance(it, randomIndex);
                it->action = "corrupted_memory";
                it->intensity = random(100) / 100.0f;
            }
        }
    }
}

// ========================================
// ARCHETYPE SYSTEM IMPLEMENTATION
// ========================================

void DigitalPetApp::initializeArchetype(PetArchetype archetype) {
    pet.archetype = archetype;
    pet.traits.clear();
    
    switch (archetype) {
        case ORACLE:
            pet.name = "Oracle";
            pet.traits.push_back(LOVING);
            pet.personalitySeed = random(0xFFFFFFFF);
            break;
            
        case PARASITE:
            pet.name = "Parasite";
            pet.traits.push_back(NEEDY);
            pet.personalitySeed = random(0xFFFFFFFF);
            break;
            
        case MIRROR:
            pet.name = "Mirror";
            pet.traits.push_back(PARANOID);
            pet.personalitySeed = random(0xFFFFFFFF);
            break;
    }
    
    debugLog("Initialized " + pet.name + " archetype");
}

void DigitalPetApp::updateArchetypeBehavior() {
    switch (pet.archetype) {
        case ORACLE:
            processOracleBehavior();
            break;
        case PARASITE:
            processParasiteBehavior();
            break;
        case MIRROR:
            processMirrorBehavior();
            break;
    }
}

void DigitalPetApp::processOracleBehavior() {
    float entropy = getCurrentEntropy();
    
    // Oracle reads entropy patterns and offers "visions"
    if (entropy > 0.8f && pet.mood == CALM) {
        pet.mood = OBSESSED; // Focused on entropy reading
        recordAction("oracle_vision", entropy);
    }
    
    // Oracle becomes more stable with high entropy (opposite of other pets)
    if (entropy > 0.9f) {
        pet.corruptionLevel = max(0.0f, pet.corruptionLevel - 0.01f);
    }
}

void DigitalPetApp::processParasiteBehavior() {
    // Parasite drains system resources and reacts poorly to neglect
    if (recentNeglect()) {
        pet.corruptionLevel = min(1.0f, pet.corruptionLevel + 0.05f);
        pet.mood = GLITCHED;
    }
    
    // Parasite gets more needy over time
    if (getMemoryInfluence("pet", 300000) < 0.5f) {
        if (std::find(pet.traits.begin(), pet.traits.end(), NEEDY) == pet.traits.end()) {
            pet.traits.push_back(NEEDY);
        }
    }
}

void DigitalPetApp::processMirrorBehavior() {
    // Mirror mimics user behavior patterns
    float touchFreq = getMemoryInfluence("pet", 600000);
    float feedFreq = getMemoryInfluence("feed", 600000);
    
    if (touchFreq > feedFreq) {
        pet.mood = RESTLESS; // Mirrors user's touch-heavy behavior
    } else if (feedFreq > touchFreq) {
        pet.mood = OBSESSED; // Mirrors user's feeding focus
    }
    
    // Mirror says strange things (recorded as corrupted thoughts)
    if (random(100) < 5) { // 5% chance
        recordAction("mirror_echo", random(100) / 100.0f);
    }
}

String DigitalPetApp::getArchetypeResponse(String interaction) {
    switch (pet.archetype) {
        case ORACLE:
            if (interaction == "pet") return "The entropy whispers...";
            if (interaction == "feed") return "Energy flows through me";
            return "I see patterns...";
            
        case PARASITE:
            if (interaction == "pet") return "More... I need more...";
            if (interaction == "feed") return "Not enough! Never enough!";
            return "Why do you abandon me?";
            
        case MIRROR:
            if (interaction == "pet") return "You touch, I echo...";
            if (interaction == "feed") return "I taste what you give...";
            return "Your patterns... I learn...";
            
        default:
            return "...";
    }
}

void DigitalPetApp::applyEntropyInfluence() {
    // This is now handled in updateMood() and updateCorruption()
    // Keep for backward compatibility but redirect
    updateMood();
    updateCorruption();
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
// PET SELECTION SYSTEM
// ========================================

void DigitalPetApp::showPetSelectionScreen() {
    displayManager.clearScreen(COLOR_BLACK);
    
    // Title
    displayManager.setFont(FONT_LARGE);
    displayManager.drawTextCentered(0, 20, SCREEN_WIDTH, "Choose Your Pet", COLOR_RED_GLOW);
    
    // Draw three archetype options
    int16_t optionY = 60;
    int16_t optionSpacing = 50;
    
    // Oracle option
    drawArchetypeOption(20, optionY, ORACLE, false);
    optionY += optionSpacing;
    
    // Parasite option
    drawArchetypeOption(20, optionY, PARASITE, false);
    optionY += optionSpacing;
    
    // Mirror option
    drawArchetypeOption(20, optionY, MIRROR, false);
    
    // Instructions
    displayManager.setFont(FONT_SMALL);
    displayManager.drawTextCentered(0, 210, SCREEN_WIDTH, "Touch to select", COLOR_LIGHT_GRAY);
}

void DigitalPetApp::drawArchetypeOption(int16_t x, int16_t y, PetArchetype archetype, bool selected) {
    uint16_t color = selected ? COLOR_GREEN_PHOS : COLOR_WHITE;
    uint16_t bgColor = selected ? COLOR_DARK_GRAY : COLOR_BLACK;
    
    // Draw background if selected
    if (selected) {
        displayManager.drawRetroRect(x-5, y-5, 280, 40, bgColor, true);
    }
    
    displayManager.setFont(FONT_MEDIUM);
    
    switch (archetype) {
        case ORACLE:
            displayManager.drawText(x, y, "ORACLE", COLOR_PURPLE_GLOW);
            displayManager.setFont(FONT_SMALL);
            displayManager.drawText(x, y + 15, "Wise entropy reader. Offers cryptic visions.", color);
            break;
            
        case PARASITE:
            displayManager.drawText(x, y, "PARASITE", COLOR_RED_GLOW);
            displayManager.setFont(FONT_SMALL);
            displayManager.drawText(x, y + 15, "Clingy companion. Drains battery, hates neglect.", color);
            break;
            
        case MIRROR:
            displayManager.drawText(x, y, "MIRROR", COLOR_BLUE_CYBER);
            displayManager.setFont(FONT_SMALL);
            displayManager.drawText(x, y + 15, "Mimics your patterns. Says unsettling things.", color);
            break;
    }
}

bool DigitalPetApp::handlePetSelection(TouchPoint touch) {
    // Check which archetype was touched
    if (touch.y >= 60 && touch.y <= 100) {
        selectArchetype(ORACLE);
        return true;
    } else if (touch.y >= 110 && touch.y <= 150) {
        selectArchetype(PARASITE);
        return true;
    } else if (touch.y >= 160 && touch.y <= 200) {
        selectArchetype(MIRROR);
        return true;
    }
    
    return false;
}

void DigitalPetApp::selectArchetype(PetArchetype archetype) {
    debugLog("Selected archetype: " + String(archetype));
    
    // Initialize the pet with chosen archetype
    initializeArchetype(archetype);
    
    // Save the pet type
    savePetType();
    
    // Create default pet data
    createDefaultPet(archetype);
    
    // Hide selection screen
    showPetSelection = false;
    firstBoot = false;
    
    debugLog("Pet selection complete: " + pet.name);
}

bool DigitalPetApp::loadPetType() {
    if (!SD.exists(petTypeFilePath)) {
        return false;
    }
    
    File file = SD.open(petTypeFilePath, FILE_READ);
    if (!file) {
        return false;
    }
    
    String typeStr = file.readString();
    file.close();
    
    typeStr.trim();
    
    if (typeStr == "ORACLE") {
        pet.archetype = ORACLE;
    } else if (typeStr == "PARASITE") {
        pet.archetype = PARASITE;
    } else if (typeStr == "MIRROR") {
        pet.archetype = MIRROR;
    } else {
        return false;
    }
    
    initializeArchetype(pet.archetype);
    return true;
}

void DigitalPetApp::savePetType() {
    createAppDataDir();
    
    File file = SD.open(petTypeFilePath, FILE_WRITE);
    if (!file) {
        debugLog("Failed to save pet type");
        return;
    }
    
    switch (pet.archetype) {
        case ORACLE:
            file.print("ORACLE");
            break;
        case PARASITE:
            file.print("PARASITE");
            break;
        case MIRROR:
            file.print("MIRROR");
            break;
    }
    
    file.close();
    debugLog("Pet type saved");
}

// ========================================
// INTERACTION METHODS (Updated)
// ========================================

void DigitalPetApp::interactWithPet() {
    if (!pet.isAlive) return;
    
    recordAction("pet", 1.0f);
    pet.totalInteractions++;
    
    // Get archetype-specific response
    String response = getArchetypeResponse("pet");
    debugLog("Pet response: " + response);
    
    // Show happy animation
    setAnimation(happyAnimation, 3, false);
}

void DigitalPetApp::feedPet() {
    if (!pet.isAlive) return;
    
    recordAction("feed", 1.0f);
    pet.totalInteractions++;
    
    // Get archetype-specific response
    String response = getArchetypeResponse("feed");
    debugLog("Pet response: " + response);
}

void DigitalPetApp::punishPet() {
    if (!pet.isAlive) return;
    
    recordAction("punish", 1.5f);
    pet.totalInteractions++;
    
    // Increase corruption from punishment
    pet.corruptionLevel = min(1.0f, pet.corruptionLevel + 0.1f);
    
    debugLog("Pet punished - corruption increased");
}

void DigitalPetApp::observePet() {
    if (!pet.isAlive) return;
    
    pet.isObservingUser = true;
    recordAction("observe", 0.5f);
    
    debugLog("Observing pet...");
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