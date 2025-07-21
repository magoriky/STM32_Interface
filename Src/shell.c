// File: shell.c
#include "usart.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "shell_commands.h"
#include "shell.h"
#include "semphr.h"
#include "stm32f7xx_hal.h"
#include "task.h"
#include "shell_extensions.h"
#include "logger_flash.h"



/*
This is the start and end of SECTOR 11 for NUCLEO-F767ZI
If you do not understand what I mean, please read documentation, since it controls
 the allowed memory space
*/
#define LOG_FLASH_START_ADDR  ((uint32_t)0x081C0000) // adjust for your chip
//#define LOG_FLASH_END_ADDR  ((uint32_t)0x081C0052)// Testing
#define LOG_FLASH_END_ADDR  ((uint32_t)0x08200000)
/*---------------------------------------------------*/
#define LOG_FLASH_PAGE_SIZE   ((uint32_t)0x20000)    // 128 KB page size example

extern UART_HandleTypeDef huart6;
#define SHELL_UART (&huart6)

QueueHandle_t shell_rx_queue;

static char login_input[32] = {0};
static uint8_t login_stage = 0; // 0 = username, 1 = password
static uint8_t is_logged_in = 0;
static SemaphoreHandle_t uart_mutex = NULL;
static SemaphoreHandle_t flash_mutex = NULL;


typedef struct {
    char history[SHELL_HISTORY_SIZE][SHELL_INPUT_BUFFER_SIZE];
    uint8_t current;
    uint8_t count;
    uint8_t index;
} ShellHistory;


static const char *log_prefix[] = {
    [SHELL_LOG_INFO]  = ANSI_GREEN "[INFO]" ANSI_RESET,
    [SHELL_LOG_WARN]  = ANSI_YELLOW "[WARN]" ANSI_RESET,
    [SHELL_LOG_ERROR] = ANSI_RED "[ERROR]" ANSI_RESET,
    [SHELL_LOG_PLAIN] = ""
};

bool flash_log_enabled[] = {
    [SHELL_LOG_INFO]  = false,
    [SHELL_LOG_WARN]  = true,
    [SHELL_LOG_ERROR] = true,
    [SHELL_LOG_PLAIN] = false
};



LineEditorState editor = {0};
static ShellHistory history = {0};
static bool paste_mode = false;


volatile static uint8_t rx_char; // shared between ISR and task

uint32_t flash_log_ptr = LOG_FLASH_START_ADDR;



void MemoryUsage(MemoryData* memory)
{

	memory->FreeHeap = xPortGetFreeHeapSize();
	memory->MinFreeHeap = xPortGetMinimumEverFreeHeapSize();

}


void Shell_LogPrintfExtended(ShellLogLevel level, bool forceFlash, const char *fmt, ...)
{
    char buf[160];
    int len = 0;

    if (level != SHELL_LOG_PLAIN) {
        uint32_t tick = xTaskGetTickCount();
        len = snprintf(buf, sizeof(buf), "%s[%lu ms] ", log_prefix[level], tick);
    }

    va_list args;
    va_start(args, fmt);
    len += vsnprintf(buf + len, sizeof(buf) - len, fmt, args);
    va_end(args);

    if (len > sizeof(buf) - 1)
        len = sizeof(buf) - 1;

    if (uart_mutex) xSemaphoreTake(uart_mutex, portMAX_DELAY);
    HAL_UART_Transmit(SHELL_UART, (uint8_t*)buf, len, HAL_MAX_DELAY);
    if (uart_mutex) xSemaphoreGive(uart_mutex);

    // 🔥 Flash logging
    if (forceFlash || flash_log_enabled[level]) {
        Flash_Log_Write(buf);  // implement this for actual flash writing
    }
}


void Shell_LogPrintf(ShellLogLevel level, const char *fmt, ...) {
	void Shell_LogPrintf(ShellLogLevel level, const char *fmt, ...)
	{
	    char buf[160];
	    int len = 0;

	    if (level != SHELL_LOG_PLAIN) {
	        uint32_t tick = xTaskGetTickCount();
	        len = snprintf(buf, sizeof(buf), "%s[%lu ms] ", log_prefix[level], tick);
	    }

	    va_list args;
	    va_start(args, fmt);
	    len += vsnprintf(buf + len, sizeof(buf) - len, fmt, args);
	    va_end(args);

	    if (len > sizeof(buf) - 1)
	        len = sizeof(buf) - 1;

	    if (uart_mutex) xSemaphoreTake(uart_mutex, portMAX_DELAY);
	    HAL_UART_Transmit(SHELL_UART, (uint8_t*)buf, len, HAL_MAX_DELAY);
	    if (uart_mutex) xSemaphoreGive(uart_mutex);

	    // ✅ Save only if the log level is enabled
	    if (flash_log_enabled[level]) {
	        Flash_Log_Write(buf);
	    }
	}
}

void Shell_Print(const char *text) {
    if (uart_mutex) xSemaphoreTake(uart_mutex, portMAX_DELAY);
    HAL_UART_Transmit(SHELL_UART, (uint8_t*)text, strlen(text), HAL_MAX_DELAY);
    if (uart_mutex) xSemaphoreGive(uart_mutex);
}


void Shell_PrintN(const char* str, size_t n) {
    if (uart_mutex) xSemaphoreTake(uart_mutex, portMAX_DELAY);
    HAL_UART_Transmit(SHELL_UART, (uint8_t*)str, n, HAL_MAX_DELAY);
    if (uart_mutex) xSemaphoreGive(uart_mutex);
}

void LineEditor_Init(void) {
    		Shell_ClearScreen();
    		Shell_PrintBanner();
    		Shell_PrintWelcome();
    		Shell_Print("\n\n");


	    Shell_Print(ANSI_CYAN "Username: " ANSI_RESET);
	    //HAL_UART_Transmit(SHELL_UART, (uint8_t*)"Username: ", 10, HAL_MAX_DELAY);


	    // Start UART reception
	    HAL_UART_Receive_IT(SHELL_UART, &rx_char, 1);
	    //HAL_UART_Transmit(SHELL_UART, (uint8_t*)"\r\n> ", 4, HAL_MAX_DELAY);
}

void Shell_ProcessLine(void) {
    char *argv[8];  // Up to 8 arguments
    int argc = 0;

    char *token = strtok(editor.line, " ");
    while (token != NULL && argc < 8) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) return;

    for (int i = 0; i < shell_command_count; ++i) {
        if (strcmp(argv[0], shell_commands[i].cmd) == 0) {
            shell_commands[i].handler(argc, argv);  // ✅ Pass args
            return;
        }
    }

    Shell_Print("Unknown command\r\n");
}

void LineEditor_ProcessChar(uint8_t ch) {
    static uint8_t esc_seq = 0;

    if (!is_logged_in) {
        static uint16_t login_index = 0;

        if (ch == '\r' || ch == '\n') {
            login_input[login_index] = '\0';

            if (login_stage == 0) {
                if (strcmp(login_input, LOGIN_USERNAME) == 0) {
                    login_stage = 1;
                    Shell_Print("\r\nPassword: ");
                } else {
                    Shell_Print("\r\nInvalid username\r\nUsername: ");
                }
            } else if (login_stage == 1) {
                if (strcmp(login_input, LOGIN_PASSWORD) == 0) {
                    is_logged_in = 1;
                    Shell_Print(ANSI_GREEN "\r\nWelcome!\r\n" ANSI_RESET);
                    Shell_PrintCursor();
                } else {
                    Shell_Print("\r\nWrong password\r\nUsername: ");
                    login_stage = 0;
                }
            }

            login_index = 0;
            memset(login_input, 0, sizeof(login_input));
            return;
        } else if (ch == 127 || ch == '\b') {
            if (login_index > 0) {
                login_index--;
                Shell_Print("\b \b");
            }
        } else if (ch >= 32 && ch <= 126 && login_index < sizeof(login_input) - 1) {
            login_input[login_index++] = ch;
            Shell_Print(login_stage == 0 ? (char[]){ch, '\0'} : "*");
        }
        return;
    }

    if (editor.escape == 1) {
        if (ch == '[') {
            esc_seq = 1;
            return;
        }
        editor.escape = 0;
    }

    if (esc_seq == 1) {
        if (ch == 'D' && editor.cursor_pos > 0) { // ← Left
            editor.cursor_pos--;
        } else if (ch == 'C' && editor.cursor_pos < editor.line_length) { // → Right
            editor.cursor_pos++;
        } else if (ch == 'A' && history.count > 0) { // ↑ Up
            if (history.index == -1)
                history.index = history.current == 0 ? history.count - 1 : history.current - 1;
            else if (history.index > 0)
                history.index--;

            memset(editor.line, 0, sizeof(editor.line));
            strncpy(editor.line, history.history[history.index], SHELL_INPUT_BUFFER_SIZE);
            editor.line_length = editor.cursor_pos = strlen(editor.line);
        } else if (ch == 'B' && history.count > 0) { // ↓ Down
            if (history.index >= 0 && history.index < history.count - 1)
                history.index++;
            else
                history.index = -1;

            memset(editor.line, 0, sizeof(editor.line));
            if (history.index != -1)
                strncpy(editor.line, history.history[history.index], SHELL_INPUT_BUFFER_SIZE);
            editor.line_length = editor.cursor_pos = strlen(editor.line);
        }
        Shell_RedrawLineSmooth();
        esc_seq = 0;
        return;
    }

    if (ch == 0x1B) {  // ESC
        editor.escape = 1;
        return;
    }

    if (ch == '\r' || ch == '\n') {
        Shell_Print("\r\n");
        editor.line[editor.line_length] = '\0';

        if (editor.line_length > 0) {
            strncpy(history.history[history.current], editor.line, SHELL_INPUT_BUFFER_SIZE);
            history.current = (history.current + 1) % SHELL_HISTORY_SIZE;
            if (history.count < SHELL_HISTORY_SIZE) history.count++;
        }

        history.index = history.current;
        Shell_ProcessLine();
        memset(&editor, 0, sizeof(editor));
        Shell_PrintCursor();
    } else if (ch == '\t') {
        Shell_HandleTabCompletion();
        return;
    } else if (ch == 127 || ch == '\b') {
        if (editor.cursor_pos > 0) {
            // Shift characters left from cursor position - 1
            for (uint16_t i = editor.cursor_pos - 1; i < editor.line_length - 1; ++i) {
                editor.line[i] = editor.line[i + 1];
            }
            editor.line_length--;
            editor.line[editor.line_length] = '\0';  // null-terminate
            editor.cursor_pos--;

            Shell_RedrawLineSmooth();  // ✅ redraw everything with cursor fix
        }
    } else if (ch >= 32 && ch <= 126) {
        if (editor.line_length < SHELL_INPUT_BUFFER_SIZE - 1) {
            for (uint16_t i = editor.line_length; i > editor.cursor_pos; i--) {
                editor.line[i] = editor.line[i - 1];
            }
            editor.line[editor.cursor_pos++] = ch;
            editor.line_length++;
            Shell_RedrawLineSmooth();
        }
    }
}


void ShellTask(void *arg) {
    uint8_t ch;
    LineEditor_Init();
    while (1) {
        if (xQueueReceive(shell_rx_queue, &ch, portMAX_DELAY) == pdPASS) {
            LineEditor_ProcessChar(ch);
        }
    }
}

void Shell_Init(void) {
	uart_mutex = xSemaphoreCreateMutex();
	if (uart_mutex == NULL) {
	    while (1); // fatal error
	}
	flash_mutex = xSemaphoreCreateMutex();
	if (flash_mutex == NULL) {
	    // handle error (fatal or retry)
	}
    shell_rx_queue = xQueueCreate(SHELL_QUEUE_LENGTH, sizeof(uint8_t));
    if (shell_rx_queue == NULL) {
        while (1);
    }
    xTaskCreate(ShellTask, "Shell", 1024, NULL, 2, NULL);
    HAL_UART_Receive_IT(SHELL_UART, (uint8_t*)&rx_char, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(shell_rx_queue, &rx_char, &xHigherPriorityTaskWoken);
        HAL_UART_Receive_IT(SHELL_UART, (uint8_t*)&rx_char, 1);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
