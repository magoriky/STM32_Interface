#include "logger_flash.h"  // Uses Flash_WriteLogEntry()
#include "logger.h"
#include <stdarg.h>
void Logger_Log(LogLevel level, const char* tag, const char* fmt, ...) {
    LogEntry_t entry;
    entry.timestamp_ms = xTaskGetTickCount();
    entry.level = level;
    strncpy(entry.tag, tag, sizeof(entry.tag) - 1);
    entry.tag[sizeof(entry.tag) - 1] = '\0';

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.message, sizeof(entry.message), fmt, args);
    va_end(args);

    Flash_WriteLogEntry(&entry);  // Write to Flash
}
