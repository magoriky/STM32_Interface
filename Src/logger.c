#include "logger_flash.h"  // Uses Flash_WriteLogEntry()
#include "logger.h"
#include <stdarg.h>





void Logger_Init(void){
	LoggerFlash_Init();
}



void Logger_Log(LogLevel level, const char* tag, const char* fmt, ...) {
    LogEntry_t entry;
    memset(&entry, 0, sizeof(entry));

    entry.timestamp = xTaskGetTickCount();
    entry.level = level;

    // Copy tag safely
    if (tag)
        strncpy(entry.tag, tag, LOGGER_TAG_LEN - 1);

    // Format message
    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.message, LOGGER_MAX_MSG_LEN, fmt, args);
    va_end(args);

    // Store to flash
    LoggerFlash_WriteStruct(&entry);

    // Print to UART (optional)
    const char* levelStr = (level == LOG_LEVEL_INFO) ? "INFO" :
                           (level == LOG_LEVEL_WARN) ? "WARN" : "ERROR";

    printf("[%s] [%lu ms] %s: %s\r\n", levelStr, entry.timestamp, entry.tag, entry.message);
}
