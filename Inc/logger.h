#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>


typedef enum {
    LOG_LEVEL_INFO = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
} LogLevel;

//typedef struct {
//    uint32_t timestamp_ms;
//    LogLevel level;
//    char tag[8];       // Optional category (e.g. "TEMP")
//    char message[96];  // Formatted string
//} LogEntry_t;
#define LOGGER_TAG_LEN     8
#define LOGGER_MAX_MSG_LEN 96

typedef struct {
	uint32_t timestamp;
	LogLevel level;
	char tag[LOGGER_TAG_LEN];
	char message[LOGGER_MAX_MSG_LEN];
} LogEntry_t;


void Logger_Init(void);
void Logger_Log(LogLevel level, const char* tag, const char* fmt, ...);


#define Logger_Info(tag, fmt, ...)  Logger_Log(LOG_LEVEL_INFO, tag, fmt, ##__VA_ARGS__)
#define Logger_Warn(tag, fmt, ...)  Logger_Log(LOG_LEVEL_WARN, tag, fmt, ##__VA_ARGS__)
#define Logger_Error(tag, fmt, ...) Logger_Log(LOG_LEVEL_ERROR, tag, fmt, ##__VA_ARGS__)

#endif
