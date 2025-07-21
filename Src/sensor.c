#include "sensor.h"
#include "cmsis_os.h"
#include "main.h"
#include "i2c.h"  // Or your specific sensor driver
#include <stdio.h>
#include "usart.h"
#include "string.h"
#include "timers.h"
#include "shell.h"



static TimerHandle_t sensor_timer = NULL;
static QueueHandle_t sensor_queue = NULL;


// Simulated read from a sensor (replace with real read)
static void read_temp_and_humidity(float *temperature, float *humidity)
{
    // TODO: Replace with real I2C sensor code (like SHT31, DHT20, etc.)
	if (HAL_I2C_DeInit(&hi2c2) != HAL_OK) // Fully resets the I2C peripheral.
			{
			   LOG_ERROR("I2C DeInit communication is broken \r\n");
			}
		if (HAL_I2C_Init(&hi2c2) != HAL_OK) // Initializes and enables the I2C peripheral based on the configuration in hi2c2.Init
		  {
			LOG_ERROR("I2C communication is broken \r\n");
		  }
		HAL_StatusTypeDef status;
	    uint8_t reg_addr = 0x00;
	    //uint8_t ADDRESS = 0x40;
		status = HAL_I2C_Master_Transmit(&hi2c2, HDC1080_ADDR, &reg_addr, 1, 1000);
		if (status != HAL_OK){
			LOG_ERROR("Master could not send data \r\n");
		}
	    HAL_Delay(15); // Wait for conversion
	    uint8_t buffer[4];
	    status = HAL_I2C_Master_Receive(&hi2c2, HDC1080_ADDR, buffer, 4, 1000);
	    if (status != HAL_OK){
	    	LOG_ERROR("Master could not receive data \r\n");

	    }
	    uint16_t raw_temp = (buffer[0] << 8) | buffer[1];
	    uint16_t raw_hum  = (buffer[2] << 8) | buffer[3];
	    *temperature = (((float)raw_temp / 65536.0) * 165.0) - 40.0;
	    *humidity    = ((float)raw_hum / 65536.0) * 100.0;
}

static void sensor_timer_callback(TimerHandle_t xTimer)
{
    SensorData data;
    read_temp_and_humidity(&data.temperature, &data.humidity);
    data.timestamp_ms = xTaskGetTickCount();
    BaseType_t result = xQueueOverwrite(sensor_queue, &data);  // only store latest
    if (result != pdTRUE) {
    	LOG_ERROR("[!] Failed to overwrite queue.\r\n");
    }
}

void Sensor_Init(void)
{
    sensor_queue = xQueueCreate(1, sizeof(SensorData));  // only latest
    SensorData dummy = {0};
    xQueueOverwrite(sensor_queue, &dummy);
    sensor_timer = xTimerCreate("SensorTimer", pdMS_TO_TICKS(500), pdTRUE, NULL, sensor_timer_callback);
    xTimerStart(sensor_timer, 0);
}

BaseType_t Sensor_GetLatest(SensorData *out_data)
{
	  if (sensor_queue == NULL) {
	        LOG_ERROR("[Sensor] Queue not initialized!\r\n");
	        return pdFAIL;
	    }
	BaseType_t result = xQueuePeek(sensor_queue, out_data, 0);
	 if (result != pdPASS) {
	        LOG_ERROR("[Sensor] Queue is empty!\r\n");
	        return pdFAIL;
	    }
    return result;
}



