/*
 * traffic_light.c
 *
 *  Created on: Oct 1, 2024
 *      Author: romay
 */
#include "traffic_light.h"

void my_GPIO_INIT(){
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();
	  // Отключил лишние пины

	  /*Configure GPIO pin Output Level */
	  set_LED(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

	  /*Configure GPIO pin : PC15 */
	  GPIO_InitStruct.Pin = GPIO_PIN_15;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : PD13 PD14 PD15 */
	  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  // судя по справке и нашей частоте нам нужна средняя скорость, а не low как было
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

void set_LED(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState){
	/* нам не нужны асерты, гарантируется работа
	 * старшие 16 бит бсра, отвечаю за сброс значения, младшие за установку*/
	  if(PinState != GPIO_PIN_RESET)
	  {
	    GPIOx->BSRR = GPIO_Pin;
	  }
	  else
	  {
	    GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;
	  }
};

GPIO_PinState read_PIN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_PinState bitstatus;
	if((GPIOx->IDR & GPIO_Pin) != (uint32_t)GPIO_PIN_RESET)
	  bitstatus = GPIO_PIN_SET;
	else
	  bitstatus = GPIO_PIN_RESET;
	return bitstatus;
}


int read_button(int* cnt){
	GPIO_PinState res = read_PIN(GPIOC, GPIO_PIN_15);
	if(res == 0){
		(*cnt) = (*cnt) + 1;
	}
	else
		(*cnt) = 0;
	/* при стандартном времени дребезга в 5-10мс, то при частоте 16 мега герц, нам нужно примерно 80-160 тактов, взял 200 для надежности
	 * */
	if((*cnt) >= 200){
		(*cnt) = 0;
		return 1;
	}
	return 0;
}

void reset_LEDS(void){
	set_LED(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
}

void nextStateMachine(TrafficLight* light, int* red_t, int* other_t){
	int button_pressed = 0;
	int start_time = 0;
	int waiting_time = 0;
	int cnt = 0;
    switch (*light) {
        case RED:
        	reset_LEDS();
        	set_LED(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        	start_time = HAL_GetTick();
        	waiting_time = start_time + (*red_t);
        	while(HAL_GetTick() < waiting_time) {
        		button_pressed = read_button(&cnt);
        		if(button_pressed){
        			(*red_t) = (*other_t);
        			waiting_time = start_time + (*red_t);
        		}
        	}
        	*light = GREEN;
            break;
        case GREEN:
        	reset_LEDS();
        	set_LED(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        	start_time = HAL_GetTick();
        	waiting_time = start_time + (*other_t);
        	while(HAL_GetTick() < waiting_time){
        		button_pressed = read_button(&cnt);
        		if(button_pressed){
        			(*red_t) = (*other_t);
        		}
        	}
        	*light = GREEN_BLINK;
            break;
        case GREEN_BLINK:
        	reset_LEDS();
        	set_LED(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        	start_time = HAL_GetTick();
        	int prev = start_time;
        	waiting_time = start_time + (*other_t);
        	while(HAL_GetTick() < waiting_time) {
        		button_pressed = read_button(&cnt);
        		if(button_pressed){
        			(*red_t) = (*other_t);
        		}
        		if(HAL_GetTick() - prev >= (*other_t) / 8){
        			prev = HAL_GetTick();
        			//inline toggle
        			uint32_t odr = GPIOD->ODR;
        			GPIOD->BSRR = ((odr & GPIO_PIN_13) << 16U) | (~odr & GPIO_PIN_13);
        		}
        	}
            *light = YELLOW;
            break;
        case YELLOW:
        	reset_LEDS();
        	set_LED(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        	start_time = HAL_GetTick();
        	waiting_time = start_time + (*other_t);
        	while(HAL_GetTick() < waiting_time){
        		button_pressed = read_button(&cnt);
        		if(button_pressed){
        			(*red_t) = (*other_t);
        		}
        	}
            *light = RED;
            break;
        default:
        	reset_LEDS();
        	*light = RED;
            break;
    }
}
