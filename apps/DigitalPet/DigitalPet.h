#ifndef DIGITAL_PET_H
#define DIGITAL_PET_H

#include "../../core/AppManager/BaseApp.h"
#include "../../core/SystemCore/SystemCore.h"
#include <ArduinoJson.h>
#include <SD.h>

// ========================================
// DigitalPet - Cyberpet companion app for remu.ii
// Features mood tracking, entropy influence, customization, and persistence
// ========================================

// Pet stats configuration
#define PET_STAT_MAX 100
#define PET_STAT_MIN 0

// Decay rates (per minute)
#define HUNGER_DECAY_RATE 2
#define LONELINESS_DECAY_RATE 1
#define SLEEP_DECAY_RATE 3
#define STABILITY_DECAY_RATE 1

// Interaction effects
#define FEED_AMOUNT 25
#define PET_AMOUNT 15
#define PLAY_AMOUNT 20
#define REST_AMOUNT 30

// Pet states
enum PetMood {
    MOOD_HAPPY,
    MOOD_CONTENT,
    MOOD_NEUTRAL,
    MOOD_SAD,
    MOOD_ANGRY,
    MOOD_SLEEPING,
    MOOD_SICK,
    MOOD_CHAOTIC  // High entropy state
};

enum PetActivity {
    ACTIVITY_IDLE,
    ACTIVITY_EATING,
    ACTIVITY_PLAYING,
    ACTIVITY_SLEEPING,
    ACTIVITY_INTERACTING
};

// Pet customization
enum PetAccessory {
    ACCESSORY_NONE = 0,
    ACCESSORY_HAT = 1,
    ACCESSORY_GLASSES = 2,
    ACCESSORY_BOWTIE = 4,
    ACCESSORY_ANTENNAE = 8
};

// Pet data structure
struct PetStats {
    uint8_t mood;           // 0-100
    uint8_t hunger;         // 0-100 (100 = full)
    uint8_t loneliness;     // 0-100 (0 = not lonely)
    uint8_t entropy;        // 0-100 (affected by system entropy)
    uint8_t sleep;          // 0-100 (100 = well rested)
    uint8_t stability;      // 0-100 (affected by entropy)
    uint8_t happiness;      // Calculated from other stats
    uint8_t health;         // Overall health indicator
};

struct PetCustomization {
    uint16_t accessories;   // Bitfield of PetAccessory
    uint8_t colorScheme;    // Color theme index
    String name;            // Pet name
    uint8_t skinType;       // Pet skin/sprite variant
};

struct PetState {
    PetStats stats;
    PetCustomization custom;
    PetMood currentMood;
    PetActivity currentActivity;
    unsigned long lastUpdate;
    unsigned long birthTime;
    unsigned long totalInteractions;
    uint8_t evolutionStage;  // Pet growth stage
    bool isAlive;
};

// Touch zones for interaction
struct TouchZone {
    int16_t x, y, w, h;
    String action;
    bool enabled;
};

// Animation frame data
struct AnimationFrame {
    const uint8_t* spriteData;
    uint16_t duration;  // ms to display this frame
};

class DigitalPetApp : public BaseApp {
private:
    // Pet state
    PetState pet;
    String saveFilePath;
    
    // UI state
    uint8_t selectedAction;
    bool showStats;
    bool showCustomization;
    unsigned long lastStatsUpdate;
    unsigned long lastAnimation;
    uint8_t currentAnimFrame;
    
    // Touch zones
    TouchZone touchZones[6];
    int8_t activeTouchZone;
    
    // Animation data
    AnimationFrame* currentAnimation;
    uint8_t animationFrameCount;
    bool animationLoop;
    
    // Private methods - Pet Logic
    void updatePetStats();
    void calculateMood();
    void calculateHappiness();
    void applyEntropyInfluence();
    void handleStatDecay();
    void checkPetHealth();
    
    // Private methods - Interactions
    void feedPet();
    void petPet();
    void playWithPet();
    void putPetToSleep();
    void showPetStats();
    void customizePet();
    
    // Private methods - Rendering
    void drawPet();
    void drawPetSprite(int16_t x, int16_t y);
    void drawMoodIndicator();
    void drawStatsDisplay();
    void drawCustomizationMenu();
    void drawInteractionButtons();
    void drawBackground();
    void drawPetRoom();
    
    // Private methods - Animation
    void updateAnimation();
    void setAnimation(AnimationFrame* frames, uint8_t frameCount, bool loop = true);
    void drawAnimatedSprite(int16_t x, int16_t y);
    
    // Private methods - Touch handling
    void setupTouchZones();
    int8_t getTouchedZone(TouchPoint touch);
    void handleZoneTouch(int8_t zone);
    
    // Private methods - File I/O
    bool loadPetData();
    bool savePetData();
    void createDefaultPet();
    bool validateSaveData(JsonDocument& doc);
    
    // Private methods - ASCII Art
    void drawASCIIMood(int16_t x, int16_t y, PetMood mood);
    void drawASCIIPet(int16_t x, int16_t y, PetMood mood);
    
    // Animation definitions
    static AnimationFrame idleAnimation[];
    static AnimationFrame happyAnimation[];
    static AnimationFrame eatingAnimation[];
    static AnimationFrame sleepingAnimation[];
    static AnimationFrame playingAnimation[];
    
    // Sprite data (16x16 bitmaps)
    static const uint8_t PET_SPRITE_IDLE[32];
    static const uint8_t PET_SPRITE_HAPPY[32];
    static const uint8_t PET_SPRITE_SAD[32];
    static const uint8_t PET_SPRITE_SLEEPING[32];
    static const uint8_t PET_SPRITE_EATING[32];
    static const uint8_t PET_SPRITE_SICK[32];
    
    // Accessory sprites
    static const uint8_t ACCESSORY_HAT_SPRITE[32];
    static const uint8_t ACCESSORY_GLASSES_SPRITE[32];
    static const uint8_t ACCESSORY_BOWTIE_SPRITE[32];

public:
    DigitalPetApp();
    virtual ~DigitalPetApp();
    
    // Mandatory BaseApp methods
    bool initialize() override;
    void update() override;
    void render() override;
    bool handleTouch(TouchPoint touch) override;
    void cleanup() override;
    String getName() const override { return "DigitalPet"; }
    const uint8_t* getIcon() const override;
    
    // Optional BaseApp methods
    void onPause() override;
    void onResume() override;
    bool saveState() override;
    bool loadState() override;
    bool handleMessage(AppMessage message, void* data = nullptr) override;
    
    // Settings menu support
    uint8_t getSettingsCount() const override { return 4; }
    String getSettingName(uint8_t index) const override;
    void handleSetting(uint8_t index) override;
    
    // Pet-specific methods
    PetStats getCurrentStats() const { return pet.stats; }
    PetMood getCurrentMood() const { return pet.currentMood; }
    String getPetName() const { return pet.custom.name; }
    void setPetName(String name);
    bool isPetAlive() const { return pet.isAlive; }
    unsigned long getPetAge() const;
    
    // Debug methods
    void debugResetPet();
    void debugSetStat(String statName, uint8_t value);
    void debugTriggerMood(PetMood mood);
};

#endif // DIGITAL_PET_H