#ifndef LED_CONTROL_H_
#define LED_CONTROL_H_

#include "main.h"

#define LED_ON GPIO_PIN_SET
#define LED_OFF GPIO_PIN_RESET

typedef enum {
	LED_NONE =0,
	LED_GREEN=1,
	LED_BLUE =2,
	LED_RED =3
} LedColor;



void LED_Control(uint8_t led_id, GPIO_PinState state);




#endif
