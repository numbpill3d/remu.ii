#ifndef SYSTEM_ERROR_H
#define SYSTEM_ERROR_H

#include <Arduino.h>
#include "error_codes.h"

// Maximum number of errors to log
#define MAX_ERROR_LOG 10

// Error log entry
struct ErrorLogEntry {
    ErrorCodes code;
    unsigned long timestamp;
    char message[64];
};

// SystemError - Error logging and tracking system
class SystemError {
private:
    ErrorCodes lastError;
    ErrorLogEntry errorLog[MAX_ERROR_LOG];
    uint8_t errorCount;
    uint8_t errorIndex;

public:
    SystemError() : lastError(ERROR_NONE), errorCount(0), errorIndex(0) {
        for (int i = 0; i < MAX_ERROR_LOG; i++) {
            errorLog[i].code = ERROR_NONE;
            errorLog[i].timestamp = 0;
            errorLog[i].message[0] = '\0';
        }
    }

    // Log an error
    void logError(ErrorCodes code, const char* message = nullptr) {
        lastError = code;

        // Store in circular buffer
        errorLog[errorIndex].code = code;
        errorLog[errorIndex].timestamp = millis();

        if (message) {
            strncpy(errorLog[errorIndex].message, message, 63);
            errorLog[errorIndex].message[63] = '\0';
        } else {
            strncpy(errorLog[errorIndex].message, getErrorDescription(code), 63);
            errorLog[errorIndex].message[63] = '\0';
        }

        errorIndex = (errorIndex + 1) % MAX_ERROR_LOG;
        if (errorCount < MAX_ERROR_LOG) {
            errorCount++;
        }

        // Log to serial
        Serial.printf("[ERROR] Code %d: %s\n", code, errorLog[errorIndex].message);
    }

    // Get the last error code
    ErrorCodes getLastError() const {
        return lastError;
    }

    // Clear the last error
    void clearError() {
        lastError = ERROR_NONE;
    }

    // Get error count
    uint8_t getErrorCount() const {
        return errorCount;
    }

    // Get error log entry
    const ErrorLogEntry* getErrorLog(uint8_t index) const {
        if (index >= errorCount) {
            return nullptr;
        }
        return &errorLog[index];
    }

    // Print error log
    void printErrorLog() const {
        Serial.println("=== Error Log ===");
        for (uint8_t i = 0; i < errorCount; i++) {
            Serial.printf("[%lu] Code %d: %s\n",
                errorLog[i].timestamp,
                errorLog[i].code,
                errorLog[i].message);
        }
    }
};

#endif // SYSTEM_ERROR_H
