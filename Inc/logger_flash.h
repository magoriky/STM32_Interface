#ifndef LOGGER_FLASH_H
#define LOGGER_FLASH_H

#include <stdint.h>


void Flash_log_Init(void);
void Flash_Log_Read(void);
void Flash_Log_Write(const char* msg);
void Flash_DebugDumpRaw(void);
void Flash_Log_Erase(void);

void LoggerFlash_Init(void);
void LoggerFlash_WriteRaw(const char* msg);
void LoggerFlash_ReadAll(void);
void LoggerFlash_Erase(void);
void LoggerFlash_DumpRaw(void);

#endif
