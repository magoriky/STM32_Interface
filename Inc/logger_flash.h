#ifndef LOGGER_FLASH_H_
#define LOGGER_FLASH_H_

#include <stdint.h>
#include <stdbool.h>

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
bool LoggerFlash_WriteStruct(const void* entry);

#endif
