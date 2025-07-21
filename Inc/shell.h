#ifndef SHELL_H
#define SHELL_H


#define ANSI_RESET        "\033[0m"
#define ANSI_BOLD         "\033[1m"
#define ANSI_UNDERLINE    "\033[4m"
#define ANSI_BLINK        "\033[5m"
#define ANSI_INVERSE  	  "\033[7m"

/* Foreground colors */
#define ANSI_BLACK		"\033[30m"
#define ANSI_RED		"\033[31m"
#define ANSI_GREEN		"\033[32m"
#define ANSI_YELLOW		"\033[33m"
#define ANSI_BLUE		"\033[34m"
#define ANSI_MAGENTA	"\033[35m"
#define ANSI_CYAN		"\033[36m"
#define ANSI_WHITE		"\033[37m"

/* Background colors */
#define ANSI_BBLACK		"\033[40m"
#define ANSI_BRED		"\033[41m"
#define ANSI_BGREEN		"\033[42m"
#define ANSI_BYELLOW	"\033[43m"
#define ANSI_BBLUE		"\033[44m"
#define ANSI_BMAGENTA	"\033[45m"
#define ANSI_BCYAN		"\033[46m"
#define ANSI_BWHITE		"\033[47m"







#include "FreeRTOS.h"
#include "queue.h"
#include "usart.h"
#include <stdbool.h>

#pragma once

#include "main.h"
#include <stdint.h>
#include <stdio.h>

typedef enum {
    SHELL_LOG_INFO,
    SHELL_LOG_WARN,
    SHELL_LOG_ERROR,
    SHELL_LOG_PLAIN
} ShellLogLevel;


typedef struct {
	size_t FreeHeap;
	size_t MinFreeHeap;
} MemoryData;

extern QueueHandle_t shell_rx_queue;

// Shell
void Shell_Init(void);
void ShellTask(void *arg);
void Shell_Print(const char *text);
void Shell_LogPrintf(ShellLogLevel level, const char *fmt, ...);
void Shell_LogPrintfExtended(ShellLogLevel level, bool forceFlash, const char *fmt, ...);
void Shell_PrintN(const char* str, size_t n);
//Memory Usage
void MemoryUsage(MemoryData* memory);

//LOG pointer
void Flash_Log_Init(void);

// Logging macros
#define LOG_INFO(...)  Shell_LogPrintfExtended(SHELL_LOG_INFO, false, __VA_ARGS__)
#define LOG_WARN(...)  Shell_LogPrintfExtended(SHELL_LOG_WARN, false, __VA_ARGS__)
#define LOG_ERROR(...) Shell_LogPrintfExtended(SHELL_LOG_ERROR, false, __VA_ARGS__)
#define LOG_PLAIN(...) Shell_LogPrintfExtended(SHELL_LOG_PLAIN, false, __VA_ARGS__)
#define LOG_INFO_FLASH(...)  Shell_LogPrintfExtended(SHELL_LOG_INFO, true, __VA_ARGS__)

#define SHELL_INPUT_BUFFER_SIZE 64
#define SHELL_HISTORY_SIZE 10
#define SHELL_QUEUE_LENGTH 64
typedef struct {
    char line[SHELL_INPUT_BUFFER_SIZE];
    uint16_t cursor_pos;
    uint16_t line_length;
    uint8_t escape;
} LineEditorState;


#define LOGIN_USERNAME "admin"
#define LOGIN_PASSWORD "1234"
extern bool flash_log_enabled[];
extern LineEditorState editor;
#endif
