#include "shell_commands.h"
#include "shell.h"
#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "sensor.h"
#include "led_control.h"
#include "accelerometer.h"

extern UART_HandleTypeDef huart6;


void ShellCmd_get_accel(int argc, char** argv)
{
	AccelData data;
	if (Accel_GetLatest(&data) == pdTRUE){
		LOG_INFO( "Accel x_raw: %d y_raw: %d z_raw: %d \r\n",
				data.x_raw,
				data.y_raw,
				data.z_raw);

		LOG_INFO( "Accel x_g: %.2f y_g: %.2f z_g: %.2f \r\n",
						data.x_g,
						data.y_g,
						data.z_g);
		LOG_INFO( "Accel magnitud: %.2f tilt_x: %.2f tilt_y: %.2f \r\n",
								data.magnitude,
								data.tilt_x,
								data.tilt_y);


	} else {
		LOG_ERROR("No sensor data available yet\r\n");
	}

}

void ShellCmd_get_memory_data(int argc, char** argv)
{
	MemoryData memory;
	MemoryUsage(&memory);
	LOG_INFO("Free heap: %u bytes\r\n"
			 "Min ever heap: %u bytes\r\n",
						memory.FreeHeap,
						memory.MinFreeHeap);
}

void ShellCmd_whoami(int argc, char** argv)
{
    LOG_INFO("You are logged in as: %s\r\n", LOGIN_USERNAME);
}

void ShellCmd_get_sensor(int argc, char** argv)
{
	SensorData data;
//	Sensor_GetLatest(&data);
	if (Sensor_GetLatest(&data) == pdTRUE) {

		LOG_INFO("Temp: %.2f°C, Hum: %.2f%%, Time: %lu ms\r\n",
	               data.temperature,
	               data.humidity,
	               data.timestamp_ms);


	} else {
		 LOG_ERROR("No sensor data available yet\r\n");
	}
}

void ShellCmd_led_on(int argc, char **argv)
{
	if (argc < 2) {
	   LOG_ERROR("Usage: led_on <1|2|3>\r\n");
	   return;
	}
	int led = atoi(argv[1]);
	if (led < 1 || led > 3) {
	    LOG_ERROR("Invalid LED number\r\n");
	        return;
	}

	LED_Control(led, LED_ON);
    LOG_INFO("LED %d ON\r\n", led);
}

void ShellCmd_led_off(int argc, char **argv)
{
	if (argc < 2) {
		   LOG_ERROR("Usage: led_off <1|2|3>\r\n");
		   return;
		}
		int led = atoi(argv[1]);
		if (led < 1 || led > 3) {
		    LOG_ERROR("Invalid LED number\r\n");
		        return;
		}

		LED_Control(led, LED_OFF);
	    LOG_INFO("LED %d OFF\r\n", led);
}

void ShellCmd_hello(int argc, char** argv)
{
    LOG_INFO("Hello from RTOS shell\r\n");
}

void ShellCmd_help(int argc, char** argv){
	LOG_INFO("Available commands:\r\n");
	char buf[64];
	for (int i=0; i< shell_command_count; ++i){
		snprintf(buf, sizeof(buf), " -%s | %s\r\n", shell_commands[i].cmd, shell_commands[i].desc);
		Shell_Print(buf);
	}

}
void ShellCmd_setlogflash(int argc, char** argv) {
    // Example hardcoded toggle for INFO
    flash_log_enabled[SHELL_LOG_INFO] = !flash_log_enabled[SHELL_LOG_INFO];
    LOG_INFO("Flash logging for INFO set to: %s\r\n", flash_log_enabled[SHELL_LOG_INFO] ? "ON" : "OFF");
}

void ShellCmd_log_dump(int argc, char** argv) {
    LOG_INFO("Reading logs from flash...\r\n");
    Flash_Log_Read();
}
void ShellCmd_clear(int argc, char** argv)
{
    Shell_Print("\033[2J\033[H");  // ANSI escape: clear screen + move cursor home
}

void ShellCmd_log_erase(int argc, char** argv){
	LOG_INFO("Erasing logs from flash...\r\n");
	Flash_Log_Erase();
	LOG_INFO("Flash erased...\r\n");
}
const ShellCommand shell_commands[] = {
		 {"led_on", ShellCmd_led_on, "Turn on the LED"},
		 {"led_off", ShellCmd_led_off, "Turn off the LED"},
		 {"hello", ShellCmd_hello, "Print hello message"},
		 {"get_sensor", ShellCmd_get_sensor, "Read temperature and humidity"},
		 {"get_accel", ShellCmd_get_accel, "Read accelerometer data"},
		 {"help", ShellCmd_help, "Show available commands"},
		{"logflash", ShellCmd_setlogflash, "Toggle Flash logging for INFO"},
		 {"log_dump", ShellCmd_log_dump, "Dump saved logs from flash"},
		 {"log_erase", ShellCmd_log_erase, "Erase saved logs from flash"},
		 {"memory_usage", ShellCmd_get_memory_data, "Get memory information"},
		 {"whoami", ShellCmd_whoami, "Show logged-in username"},
		 {"clear", ShellCmd_clear, "Clear the screen"},

};
const int shell_command_count = sizeof(shell_commands) / sizeof(ShellCommand);
