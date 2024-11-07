/*
 * traffic_light.h
 *
 *  Created on: Oct 1, 2024
 *      Author: romay
 */
#include "stm32f4xx_hal.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

typedef enum {
    RED,
    GREEN,
	GREEN_BLINK,
    YELLOW
} TrafficLight;

const char* getColorName(TrafficLight c);

struct mech_data{
	  int red_time;
	  int other_time;
	  int current_red;
	  int fsm_flag;
	  int button_pressed_flag;
	  int wait_time;
	  int start_time;
	  int button_cnt;
	  int work_mode;
	  TrafficLight state;

	  int interupt_enable;
};

void my_GPIO_INIT();

void set_LED(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

int read_button(int* cnt);

GPIO_PinState read_PIN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

void reset_LEDS(void);

void nextStateMachine(TrafficLight* light, int red_t, int other_t, int* current_red, int* flag, int* waiting_time, int* button_flag_button, int* start_time);

void check_button(int* cnt, int other_t, int* current_red, int* button_flag_button);

void deshifr(char* in_buf, int* cur_length, struct mech_data* tfl_obj);

void enable_interrupt(struct mech_data* tfl_obj);
void disable_interrupt(struct mech_data* tfl_obj);
void init_vals();

#endif /* INC_TRAFFIC_LIGHT_H_ */
