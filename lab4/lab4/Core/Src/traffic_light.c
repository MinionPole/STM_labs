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
	if((GPIOx->IDR & GPIO_Pin))
	  bitstatus = GPIO_PIN_SET;
	else
	  bitstatus = GPIO_PIN_RESET;
	return bitstatus;
}

const char* getColorName(TrafficLight c) {
    switch (c) {
        case RED: return "RED";
        case GREEN: return "GREEN";
        case GREEN_BLINK: return "GREEN_BLINK";
        case YELLOW: return "YELLOW";
        default: return "UNKNOWN";
    }
}


int read_button(int* cnt){
	GPIO_PinState res = read_PIN(GPIOC, GPIO_PIN_15);
	if(res == 0){
		(*cnt) = (*cnt) + 1;
	}
	else
		(*cnt) = 0;
	if((*cnt) >= 200){
		(*cnt) = 0;
		return 1;
	}
	return 0;
}

void reset_LEDS(void){
	set_LED(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
}

void nextStateMachine(TrafficLight* light, int red_t, int other_t, int* current_red, int* flag, int* waiting_time, int* button_flag_button, int* start_time) {
	static int prev = 0;
    switch (*light) {
        case RED:
        	if(*flag == 1){ // ждём
        		if(HAL_GetTick() >= *waiting_time){
        			*light = GREEN;
        			*flag = 0;
        		}
        		if(*button_flag_button){
        			(*button_flag_button) = 0;
        			(*waiting_time) = (*start_time) + (*current_red);
        			(*current_red) = red_t;
        		}
        	}
        	else{
        		reset_LEDS();
        		set_LED(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        		(*start_time) = HAL_GetTick();
        		*waiting_time = (*start_time) + (*current_red);
        		*button_flag_button = 0;
        		(*current_red) = red_t;
        		*flag = 1;
        	}
            break;
        case GREEN:
        	if(*flag == 1){ // ждём
        		if(HAL_GetTick() >= *waiting_time){
        			*light = GREEN_BLINK;
        			*flag = 0;
        		}
        	}
        	else{
        		reset_LEDS();
        		set_LED(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        		(*start_time) = HAL_GetTick();
        		(*waiting_time) = (*start_time) + (other_t);
        		*flag = 1;
        	}
            break;
        case GREEN_BLINK:
        	if(*flag == 1){ // ждём
        		if(HAL_GetTick() >= *waiting_time){
        			*light = YELLOW;
        			*flag = 0;
        		}
        		if(HAL_GetTick() - prev >= (other_t) / 8) {
        			prev = HAL_GetTick();
        			//inline toggle
        			uint32_t odr = GPIOD->ODR;
        			GPIOD->BSRR = ((odr & GPIO_PIN_13) << 16U) | (~odr & GPIO_PIN_13);
        		}
        	}
        	else{
        		reset_LEDS();
        		set_LED(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        		(*start_time) = HAL_GetTick();
        		(*waiting_time) = (*start_time) + (other_t);
        		prev = HAL_GetTick();
        		*flag = 1;
        	}
            break;
        case YELLOW:
        	if(*flag == 1){ // ждём
        		if(HAL_GetTick() >= *waiting_time){
        			*light = RED;
        			*flag = 0;
        		}
        	}
        	else{
        		reset_LEDS();
        		set_LED(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        		(*start_time) = HAL_GetTick();
        		(*waiting_time) = (*start_time) + (other_t);
        		*flag = 1;
        	}
            break;
        default:
        	reset_LEDS();
        	*flag = 0;
        	*light = RED;
            break;
    }
}

void check_button(int* cnt, int other_t, int* current_red, int* button_flag_button){
	if(read_button(cnt)){
		(*current_red) = other_t;
		(*button_flag_button) = 1;
	}
}

void enable_interrupt(struct mech_data* tfl_obj) {
	HAL_NVIC_EnableIRQ(USART6_IRQn);
	tfl_obj->interupt_enable = 1;
}

void disable_interrupt(struct mech_data* tfl_obj) {
	HAL_UART_AbortReceive(&huart6);
	HAL_NVIC_DisableIRQ(USART6_IRQn);
	tfl_obj->interupt_enable = 0;
}

void deshifr(char* in_buf, int* cur_length, struct mech_data* tfl_obj){
	int cmp_res = strncmp(in_buf, "!", 1);
	if(cmp_res == 0){
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "current_state is %s, timeout is %d, mode is %d, I is %d\r", getColorName(tfl_obj->state), tfl_obj->red_time, tfl_obj->work_mode, tfl_obj->interupt_enable);
		if(tfl_obj->interupt_enable) {
			transend_data_IT(out_buffer, transmitted_data_len);
		}
		else{
			HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
		}
	}
	char* set_mode_flag = strstr(in_buf, "set mode ");
	if(set_mode_flag) {
		int val = (*(set_mode_flag + 9) - '0');
		tfl_obj->work_mode = val;
		if(val == 0)
			tfl_obj->button_pressed_flag = 0;
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "data change to %d\r", val);
		if(tfl_obj->interupt_enable){
			transend_data_IT(out_buffer, transmitted_data_len);
		}
		else{
		  HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 10 );
		}
	}
	char* set_red_time = strstr(in_buf, "set timeout ");
	if(set_red_time) {
		set_red_time = set_red_time + 12;
		int val = 0;
		while(*set_red_time != '\0' && *set_red_time != '\r') {
			val = val * 10 + (*set_red_time - '0');
			set_red_time++;
		}
		if(tfl_obj->current_red == tfl_obj->red_time)
			tfl_obj->current_red = val;
		else
			tfl_obj->current_red = val / 4;
		tfl_obj->red_time = val;
		tfl_obj->other_time = tfl_obj-> red_time / 4;
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "red timeout change to %d\r", tfl_obj->current_red);
		if(tfl_obj->interupt_enable){
	      transend_data_IT(out_buffer, transmitted_data_len);
		}
		else{
		  HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 10 );
		}
	}

	cmp_res = strncmp(in_buf, "set interrupts on", 17);
	if(cmp_res == 0) {
		enable_interrupt(tfl_obj);
	}

	cmp_res = strncmp(in_buf, "set interrupts off", 18);
	if(cmp_res == 0) {
		disable_interrupt(tfl_obj);
	}

	return;
}
