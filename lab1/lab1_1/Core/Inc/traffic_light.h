/*
 * traffic_light.h
 *
 *  Created on: Oct 1, 2024
 *      Author: romay
 */
#include "stm32f4xx_hal.h"

#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

typedef enum {
    RED,
    GREEN,
	GREEN_BLINK,
    YELLOW
} TrafficLight;

void my_GPIO_INIT();

void set_LED(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

int read_button(int* cnt);

GPIO_PinState read_PIN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

void reset_LEDS(void);

void nextStateMachine(TrafficLight* light, int* red_t, int* other_t);



#endif /* INC_TRAFFIC_LIGHT_H_ */
