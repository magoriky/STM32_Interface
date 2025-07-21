#ifndef SENSOR_H_
#define SENSOR_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
// Latest values (accessed by shell)
#define HDC1080_ADDR     (0x40 << 1)  // 8-bit HAL I2C address
#define HDC1080_REG_CONFIG 0x02
#define HDC1080_REG_TEMP   0x00


typedef struct {
    float temperature;
    float humidity;
    uint32_t timestamp_ms;
} SensorData;

void Sensor_Init(void);
BaseType_t Sensor_GetLatest(SensorData *out_data);

#endif
