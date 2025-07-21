#include "FreeRTOS.h"
#include "task.h"
#include "app_main.h"
#include "main.h"
#include "shell.h"
#include "sensor.h"
#include "accelerometer.h"
//#include "temperature.h"
//#include "accelerometer.h"
#include "logger.h"

void AppMain(void){
	Shell_Init();
	Logger_Init();



	 //pvTaskCode pcName uxStackDepth pvParameters uxPriority pxCreatedTask
	//xTaskCreate(LedBlinkTask,"LED", 128, NULL, 1, NULL);
	//xTaskCreate(ShellTask, "Shell", 512, NULL, 2, NULL);
	//xTaskCreate(SensorTask, "Sensor", 256, NULL, 1, NULL);
	Sensor_Init(); //Init timer + queue
	Accel_adxl345_Init();

//	vTaskDelay(pdMS_TO_TICKS(5000));
//	Logger_Info("SYS", "Logger initialized!");
//	Logger_Warn("I2C", "Bus busy at startup");
//	Logger_Error("MOTOR", "Overcurrent detected: %d A", 22);


	//xTaskCreate(AccelTask, "Accel", 256, NULL, 1, NULL);
//	xTaskCreate(TemperatureTask, "Temp",256,NULL,1,NULL);
//	xTaskCreate(AccelTask, "Accel", 256, NULL, 1, NULL);
}

void LedBlinkTask(void *argument){
	while(1){
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		vTaskDelay(pdMS_TO_TICKS(500));

	}

}
