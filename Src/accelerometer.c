#include "accelerometer.h"
#include "cmsis_os.h"
#include "main.h"
#include "i2c.h"   // Or SPI, depending on your IMU
#include <stdio.h>
#include "timers.h"
#include "string.h"
#include "shell.h"
#include "task.h"
#include <math.h>

static TimerHandle_t sensor_timer = NULL;
static QueueHandle_t sensor_queue = NULL;



// Simulated read function (replace with real driver)
static void read_accel_xyz(AccelData* data)
{
	    HAL_StatusTypeDef status;
	 	uint8_t raw_data[6];
	//    status = HAL_I2C_Mem_Read(&hi2c2, ADXL345_ADDRESS, ADXL345_DATAX0, I2C_MEMADD_SIZE_8BIT, raw_data, 6, 1000);
	    status = HAL_I2C_Mem_Read(&hi2c2, ADXL345_ADDRESS, ADXL345_DATAX0, I2C_MEMADD_SIZE_8BIT, raw_data, 6, 1000);
	    if (status != HAL_OK) {
	    LOG_ERROR("There is no communication with Accelerometer\r\n");
	    }

	    data->x_raw = (int16_t)((raw_data[1] << 8) | raw_data[0]);
	    data->y_raw = (int16_t)((raw_data[3] << 8) | raw_data[2]);
	    data->z_raw = (int16_t)((raw_data[5] << 8) | raw_data[4]);

	    data->x_g = (float)data->x_raw / 256.0f;
	    data->y_g = (float)data->y_raw / 256.0f;
	    data->z_g = (float)data->z_raw / 256.0f;

	    data->magnitude = sqrtf(data->x_g * data->x_g +
	                                data->y_g * data->y_g +
	                                data->z_g * data->z_g);
	    data->tilt_x = calculate_tilt_angle(data->x_g, data->z_g);
	    data->tilt_y = calculate_tilt_angle(data->y_g, data->z_g);


}

static void accel_timer_callback(TimerHandle_t xTimer){
	AccelData data;
	read_accel_xyz(&data);
	data.timestamp_ms = xTaskGetTickCount();
	BaseType_t result = xQueueOverwrite(sensor_queue, &data);
	if (result != pdTRUE){
		LOG_ERROR("[!] Failed to overwrite queue.\r\n");
	}
}

void Accel_adxl345_Init(void) {

    HAL_StatusTypeDef status;
    uint8_t device_id, config_data;
    status = HAL_I2C_IsDeviceReady(&hi2c2, ADXL345_ADDRESS, 3, 1000);
    if (status != HAL_OK){
    	LOG_ERROR("HAL_I2C is not ready\r\n");
    }
     status = HAL_I2C_Mem_Read(&hi2c2, ADXL345_ADDRESS, ADXL345_DEVID_REG, I2C_MEMADD_SIZE_8BIT, &device_id, 1, 1000);
     if (status != HAL_OK || device_id != ADXL345_DEVICE_ID){
    	 LOG_ERROR("HAL is not ok or device id is wrong \r\n");
     }
    config_data = ADXL345_RANGE_2G | 0x08;
    status = HAL_I2C_Mem_Write(&hi2c2, ADXL345_ADDRESS, ADXL345_DATA_FORMAT, I2C_MEMADD_SIZE_8BIT, &config_data, 1, 1000);
    if (status != HAL_OK){
       	LOG_ERROR("HAL_I2C is not ok\r\n");
       }

      config_data = ADXL345_MEASURE_MODE;
      HAL_I2C_Mem_Write(&hi2c2, ADXL345_ADDRESS, ADXL345_POWER_CTL, I2C_MEMADD_SIZE_8BIT, &config_data, 1, 1000);
      if (status != HAL_OK){
           	LOG_ERROR("HAL_I2C is not ok\r\n");
        }
      vTaskDelay(pdMS_TO_TICKS(50));

    sensor_queue = xQueueCreate(1, sizeof(AccelData));  // only latest
    AccelData dummy = {0};
    xQueueOverwrite(sensor_queue, &dummy);
    sensor_timer = xTimerCreate("AccelTimer", pdMS_TO_TICKS(1000), pdTRUE, NULL, accel_timer_callback);
    xTimerStart(sensor_timer, 0);
}

BaseType_t Accel_GetLatest(AccelData *out_data)
{
	  if (sensor_queue == NULL) {
	        LOG_ERROR("[Accelerometer] Queue not initialized!\r\n");
	        return pdFAIL;
	    }
	  BaseType_t result = xQueuePeek(sensor_queue, out_data, 0);
	  	 if (result != pdPASS) {
	  	        LOG_ERROR("[Accelerometer] Queue is empty!\r\n");
	  	        return pdFAIL;
	  	    }
	  return result;
}
float calculate_tilt_angle(float accel_axis, float accel_z) {
    if (accel_z == 0.0f) {
        return (accel_axis >= 0) ? 90.0f : -90.0f;
    }

    float angle_rad = atanf(accel_axis / accel_z);
    float angle_deg = angle_rad * 180.0f / M_PI;

    if (accel_z < 0) {
        angle_deg = (angle_deg >= 0) ? (angle_deg - 180.0f) : (angle_deg + 180.0f);
    }

    return angle_deg;  // This was missing - function needs to return the calculated angle
}
