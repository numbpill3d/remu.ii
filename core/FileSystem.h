#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include <SD.h>
#include <FS.h>
#include <vector>
#include "Config.h"
#include "Config/hardware_pins.h"

// ========================================
// FileSystem - SD card operations for remu.ii
// Provides file and directory management for all applications
// ========================================

// FileSystem error codes
enum FileSystemError {
    FS_SUCCESS = 0,
    FS_ERROR_SD_NOT_INITIALIZED,
    FS_ERROR_SD_NOT_PRESENT,
    FS_ERROR_FILE_NOT_FOUND,
    FS_ERROR_FILE_EXISTS,
    FS_ERROR_DIRECTORY_NOT_FOUND,
    FS_ERROR_DIRECTORY_EXISTS,
    FS_ERROR_PERMISSION_DENIED,
    FS_ERROR_DISK_FULL,
    FS_ERROR_INVALID_PATH,
    FS_ERROR_BUFFER_OVERFLOW,
    FS_ERROR_MEMORY_ERROR,
    FS_ERROR_OPERATION_FAILED,
    FS_ERROR_INVALID_PARAMETER
};

// FileSystem status structure
struct FileSystemStatus {
    bool isInitialized;
    bool sdCardPresent;
    uint64_t totalBytes;
    uint64_t usedBytes;
    uint64_t freeBytes;
    uint32_t totalFiles;
    uint32_t totalDirectories;
    FileSystemError lastError;
    String lastErrorMessage;
};

// File information structure
struct FileInfo {
    String name;
    String fullPath;
    size_t size;
    bool isDirectory;
    time_t lastModified;
    time_t created;
};

class FileSystem {
private:
    // Singleton instance
    static FileSystem* instance;
    
    // Internal state
    bool initialized;
    FileSystemStatus status;
    char workingBuffer[FILE_BUFFER_SIZE];
    
    // Error handling
    FileSystemError lastError;
    String lastErrorMessage;
    
    // Private constructor for singleton
    FileSystem();
    
    // Internal helper methods
    bool initializeSD();
    String sanitizePath(const String& path);
    bool isValidPath(const String& path);
    bool isValidFilename(const String& filename);
    void setError(FileSystemError error, const String& message = "");
    void clearError();
    void logOperation(const String& operation, const String& path, bool success);
    String getErrorString(FileSystemError error);
    bool createDirectoryRecursive(const String& path);

public:
    // Singleton access
    static FileSystem& getInstance();
    static void destroyInstance();
    
    // Destructor
    ~FileSystem();
    
    // ===========================================
    // CORE INITIALIZATION
    // ===========================================
    
    /**
     * Initialize the filesystem
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * Check if filesystem is initialized and ready
     * @return true if ready for operations
     */
    bool isReady() const { return initialized && status.sdCardPresent; }
    
    /**
     * Get current filesystem status
     * @return FileSystemStatus structure with current state
     */
    FileSystemStatus getStatus() const { return status; }
    
    /**
     * Get last error information
     * @return Last error code
     */
    FileSystemError getLastError() const { return lastError; }
    
    /**
     * Get last error message
     * @return Human-readable error description
     */
    String getLastErrorMessage() const { return lastErrorMessage; }
    
    // ===========================================
    // DIRECTORY MANAGEMENT
    // ===========================================
    
    /**
     * Ensure directory exists, create if necessary
     * @param path Directory path to create
     * @return true if directory exists or was created successfully
     */
    bool ensureDirExists(const String& path);
    
    /**
     * Create a directory
     * @param path Directory path to create
     * @return true if directory was created successfully
     */
    bool createDirectory(const String& path);
    
    /**
     * Remove a directory (must be empty)
     * @param path Directory path to remove
     * @return true if directory was removed successfully
     */
    bool removeDirectory(const String& path);
    
    /**
     * Check if directory exists
     * @param path Directory path to check
     * @return true if directory exists
     */
    bool directoryExists(const String& path);
    
    // ===========================================
    // FILE OPERATIONS - TEXT
    // ===========================================
    
    /**
     * Read entire file content as string
     * @param path File path to read
     * @return File content as String (empty if failed)
     */
    String readFile(const String& path);
    
    /**
     * Write content to file (overwrites existing)
     * @param path File path to write
     * @param content Content to write
     * @return true if write successful
     */
    bool writeFile(const String& path, const String& content);
    
    /**
     * Append content to existing file
     * @param path File path to append to
     * @param content Content to append
     * @return true if append successful
     */
    bool appendFile(const String& path, const String& content);
    
    // ===========================================
    // FILE OPERATIONS - BINARY
    // ===========================================
    
    /**
     * Read binary data from file
     * @param path File path to read
     * @param buffer Buffer to store data
     * @param maxSize Maximum bytes to read
     * @return Number of bytes read (0 if failed)
     */
    size_t readBinaryFile(const String& path, uint8_t* buffer, size_t maxSize);
    
    /**
     * Write binary data to file
     * @param path File path to write
     * @param data Binary data to write
     * @param size Number of bytes to write
     * @return true if write successful
     */
    bool writeBinaryFile(const String& path, const uint8_t* data, size_t size);
    
    /**
     * Append binary data to existing file
     * @param path File path to append to
     * @param data Binary data to append
     * @param size Number of bytes to append
     * @return true if append successful
     */
    bool appendBinaryFile(const String& path, const uint8_t* data, size_t size);
    
    // ===========================================
    // FILE MANAGEMENT
    // ===========================================
    
    /**
     * Delete a file
     * @param path File path to delete
     * @return true if file was deleted successfully
     */
    bool deleteFile(const String& path);
    
    /**
     * Check if file exists
     * @param path File path to check
     * @return true if file exists
     */
    bool fileExists(const String& path);
    
    /**
     * Get file size in bytes
     * @param path File path to check
     * @return File size in bytes (0 if file doesn't exist)
     */
    size_t getFileSize(const String& path);
    
    /**
     * Rename or move a file
     * @param oldPath Current file path
     * @param newPath New file path
     * @return true if rename successful
     */
    bool renameFile(const String& oldPath, const String& newPath);
    
    /**
     * Copy a file
     * @param sourcePath Source file path
     * @param destPath Destination file path
     * @return true if copy successful
     */
    bool copyFile(const String& sourcePath, const String& destPath);
    
    // ===========================================
    // DIRECTORY LISTING
    // ===========================================
    
    /**
     * List files in a directory
     * @param directory Directory path to list
     * @return Vector of filenames (empty if failed)
     */
    std::vector<String> listFiles(const String& directory);
    
    /**
     * List files with detailed information
     * @param directory Directory path to list
     * @return Vector of FileInfo structures
     */
    std::vector<FileInfo> listFilesDetailed(const String& directory);
    
    /**
     * List files matching a pattern
     * @param directory Directory path to search
     * @param pattern File pattern (e.g., "*.wav")
     * @return Vector of matching filenames
     */
    std::vector<String> listFilesPattern(const String& directory, const String& pattern);
    
    // ===========================================
    // UTILITY FUNCTIONS
    // ===========================================
    
    /**
     * Get available free space in bytes
     * @return Free space in bytes
     */
    uint64_t getFreeSpace();
    
    /**
     * Get total space in bytes
     * @return Total space in bytes
     */
    uint64_t getTotalSpace();
    
    /**
     * Get used space in bytes
     * @return Used space in bytes
     */
    uint64_t getUsedSpace();
    
    /**
     * Format the SD card (WARNING: Destroys all data)
     * @return true if format successful
     */
    bool formatSD();
    
    /**
     * Check SD card health and update status
     * @return true if SD card is healthy
     */
    bool checkSDHealth();
    
    /**
     * Print filesystem statistics to Serial
     */
    void printStats();
    
    /**
     * Print directory tree to Serial
     * @param rootPath Root directory to start from
     * @param maxDepth Maximum depth to traverse
     */
    void printDirectoryTree(const String& rootPath = "/", int maxDepth = 5);
    
    /**
     * Recursive helper for printing directory tree
     * @param path Current directory path
     * @param currentDepth Current recursion depth
     * @param maxDepth Maximum depth to traverse
     */
    void printDirectoryTreeRecursive(const String& path, int currentDepth, int maxDepth);
};

// Global filesystem instance access
#define filesystem FileSystem::getInstance()

// Convenience macros for common operations
#define FS_ENSURE_DIR(path) filesystem.ensureDirExists(path)
#define FS_READ_FILE(path) filesystem.readFile(path)
#define FS_WRITE_FILE(path, content) filesystem.writeFile(path, content)
#define FS_FILE_EXISTS(path) filesystem.fileExists(path)
#define FS_DELETE_FILE(path) filesystem.deleteFile(path)

#endif // FILESYSTEM_H