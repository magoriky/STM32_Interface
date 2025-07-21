#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"


#define ADXL345_ADDRESS     0x53 << 1
#define ADXL345_DEVID_REG   0x00
#define ADXL345_POWER_CTL   0x2D
#define ADXL345_DATA_FORMAT 0x31
#define ADXL345_DATAX0      0x32
#define ADXL345_DEVICE_ID   0xE5
#define ADXL345_MEASURE_MODE 0x08
#define ADXL345_RANGE_2G    0x00


typedef struct {
	int x_raw;
	int y_raw;
	int z_raw;
	float x_g;
	float y_g;
	float z_g;
	float magnitude;
	float tilt_x;
	float tilt_y;

	uint32_t timestamp_ms;
} AccelData;


void Accel_adxl345_Init(void);
float calculate_tilt_angle(float accel_axis, float accel_z);
BaseType_t Accel_GetLatest(AccelData *out_data);

#endif
