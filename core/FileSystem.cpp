#include "FileSystem.h"

// Static instance for singleton pattern
FileSystem* FileSystem::instance = nullptr;

FileSystem::FileSystem() : 
    initialized(false),
    lastError(FS_SUCCESS)
{
    // Initialize status structure
    status.isInitialized = false;
    status.sdCardPresent = false;
    status.totalBytes = 0;
    status.usedBytes = 0;
    status.freeBytes = 0;
    status.totalFiles = 0;
    status.totalDirectories = 0;
    status.lastError = FS_SUCCESS;
    status.lastErrorMessage = "";
    
    // Clear working buffer
    memset(workingBuffer, 0, FILE_BUFFER_SIZE);
}

FileSystem::~FileSystem() {
    if (initialized) {
        Serial.println("[FileSystem] Shutting down filesystem");
        SD.end();
    }
}

FileSystem& FileSystem::getInstance() {
    if (instance == nullptr) {
        instance = new FileSystem();
    }
    return *instance;
}

void FileSystem::destroyInstance() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

// ===========================================
// CORE INITIALIZATION
// ===========================================

bool FileSystem::begin() {
    Serial.println("[FileSystem] Initializing filesystem...");
    
    clearError();
    
    // Initialize SD card
    if (!initializeSD()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Failed to initialize SD card");
        return false;
    }
    
    // Create standard directories
    String standardDirs[] = {APPS_DIR, DATA_DIR, SAMPLES_DIR, SETTINGS_DIR, TEMP_DIR, LOGS_DIR};
    int numDirs = sizeof(standardDirs) / sizeof(standardDirs[0]);
    
    for (int i = 0; i < numDirs; i++) {
        if (!ensureDirExists(standardDirs[i])) {
            Serial.printf("[FileSystem] Warning: Could not create directory %s\n", standardDirs[i].c_str());
        }
    }
    
    // Update filesystem status
    checkSDHealth();
    
    initialized = true;
    status.isInitialized = true;
    
    Serial.println("[FileSystem] Filesystem initialized successfully");
    printStats();
    
    return true;
}

bool FileSystem::initializeSD() {
    Serial.printf("[FileSystem] Initializing SD card on CS pin %d...\n", SD_CS);
    
    // Initialize SPI and SD card
    if (!SD.begin(SD_CS)) {
        Serial.println("[FileSystem] ERROR: SD card initialization failed");
        return false;
    }
    
    // Check if card is present
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[FileSystem] ERROR: No SD card attached");
        return false;
    }
    
    status.sdCardPresent = true;
    
    // Print card information
    Serial.print("[FileSystem] SD card type: ");
    switch (cardType) {
        case CARD_MMC:
            Serial.println("MMC");
            break;
        case CARD_SD:
            Serial.println("SDSC");
            break;
        case CARD_SDHC:
            Serial.println("SDHC");
            break;
        default:
            Serial.println("Unknown");
            break;
    }
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("[FileSystem] SD card size: %llu MB\n", cardSize);
    
    return true;
}

// ===========================================
// ERROR HANDLING AND UTILITIES
// ===========================================

void FileSystem::setError(FileSystemError error, const String& message) {
    lastError = error;
    lastErrorMessage = message;
    status.lastError = error;
    status.lastErrorMessage = message;
    
    Serial.printf("[FileSystem] ERROR: %s (%s)\n", getErrorString(error).c_str(), message.c_str());
}

void FileSystem::clearError() {
    lastError = FS_SUCCESS;
    lastErrorMessage = "";
    status.lastError = FS_SUCCESS;
    status.lastErrorMessage = "";
}

String FileSystem::getErrorString(FileSystemError error) {
    switch (error) {
        case FS_SUCCESS: return "Success";
        case FS_ERROR_SD_NOT_INITIALIZED: return "SD not initialized";
        case FS_ERROR_SD_NOT_PRESENT: return "SD card not present";
        case FS_ERROR_FILE_NOT_FOUND: return "File not found";
        case FS_ERROR_FILE_EXISTS: return "File already exists";
        case FS_ERROR_DIRECTORY_NOT_FOUND: return "Directory not found";
        case FS_ERROR_DIRECTORY_EXISTS: return "Directory already exists";
        case FS_ERROR_PERMISSION_DENIED: return "Permission denied";
        case FS_ERROR_DISK_FULL: return "Disk full";
        case FS_ERROR_INVALID_PATH: return "Invalid path";
        case FS_ERROR_BUFFER_OVERFLOW: return "Buffer overflow";
        case FS_ERROR_MEMORY_ERROR: return "Memory error";
        case FS_ERROR_OPERATION_FAILED: return "Operation failed";
        case FS_ERROR_INVALID_PARAMETER: return "Invalid parameter";
        default: return "Unknown error";
    }
}

String FileSystem::sanitizePath(const String& path) {
    String sanitized = path;
    
    // Ensure path starts with /
    if (!sanitized.startsWith("/")) {
        sanitized = "/" + sanitized;
    }
    
    // Replace backslashes with forward slashes
    sanitized.replace("\\", "/");
    
    // Remove double slashes
    while (sanitized.indexOf("//") >= 0) {
        sanitized.replace("//", "/");
    }
    
    // Remove trailing slash (except for root)
    if (sanitized.length() > 1 && sanitized.endsWith("/")) {
        sanitized = sanitized.substring(0, sanitized.length() - 1);
    }
    
    return sanitized;
}

bool FileSystem::isValidPath(const String& path) {
    if (path.length() == 0 || path.length() > MAX_PATH_LENGTH) {
        return false;
    }
    
    // Check for invalid characters
    String invalidChars = "<>:\"|?*";
    for (unsigned int i = 0; i < invalidChars.length(); i++) {
        if (path.indexOf(invalidChars[i]) >= 0) {
            return false;
        }
    }
    
    return true;
}

bool FileSystem::isValidFilename(const String& filename) {
    if (filename.length() == 0 || filename.length() > MAX_FILENAME_LENGTH) {
        return false;
    }
    
    // Check for invalid characters
    String invalidChars = "<>:\"/\\|?*";
    for (unsigned int i = 0; i < invalidChars.length(); i++) {
        if (filename.indexOf(invalidChars[i]) >= 0) {
            return false;
        }
    }
    
    return true;
}

void FileSystem::logOperation(const String& operation, const String& path, bool success) {
    Serial.printf("[FileSystem] %s: %s - %s\n", 
                  operation.c_str(), 
                  path.c_str(), 
                  success ? "SUCCESS" : "FAILED");
}

// ===========================================
// DIRECTORY MANAGEMENT
// ===========================================

bool FileSystem::ensureDirExists(const String& path) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!isValidPath(sanitizedPath)) {
        setError(FS_ERROR_INVALID_PATH, "Invalid directory path");
        return false;
    }
    
    if (directoryExists(sanitizedPath)) {
        return true; // Directory already exists
    }
    
    return createDirectoryRecursive(sanitizedPath);
}

bool FileSystem::createDirectory(const String& path) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!isValidPath(sanitizedPath)) {
        setError(FS_ERROR_INVALID_PATH, "Invalid directory path");
        return false;
    }
    
    if (directoryExists(sanitizedPath)) {
        setError(FS_ERROR_DIRECTORY_EXISTS, "Directory already exists");
        return false;
    }
    
    bool success = SD.mkdir(sanitizedPath);
    logOperation("CREATE_DIR", sanitizedPath, success);
    
    if (!success) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to create directory");
    }
    
    return success;
}

bool FileSystem::createDirectoryRecursive(const String& path) {
    String sanitizedPath = sanitizePath(path);
    
    // Split path into components
    std::vector<String> pathComponents;
    String currentPath = "";
    int startPos = 1; // Skip leading /
    
    while (startPos < sanitizedPath.length()) {
        int slashPos = sanitizedPath.indexOf('/', startPos);
        if (slashPos == -1) {
            slashPos = sanitizedPath.length();
        }
        
        String component = sanitizedPath.substring(startPos, slashPos);
        if (component.length() > 0) {
            currentPath += "/" + component;
            pathComponents.push_back(currentPath);
        }
        
        startPos = slashPos + 1;
    }
    
    // Create each directory in the path
    for (const String& dir : pathComponents) {
        if (!directoryExists(dir)) {
            if (!SD.mkdir(dir)) {
                setError(FS_ERROR_OPERATION_FAILED, "Failed to create directory: " + dir);
                return false;
            }
            Serial.printf("[FileSystem] Created directory: %s\n", dir.c_str());
        }
    }
    
    return true;
}

bool FileSystem::removeDirectory(const String& path) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!directoryExists(sanitizedPath)) {
        setError(FS_ERROR_DIRECTORY_NOT_FOUND, "Directory not found");
        return false;
    }
    
    bool success = SD.rmdir(sanitizedPath);
    logOperation("REMOVE_DIR", sanitizedPath, success);
    
    if (!success) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to remove directory (may not be empty)");
    }
    
    return success;
}

bool FileSystem::directoryExists(const String& path) {
    if (!isReady()) {
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    File dir = SD.open(sanitizedPath);
    bool exists = dir && dir.isDirectory();
    if (dir) dir.close();
    return exists;
}

// ===========================================
// FILE OPERATIONS - TEXT
// ===========================================

String FileSystem::readFile(const String& path) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return "";
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!fileExists(sanitizedPath)) {
        setError(FS_ERROR_FILE_NOT_FOUND, "File not found");
        return "";
    }
    
    File file = SD.open(sanitizedPath, FILE_READ);
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for reading");
        return "";
    }
    
    String content = "";
    size_t fileSize = file.size();
    
    // Read file in chunks to avoid memory issues
    size_t totalRead = 0;
    while (file.available() && totalRead < fileSize) {
        size_t toRead = min((size_t)FILE_BUFFER_SIZE, fileSize - totalRead);
        size_t bytesRead = file.readBytes(workingBuffer, toRead);
        workingBuffer[bytesRead] = '\0';
        content += String(workingBuffer);
        totalRead += bytesRead;
    }
    
    file.close();
    logOperation("READ_FILE", sanitizedPath, true);
    
    return content;
}

bool FileSystem::writeFile(const String& path, const String& content) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!isValidPath(sanitizedPath)) {
        setError(FS_ERROR_INVALID_PATH, "Invalid file path");
        return false;
    }
    
    // Ensure parent directory exists
    int lastSlash = sanitizedPath.lastIndexOf('/');
    if (lastSlash > 0) {
        String parentDir = sanitizedPath.substring(0, lastSlash);
        if (!ensureDirExists(parentDir)) {
            return false; // Error already set
        }
    }
    
    File file = SD.open(sanitizedPath, FILE_WRITE);
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for writing");
        return false;
    }
    
    // Write content in chunks
    size_t totalWritten = 0;
    size_t contentLength = content.length();
    
    while (totalWritten < contentLength) {
        size_t toWrite = min((size_t)FILE_BUFFER_SIZE, contentLength - totalWritten);
        String chunk = content.substring(totalWritten, totalWritten + toWrite);
        size_t bytesWritten = file.write((const uint8_t*)chunk.c_str(), chunk.length());
        
        if (bytesWritten != chunk.length()) {
            file.close();
            setError(FS_ERROR_OPERATION_FAILED, "Write operation failed");
            return false;
        }
        
        totalWritten += bytesWritten;
    }
    
    file.close();
    logOperation("WRITE_FILE", sanitizedPath, true);
    
    return true;
}

bool FileSystem::appendFile(const String& path, const String& content) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!isValidPath(sanitizedPath)) {
        setError(FS_ERROR_INVALID_PATH, "Invalid file path");
        return false;
    }
    
    // Ensure parent directory exists
    int lastSlash = sanitizedPath.lastIndexOf('/');
    if (lastSlash > 0) {
        String parentDir = sanitizedPath.substring(0, lastSlash);
        if (!ensureDirExists(parentDir)) {
            return false; // Error already set
        }
    }
    
    File file = SD.open(sanitizedPath, FILE_APPEND);
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for appending");
        return false;
    }
    
    size_t bytesWritten = file.write((const uint8_t*)content.c_str(), content.length());
    file.close();
    
    bool success = (bytesWritten == content.length());
    logOperation("APPEND_FILE", sanitizedPath, success);
    
    if (!success) {
        setError(FS_ERROR_OPERATION_FAILED, "Append operation failed");
    }
    
    return success;
}

// ===========================================
// FILE OPERATIONS - BINARY
// ===========================================

size_t FileSystem::readBinaryFile(const String& path, uint8_t* buffer, size_t maxSize) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return 0;
    }
    
    if (buffer == nullptr || maxSize == 0) {
        setError(FS_ERROR_INVALID_PARAMETER, "Invalid buffer or size");
        return 0;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!fileExists(sanitizedPath)) {
        setError(FS_ERROR_FILE_NOT_FOUND, "File not found");
        return 0;
    }
    
    File file = SD.open(sanitizedPath, FILE_READ);
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for reading");
        return 0;
    }
    
    size_t fileSize = file.size();
    size_t toRead = min(maxSize, fileSize);
    size_t bytesRead = file.readBytes((char*)buffer, toRead);
    
    file.close();
    logOperation("READ_BINARY", sanitizedPath, bytesRead > 0);
    
    return bytesRead;
}

bool FileSystem::writeBinaryFile(const String& path, const uint8_t* data, size_t size) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    if (data == nullptr || size == 0) {
        setError(FS_ERROR_INVALID_PARAMETER, "Invalid data or size");
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!isValidPath(sanitizedPath)) {
        setError(FS_ERROR_INVALID_PATH, "Invalid file path");
        return false;
    }
    
    // Ensure parent directory exists
    int lastSlash = sanitizedPath.lastIndexOf('/');
    if (lastSlash > 0) {
        String parentDir = sanitizedPath.substring(0, lastSlash);
        if (!ensureDirExists(parentDir)) {
            return false; // Error already set
        }
    }
    
    File file = SD.open(sanitizedPath, FILE_WRITE);
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for writing");
        return false;
    }
    
    size_t bytesWritten = file.write(data, size);
    file.close();
    
    bool success = (bytesWritten == size);
    logOperation("WRITE_BINARY", sanitizedPath, success);
    
    if (!success) {
        setError(FS_ERROR_OPERATION_FAILED, "Binary write operation failed");
    }
    
    return success;
}

bool FileSystem::appendBinaryFile(const String& path, const uint8_t* data, size_t size) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    if (data == nullptr || size == 0) {
        setError(FS_ERROR_INVALID_PARAMETER, "Invalid data or size");
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!isValidPath(sanitizedPath)) {
        setError(FS_ERROR_INVALID_PATH, "Invalid file path");
        return false;
    }
    
    // Ensure parent directory exists
    int lastSlash = sanitizedPath.lastIndexOf('/');
    if (lastSlash > 0) {
        String parentDir = sanitizedPath.substring(0, lastSlash);
        if (!ensureDirExists(parentDir)) {
            return false; // Error already set
        }
    }
    
    File file = SD.open(sanitizedPath, FILE_APPEND);
    if (!file) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open file for appending");
        return false;
    }
    
    size_t bytesWritten = file.write(data, size);
    file.close();
    
    bool success = (bytesWritten == size);
    logOperation("APPEND_BINARY", sanitizedPath, success);
    
    if (!success) {
        setError(FS_ERROR_OPERATION_FAILED, "Binary append operation failed");
    }
    
    return success;
}

// ===========================================
// FILE MANAGEMENT
// ===========================================

bool FileSystem::deleteFile(const String& path) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    if (!fileExists(sanitizedPath)) {
        setError(FS_ERROR_FILE_NOT_FOUND, "File not found");
        return false;
    }
    
    bool success = SD.remove(sanitizedPath);
    logOperation("DELETE_FILE", sanitizedPath, success);
    
    if (!success) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to delete file");
    }
    
    return success;
}

bool FileSystem::fileExists(const String& path) {
    if (!isReady()) {
        return false;
    }
    
    String sanitizedPath = sanitizePath(path);
    return SD.exists(sanitizedPath);
}

size_t FileSystem::getFileSize(const String& path) {
    if (!isReady() || !fileExists(path)) {
        return 0;
    }
    
    String sanitizedPath = sanitizePath(path);
    File file = SD.open(sanitizedPath, FILE_READ);
    if (!file) {
        return 0;
    }
    
    size_t size = file.size();
    file.close();
    return size;
}

bool FileSystem::renameFile(const String& oldPath, const String& newPath) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    String sanitizedOldPath = sanitizePath(oldPath);
    String sanitizedNewPath = sanitizePath(newPath);
    
    if (!fileExists(sanitizedOldPath)) {
        setError(FS_ERROR_FILE_NOT_FOUND, "Source file not found");
        return false;
    }
    
    if (fileExists(sanitizedNewPath)) {
        setError(FS_ERROR_FILE_EXISTS, "Destination file already exists");
        return false;
    }
    
    bool success = SD.rename(sanitizedOldPath, sanitizedNewPath);
    logOperation("RENAME_FILE", sanitizedOldPath + " -> " + sanitizedNewPath, success);
    
    if (!success) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to rename file");
    }
    
    return success;
}

bool FileSystem::copyFile(const String& sourcePath, const String& destPath) {
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return false;
    }
    
    String sanitizedSource = sanitizePath(sourcePath);
    String sanitizedDest = sanitizePath(destPath);
    
    if (!fileExists(sanitizedSource)) {
        setError(FS_ERROR_FILE_NOT_FOUND, "Source file not found");
        return false;
    }
    
    if (fileExists(sanitizedDest)) {
        setError(FS_ERROR_FILE_EXISTS, "Destination file already exists");
        return false;
    }
    
    File sourceFile = SD.open(sanitizedSource, FILE_READ);
    if (!sourceFile) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open source file");
        return false;
    }
    
    // Ensure destination directory exists
    int lastSlash = sanitizedDest.lastIndexOf('/');
    if (lastSlash > 0) {
        String destDir = sanitizedDest.substring(0, lastSlash);
        if (!ensureDirExists(destDir)) {
            sourceFile.close();
            return false; // Error already set
        }
    }
    
    File destFile = SD.open(sanitizedDest, FILE_WRITE);
    if (!destFile) {
        sourceFile.close();
        setError(FS_ERROR_OPERATION_FAILED, "Failed to create destination file");
        return false;
    }
    
    // Copy file in chunks
    bool success = true;
    while (sourceFile.available() && success) {
        size_t bytesRead = sourceFile.readBytes(workingBuffer, FILE_BUFFER_SIZE);
        size_t bytesWritten = destFile.write((uint8_t*)workingBuffer, bytesRead);
        success = (bytesWritten == bytesRead);
    }
    
    sourceFile.close();
    destFile.close();
    
    logOperation("COPY_FILE", sanitizedSource + " -> " + sanitizedDest, success);
    
    if (!success) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to copy file");
        SD.remove(sanitizedDest); // Clean up partial copy
    }
    
    return success;
}

// ===========================================
// DIRECTORY LISTING
// ===========================================

std::vector<String> FileSystem::listFiles(const String& directory) {
    std::vector<String> files;
    
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return files;
    }
    
    String sanitizedDir = sanitizePath(directory);
    if (!directoryExists(sanitizedDir)) {
        setError(FS_ERROR_DIRECTORY_NOT_FOUND, "Directory not found");
        return files;
    }
    
    File dir = SD.open(sanitizedDir);
    if (!dir || !dir.isDirectory()) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open directory");
        return files;
    }
    
    File file = dir.openNextFile();
    while (file) {
        files.push_back(String(file.name()));
        file = dir.openNextFile();
    }
    
    dir.close();
    logOperation("LIST_FILES", sanitizedDir, true);
    
    return files;
}

std::vector<FileInfo> FileSystem::listFilesDetailed(const String& directory) {
    std::vector<FileInfo> files;
    
    if (!isReady()) {
        setError(FS_ERROR_SD_NOT_INITIALIZED, "Filesystem not ready");
        return files;
    }
    
    String sanitizedDir = sanitizePath(directory);
    if (!directoryExists(sanitizedDir)) {
        setError(FS_ERROR_DIRECTORY_NOT_FOUND, "Directory not found");
        return files;
    }
    
    File dir = SD.open(sanitizedDir);
    if (!dir || !dir.isDirectory()) {
        setError(FS_ERROR_OPERATION_FAILED, "Failed to open directory");
        return files;
    }
    
    File file = dir.openNextFile();
    while (file) {
        FileInfo info;
        info.name = String(file.name());
        info.fullPath = sanitizedDir + "/" + info.name;
        info.size = file.size();
        info.isDirectory = file.isDirectory();
        info.lastModified = file.getLastWrite();
        info.created = file.getCreationTime();
        
        files.push_back(info);
        file = dir.openNextFile();
    }
    
    dir.close();
    logOperation("LIST_FILES_DETAILED", sanitizedDir, true);
    
    return files;
}

std::vector<String> FileSystem::listFilesPattern(const String& directory, const String& pattern) {
    std::vector<String> matchingFiles;
    std::vector<String> allFiles = listFiles(directory);
    
    // Simple pattern matching (supports * wildcard)
    for (const String& filename : allFiles) {
        if (pattern == "*" || pattern == "*.*") {
            matchingFiles.push_back(filename);
        } else if (pattern.startsWith("*.")) {
            // Extension matching
            String extension = pattern.substring(1); // Remove *
            if (filename.endsWith(extension)) {
                matchingFiles.push_back(filename);
            }
        } else if (pattern.endsWith("*")) {
            // Prefix matching
            String prefix = pattern.substring(0, pattern.length() - 1);
            if (filename.startsWith(prefix)) {
                matchingFiles.push_back(filename);
            }
        } else if (filename == pattern) {
            // Exact match
            matchingFiles.push_back(filename);
        }
    }
    
    return matchingFiles;
}

// ===========================================
// UTILITY FUNCTIONS
// ===========================================

uint64_t FileSystem::getFreeSpace() {
    if (!isReady()) {
        return 0;
    }
    
    return SD.cardSize() - SD.usedBytes();
}

uint64_t FileSystem::getTotalSpace() {
    if (!isReady()) {
        return 0;
    }
    
    return SD.cardSize();
}

uint64_t FileSystem::getUsedSpace() {
    if (!isReady()) {
        return 0;
    }
    
    return SD.usedBytes();
}

bool FileSystem::formatSD() {
    Serial.println("[FileSystem] WARNING: Formatting SD card - all data will be lost!");
    
    // This is a destructive operation - typically not implemented
    // on embedded systems for safety reasons
    setError(FS_ERROR_OPERATION_FAILED, "Format operation not supported");
    return false;
}

bool FileSystem::checkSDHealth() {
    if (!status.sdCardPresent) {
        return false;
    }
    
    // Update space information
    status.totalBytes = getTotalSpace();
    status.usedBytes = getUsedSpace();
    status.freeBytes = getFreeSpace();
    
    return true;
}

void FileSystem::printStats() {
    Serial.println();
    Serial.println("=== FileSystem Statistics ===");
    Serial.printf("Initialized: %s\n", status.isInitialized ? "YES" : "NO");
    Serial.printf("SD Card Present: %s\n", status.sdCardPresent ? "YES" : "NO");
    
    if (status.sdCardPresent) {
        Serial.printf("Total Space: %.2f MB\n", status.totalBytes / (1024.0 * 1024.0));
        Serial.printf("Used Space: %.2f MB\n", status.usedBytes / (1024.0 * 1024.0));
        Serial.printf("Free Space: %.2f MB\n", status.freeBytes / (1024.0 * 1024.0));
        Serial.printf("Usage: %.1f%%\n", (status.usedBytes * 100.0) / status.totalBytes);
    }
    
    if (lastError != FS_SUCCESS) {
        Serial.printf("Last Error: %s (%s)\n", getErrorString(lastError).c_str(), lastErrorMessage.c_str());
    }
    
    Serial.println("==============================");
    Serial.println();
}

void FileSystem::printDirectoryTree(const String& rootPath, int maxDepth) {
    Serial.printf("Directory tree for %s:\n", rootPath.c_str());
    printDirectoryTreeRecursive(rootPath, 0, maxDepth);
}

void FileSystem::printDirectoryTreeRecursive(const String& path, int currentDepth, int maxDepth) {
    if (currentDepth >= maxDepth || !directoryExists(path)) {
        return;
    }
    
    std::vector<FileInfo> items = listFilesDetailed(path);
    
    for (const FileInfo& item : items) {
        // Print indentation
        for (int i = 0; i < currentDepth; i++) {
            Serial.print("  ");
        }
        
        if (item.isDirectory) {
            Serial.printf("📁 %s/\n", item.name.c_str());
            printDirectoryTreeRecursive(item.fullPath, currentDepth + 1, maxDepth);
        } else {
            Serial.printf("📄 %s (%d bytes)\n", item.name.c_str(), item.size);
        }
    }
}