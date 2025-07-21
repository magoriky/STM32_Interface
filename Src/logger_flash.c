#include "logger_flash.h"
#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>
#include "logger.h"
#include <stdbool.h>

#define LOG_FLASH_START_ADDR  ((uint32_t)0x081C0000)
#define LOG_FLASH_END_ADDR    ((uint32_t)0x08200000)

#define LOG_FLASH_PAGE_SIZE   (LOG_FLASH_END_ADDR - LOG_FLASH_START_ADDR)


static uint32_t flash_log_ptr = LOG_FLASH_START_ADDR;
static SemaphoreHandle_t flash_mutex = NULL;
extern UART_HandleTypeDef huart6;
#define SHELL_UART (&huart6)




bool LoggerFlash_WriteStruct(const void* entry) {
    if (!entry) return false;
    const LogEntry_t* log = (const LogEntry_t*)entry;

    xSemaphoreTake(flash_mutex, portMAX_DELAY);

    if (flash_log_ptr + sizeof(LogEntry_t) > LOG_FLASH_END_ADDR) {
        flash_log_ptr = LOG_FLASH_START_ADDR;
    }

    HAL_FLASH_Unlock();

    for (uint32_t i = 0; i < sizeof(LogEntry_t); i += 4) {
        uint32_t word = 0xFFFFFFFF;
        memcpy(&word, ((uint8_t*)log) + i, 4);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_log_ptr, word);
        flash_log_ptr += 4;
    }

    HAL_FLASH_Lock();
    xSemaphoreGive(flash_mutex);
    return true;
}

void Flash_Log_Init(void)
{
    flash_log_ptr = LOG_FLASH_START_ADDR;

    while (flash_log_ptr < LOG_FLASH_END_ADDR) {
        if (*(uint32_t*)flash_log_ptr == 0xFFFFFFFF) {
            return; // found next free slot
        }
        flash_log_ptr += 4;
    }

    // Flash is full — wrap to start (circular logging)
    flash_log_ptr = LOG_FLASH_START_ADDR;
}

void Flash_Log_Read(void)
{
	if (flash_mutex) xSemaphoreTake(flash_mutex, portMAX_DELAY);
    const uint32_t *addr = (uint32_t *)LOG_FLASH_START_ADDR;
    uint32_t max_len = LOG_FLASH_PAGE_SIZE;
    uint32_t i = 0;

    while (i < max_len) {
        uint32_t word = *addr++;

        // Stop if flash is unwritten (erased flash is 0xFFFFFFFF)
        if (word == 0xFFFFFFFF || word == 0x00000000) break;

        for (int j = 0; j < 4; ++j) {
            uint8_t c = (word >> (8 * j)) & 0xFF;
            if (c == 0xFF || c == '\0') continue;
            HAL_UART_Transmit(SHELL_UART, &c, 1, HAL_MAX_DELAY);
        }

        i += 4;
    }

    const char newline[2] = "\r\n";
    HAL_UART_Transmit(SHELL_UART, (uint8_t *)newline, 2, HAL_MAX_DELAY);
    if (flash_mutex) xSemaphoreGive(flash_mutex);
}



void Flash_Log_Write(const char* msg)
{
	if (flash_mutex) xSemaphoreTake(flash_mutex, portMAX_DELAY);
	uint32_t len = strlen(msg);
    // Wrap around if needed
    if (flash_log_ptr + len > LOG_FLASH_END_ADDR) {
        flash_log_ptr = LOG_FLASH_START_ADDR;
    }
    HAL_FLASH_Unlock();
    for (uint32_t i = 0; i < len; i += 4) {
        uint32_t data = 0xFFFFFFFF;
        memcpy(&data, msg + i, (len - i >= 4) ? 4 : (len - i));
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_log_ptr, data);
        flash_log_ptr += 4;
        if (flash_log_ptr >= LOG_FLASH_END_ADDR) {
            flash_log_ptr = LOG_FLASH_START_ADDR; // wrap
        }
    }
    HAL_FLASH_Lock();

    if (flash_mutex) xSemaphoreGive(flash_mutex);
}



void Flash_DebugDumpRaw(void)
{
    const uint32_t *addr = (uint32_t *)LOG_FLASH_START_ADDR;
    LOG_INFO("Dumping raw flash values...\r\n");

    for (int i = 0; i < 64; ++i) {
        LOG_INFO("0x%08lX: 0x%08lX\r\n", (uint32_t)(addr + i), *(addr + i));
    }
}

void Flash_Log_Erase(void)
{
	if (flash_mutex) xSemaphoreTake(flash_mutex, portMAX_DELAY);
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase;
    uint32_t error;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = FLASH_SECTOR_11;
    erase.NbSectors = 1;

    printf("Erasing sector 11...\r\n");



    if (HAL_FLASHEx_Erase(&erase, &error) != HAL_OK) {
    	  printf("Flash erase failed, error code: 0x%08lX\r\n", error);
    } else {
    	  printf("Sector erase successful!\r\n");
    }
    // Important on STM32F7
//    SCB_CleanInvalidateDCache();
//    __ISB();
//    __DSB();
    HAL_FLASH_Lock();
    flash_log_ptr = LOG_FLASH_START_ADDR;
    if (flash_mutex) xSemaphoreGive(flash_mutex);
}

void LoggerFlash_Init(void)
{
    flash_log_ptr = LOG_FLASH_START_ADDR;
    if (!flash_mutex)
        flash_mutex = xSemaphoreCreateMutex();

    while (flash_log_ptr < LOG_FLASH_END_ADDR) {
        if (*(uint32_t*)flash_log_ptr == 0xFFFFFFFF)
            return;
        flash_log_ptr += 4;
    }

    flash_log_ptr = LOG_FLASH_START_ADDR;
}

void LoggerFlash_WriteRaw(const char* msg)
{
    if (!msg || !flash_mutex) return;
    xSemaphoreTake(flash_mutex, portMAX_DELAY);

    uint32_t len = strlen(msg);
    if (flash_log_ptr + len > LOG_FLASH_END_ADDR)
        flash_log_ptr = LOG_FLASH_START_ADDR;

    HAL_FLASH_Unlock();
    for (uint32_t i = 0; i < len; i += 4) {
        uint32_t data = 0xFFFFFFFF;
        memcpy(&data, msg + i, (len - i >= 4) ? 4 : (len - i));
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_log_ptr, data);
        flash_log_ptr += 4;
    }
    HAL_FLASH_Lock();

    xSemaphoreGive(flash_mutex);
}

void LoggerFlash_ReadAll(void)
{
    if (!flash_mutex) return;
    xSemaphoreTake(flash_mutex, portMAX_DELAY);

    const uint32_t* addr = (uint32_t*)LOG_FLASH_START_ADDR;
    uint32_t max_len = LOG_FLASH_END_ADDR - LOG_FLASH_START_ADDR;

    for (uint32_t i = 0; i < max_len; i += 4) {
        uint32_t word = *addr++;
        if (word == 0xFFFFFFFF || word == 0x00000000) break;

        for (int j = 0; j < 4; ++j) {
            uint8_t c = (word >> (8 * j)) & 0xFF;
            if (c == 0xFF || c == '\0') continue;
            HAL_UART_Transmit(&huart6, &c, 1, HAL_MAX_DELAY);
        }
    }

    const char newline[2] = "\r\n";
    HAL_UART_Transmit(&huart6, (uint8_t *)newline, 2, HAL_MAX_DELAY);

    xSemaphoreGive(flash_mutex);
}

void LoggerFlash_Erase(void)
{
    if (!flash_mutex) return;
    xSemaphoreTake(flash_mutex, portMAX_DELAY);

    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase;
    uint32_t error;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = FLASH_SECTOR_11;
    erase.NbSectors = 1;

    if (HAL_FLASHEx_Erase(&erase, &error) != HAL_OK) {
        printf("Flash erase failed! 0x%08lX\r\n", error);
    } else {
        printf("Sector 11 erased.\r\n");
    }

    HAL_FLASH_Lock();
    flash_log_ptr = LOG_FLASH_START_ADDR;

    xSemaphoreGive(flash_mutex);
}

void LoggerFlash_DumpRaw(void)
{
    const uint32_t *addr = (uint32_t *)LOG_FLASH_START_ADDR;
    printf("Raw flash dump:\r\n");

    for (int i = 0; i < 64; ++i) {
        printf("0x%08lX: 0x%08lX\r\n", (uint32_t)(addr + i), *(addr + i));
    }
}
