#include "led_control.h"
#include <stdio.h>
typedef struct {
	uint8_t id;
	GPIO_TypeDef* port;
	uint16_t pin;
} LedMapping;

static const LedMapping led_map[] = {
		{(uint8_t)LED_GREEN, GPIOB, GPIO_PIN_0},
		{(uint8_t)LED_BLUE, GPIOB, GPIO_PIN_7},
		{(uint8_t)LED_RED, GPIOB, GPIO_PIN_14}
};

size_t NUM_LEDS = sizeof(led_map)/sizeof(led_map[0]);

static LedMapping* get_led_mapping(LedColor led){
	for (int i=0; i < NUM_LEDS; ++i){
		if (led_map[i].id == led){
			return &led_map[i];
		}
	}
	return NULL;
}

void LED_Control(uint8_t led_id, GPIO_PinState state){

	LedMapping *mapping = get_led_mapping(led_id);

	if(!mapping) return; //We could receive a NULL

	HAL_GPIO_WritePin(mapping->port, mapping->pin, state);
}

