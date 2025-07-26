#include "FileSystem.h"
#include <SD.h>

// Static instance pointer for singleton
FileSystem* FileSystem::instance = nullptr;

FileSystem::FileSystem() : 
    initialized(false),
    lastError(FS_SUCCESS)
{
    status = {false, false, 0, 0, 0, 0, 0, FS_SUCCESS, ""};
    memset(workingBuffer, 0, FILE_BUFFER_SIZE);
}

FileSystem::~FileSystem() {
    // Cleanup if needed
}

FileSystem& FileSystem::getInstance() {
    if (!instance) {
        instance = new FileSystem();
    }
    return *instance;
}

void FileSystem::destroyInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

bool FileSystem::begin() {
    Serial.println("[FileSystem] Initializing SD card...");
    
    if (!initializeSD()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "SD card initialization failed");
        return false;
    }
    
    // Create standard directories
    ensureDirExists(APPS_DIR);
    ensureDirExists(DATA_DIR);
    ensureDirExists(SAMPLES_DIR);
    ensureDirExists(SETTINGS_DIR);
    ensureDirExists(TEMP_DIR);
    ensureDirExists(LOGS_DIR);
    
    initialized = true;
    status.isInitialized = true;
    status.sdCardPresent = true;
    
    // Update status information
    checkSDHealth();
    
    Serial.println("[FileSystem] SD card initialized successfully");
    printStats();
    
    return true;
}

bool FileSystem::initializeSD() {
    if (!SD.begin(SD_CS)) {
        Serial.println("[FileSystem] ERROR: SD card mount failed");
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[FileSystem] ERROR: No SD card attached");
        return false;
    }
    
    Serial.print("[FileSystem] SD Card Type: ");
    switch (cardType) {
        case CARD_MMC: Serial.println("MMC"); break;
        case CARD_SD: Serial.println("SDSC"); break;
        case CARD_SDHC: Serial.println("SDHC"); break;
        default: Serial.println("UNKNOWN"); break;
    }
    
    return true;
}

bool FileSystem::ensureDirExists(const String& path) {
    if (directoryExists(path)) {
        return true;
    }
    return createDirectoryRecursive(path);
}

bool FileSystem::createDirectory(const String& path) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "SD card not ready");
        return false;
    }
    
    String cleanPath = sanitizePath(path);
    if (!isValidPath(cleanPath)) {
        setError(FS_ERROR_INVALID_PATH, "Invalid directory path");
        return false;
    }
    
    if (SD.mkdir(cleanPath)) {
        clearError();
        logOperation("CREATE_DIR", cleanPath, true);
        return true;
    } else {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to create directory");
        logOperation("CREATE_DIR", cleanPath, false);
        return false;
    }
}

bool FileSystem::createDirectoryRecursive(const String& path) {
    String cleanPath = sanitizePath(path);
    
    // Split path into components
    int lastSlash = 0;
    for (int i = 1; i < cleanPath.length(); i++) {
        if (cleanPath[i] == '/') {
            String subPath = cleanPath.substring(0, i);
            if (!directoryExists(subPath)) {
                if (!SD.mkdir(subPath)) {
                    setError(FS_ERROR_OPERATION_FAILED, "Failed to create directory: " + subPath);
                    return false;
                }
            }
        }
    }
    
    // Create final directory
    if (!directoryExists(cleanPath)) {
        return SD.mkdir(cleanPath);
    }
    
    return true;
}

bool FileSystem::directoryExists(const String& path) {
    if (!isReady()) return false;
    
    String cleanPath = sanitizePath(path);
    File dir = SD.open(cleanPath);
    
    if (dir && dir.isDirectory()) {
        dir.close();
        return true;
    }
    
    if (dir) dir.close();
    return false;
}

String FileSystem::readFile(const String& path) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "SD card not ready");
        return "";
    }
    
    String cleanPath = sanitizePath(path);
    File file = SD.open(cleanPath, FILE_READ);
    
    if (!file) {
        setError(FS_ERROR_FILE_NOT_FOUND, "File not found: " + cleanPath);
        return "";
    }
    
    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    
    file.close();
    clearError();
    logOperation("READ_FILE", cleanPath, true);
    
    return content;
}

bool FileSystem::writeFile(const String& path, const String& content) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "SD card not ready");
        return false;
    }
    
    String cleanPath = sanitizePath(path);
    
    // Ensure parent directory exists
    int lastSlash = cleanPath.lastIndexOf('/');
    if (lastSlash > 0) {
        String parentDir = cleanPath.substring(0, lastSlash);
        ensureDirExists(parentDir);
    }
    
    File file = SD.open(cleanPath, FILE_WRITE);
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for writing");
        return false;
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    if (bytesWritten == content.length()) {
        clearError();
        logOperation("WRITE_FILE", cleanPath, true);
        return true;
    } else {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to write complete file");
        logOperation("WRITE_FILE", cleanPath, false);
        return false;
    }
}

bool FileSystem::appendFile(const String& path, const String& content) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "SD card not ready");
        return false;
    }
    
    String cleanPath = sanitizePath(path);
    File file = SD.open(cleanPath, FILE_APPEND);
    
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for appending");
        return false;
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    if (bytesWritten == content.length()) {
        clearError();
        logOperation("APPEND_FILE", cleanPath, true);
        return true;
    } else {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to append to file");
        logOperation("APPEND_FILE", cleanPath, false);
        return false;
    }
}

bool FileSystem::deleteFile(const String& path) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "SD card not ready");
        return false;
    }
    
    String cleanPath = sanitizePath(path);
    
    if (SD.remove(cleanPath)) {
        clearError();
        logOperation("DELETE_FILE", cleanPath, true);
        return true;
    } else {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to delete file");
        logOperation("DELETE_FILE", cleanPath, false);
        return false;
    }
}

bool FileSystem::fileExists(const String& path) {
    if (!isReady()) return false;
    
    String cleanPath = sanitizePath(path);
    return SD.exists(cleanPath);
}

size_t FileSystem::getFileSize(const String& path) {
    if (!isReady()) return 0;
    
    String cleanPath = sanitizePath(path);
    File file = SD.open(cleanPath, FILE_READ);
    
    if (!file) return 0;
    
    size_t size = file.size();
    file.close();
    
    return size;
}

std::vector<String> FileSystem::listFiles(const String& directory) {
    std::vector<String> files;
    
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "SD card not ready");
        return files;
    }
    
    String cleanPath = sanitizePath(directory);
    File dir = SD.open(cleanPath);
    
    if (!dir || !dir.isDirectory()) {
        setError(FS_ERROR_DIRECTORY_NOT_FOUND, "Directory not found");
        if (dir) dir.close();
        return files;
    }
    
    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            files.push_back(String(file.name()));
        }
        file.close();
        file = dir.openNextFile();
    }
    
    dir.close();
    clearError();
    
    return files;
}

uint64_t FileSystem::getFreeSpace() {
    if (!isReady()) return 0;
    return SD.totalBytes() - SD.usedBytes();
}

uint64_t FileSystem::getTotalSpace() {
    if (!isReady()) return 0;
    return SD.totalBytes();
}

uint64_t FileSystem::getUsedSpace() {
    if (!isReady()) return 0;
    return SD.usedBytes();
}

bool FileSystem::checkSDHealth() {
    if (!initialized) return false;
    
    status.totalBytes = SD.totalBytes();
    status.usedBytes = SD.usedBytes();
    status.freeBytes = status.totalBytes - status.usedBytes;
    
    // Simple health check - ensure we can read/write
    String testFile = "/health_check.tmp";
    String testContent = "health_check";
    
    if (writeFile(testFile, testContent)) {
        String readContent = readFile(testFile);
        deleteFile(testFile);
        
        if (readContent == testContent) {
            status.sdCardPresent = true;
            return true;
        }
    }
    
    status.sdCardPresent = false;
    return false;
}

void FileSystem::printStats() {
    Serial.println("[FileSystem] SD Card Statistics:");
    Serial.printf("  Total Space: %.2f MB\n", status.totalBytes / (1024.0 * 1024.0));
    Serial.printf("  Used Space:  %.2f MB\n", status.usedBytes / (1024.0 * 1024.0));
    Serial.printf("  Free Space:  %.2f MB\n", status.freeBytes / (1024.0 * 1024.0));
    Serial.printf("  Usage:       %.1f%%\n", (status.usedBytes * 100.0) / status.totalBytes);
}

String FileSystem::sanitizePath(const String& path) {
    String clean = path;
    
    // Ensure path starts with /
    if (!clean.startsWith("/")) {
        clean = "/" + clean;
    }
    
    // Remove double slashes
    while (clean.indexOf("//") >= 0) {
        clean.replace("//", "/");
    }
    
    // Remove trailing slash (except for root)
    if (clean.length() > 1 && clean.endsWith("/")) {
        clean = clean.substring(0, clean.length() - 1);
    }
    
    return clean;
}

bool FileSystem::isValidPath(const String& path) {
    if (path.length() == 0 || path.length() > MAX_PATH_LENGTH) {
        return false;
    }
    
    // Check for invalid characters
    String invalidChars = "<>:\"|?*";
    for (int i = 0; i < invalidChars.length(); i++) {
        if (path.indexOf(invalidChars[i]) >= 0) {
            return false;
        }
    }
    
    return true;
}

void FileSystem::setError(FileSystemError error, const String& message) {
    lastError = error;
    lastErrorMessage = message;
    status.lastError = error;
    status.lastErrorMessage = message;
    
    if (error != FS_SUCCESS) {
        Serial.printf("[FileSystem] ERROR: %s\n", message.c_str());
    }
}

void FileSystem::clearError() {
    lastError = FS_SUCCESS;
    lastErrorMessage = "";
    status.lastError = FS_SUCCESS;
    status.lastErrorMessage = "";
}

void FileSystem::logOperation(const String& operation, const String& path, bool success) {
    // Simple logging to serial for now
    Serial.printf("[FileSystem] %s: %s - %s\n", 
                 operation.c_str(), path.c_str(), success ? "SUCCESS" : "FAILED");
}

// Binary file operations
size_t FileSystem::readBinaryFile(const String& path, uint8_t* buffer, size_t maxSize) {
    if (!isReady() || !buffer) {
        setError(FS_ERROR_INVALID_PARAMETER, "Invalid parameters");
        return 0;
    }
    
    String cleanPath = sanitizePath(path);
    File file = SD.open(cleanPath, FILE_READ);
    
    if (!file) {
        setError(FS_ERROR_FILE_NOT_FOUND, "File not found");
        return 0;
    }
    
    size_t bytesRead = file.readBytes((char*)buffer, maxSize);
    file.close();
    
    clearError();
    return bytesRead;
}

bool FileSystem::writeBinaryFile(const String& path, const uint8_t* data, size_t size) {
    if (!isReady() || !data) {
        setError(FS_ERROR_INVALID_PARAMETER, "Invalid parameters");
        return false;
    }
    
    String cleanPath = sanitizePath(path);
    File file = SD.open(cleanPath, FILE_WRITE);
    
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for writing");
        return false;
    }
    
    size_t bytesWritten = file.write(data, size);
    file.close();
    
    if (bytesWritten == size) {
        clearError();
        return true;
    } else {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to write binary data");
        return false;
    }
}