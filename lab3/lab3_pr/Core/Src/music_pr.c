/*
 * music_pr.c
 *
 *  Created on: Nov 22, 2024
 *      Author: romay
 */
#include "music_pr.h"

void change_now_oktav(int val) {
	int new_val = now_oktav + val;
	if(new_val >= 0 && new_val < 9)
		now_oktav = new_val;
}

void change_music_time(int val) {
	int new_music_time = music_time + val;
	if(new_music_time >= 10 && new_music_time <= 5000)
		music_time = new_music_time;
}

void analize_symb(char el){
	if(el >= '1' && el <= '7'){
		int note_to_ring = el - '1';
		execute_one_oktav(note_to_ring);
	}
	else
	if(el == '+' || el == '-'){
		if(el == '+')
			change_now_oktav(1);
		else
			change_now_oktav(-1);
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "change oktav_num, now is %d\r", now_oktav);
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	}
	else
	if(el == 'A' || el == 'a') {
		if(el == 'A')
			change_music_time(10);
		else
			change_music_time(-10);
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "change time, now is %d\r", music_time);
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	}
	else
	if(el == '\r'){
		execute_melody();
	}
	else
	{
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "wrong symbol %c\r", el);
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	}
}


void execute_one_oktav(int note_to_ring){
	char out_buffer[100];
	int transmitted_data_len;
	transmitted_data_len = sprintf(out_buffer, "note %s is ringning on %d oktav, time is %d\r", noteNames[note_to_ring], now_oktav, music_time);
	HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	htim1.Instance->ARR = oktava_info[now_oktav][note_to_ring] - 1;
	htim1.Instance->CCR1 = (oktava_info[now_oktav][note_to_ring] - 1) / 2;
	int start_time = HAL_GetTick();
	while(HAL_GetTick() - start_time < music_time){

	}
	htim1.Instance->CCR1 = 0;
}
void execute_melody(){
	for(int i = 0; i < 7;i++){
		execute_one_oktav(i);
	}
}
