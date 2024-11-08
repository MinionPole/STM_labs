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

/* Traffic Light States */
typedef enum {
    RED,
    GREEN,
	GREEN_BLINK,
    YELLOW
} TrafficLight;

/* Function to get the name of the traffic light color */
const char* getColorName(TrafficLight c);

/* Structure to hold traffic light mechanism data */
typedef struct {
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
} MechData;

/* GPIO Initialization */
void my_GPIO_INIT();

/* LED Control Functions */
void set_LED(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void reset_LEDS(void);

/* Button Handling */
int read_button(int* cnt);
GPIO_PinState read_PIN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

/* State Machine */
void nextStateMachine(TrafficLight* light, int red_t, int other_t, int* current_red, int* flag, int* waiting_time, int* button_flag_button, int* start_time);
void check_button(int* cnt, int other_t, int* current_red, int* button_flag_button);

/* Command Handling */
void parser(char* in_buf, int* cur_length, struct mech_data* tfl_obj);

/* Interrupt Control */
void enable_interrupt(struct mech_data* tfl_obj);
void disable_interrupt(struct mech_data* tfl_obj);

/* Initialization */
void init_vals_IT();

#endif /* INC_TRAFFIC_LIGHT_H_ */
