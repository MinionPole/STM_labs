/*
 * kb_up.h
 *
 *  Created on: 2 дек. 2024 г.
 *      Author: romay
 */

#ifndef INC_KB_UP_H_
#define INC_KB_UP_H_

#include "stm32f4xx_hal.h"
#include "usart.h"
#include "kb.h"
#include <string.h>
#include <stdio.h>

#define DREBLIM 5

uint8_t checkButtons();

int checkKB(char* el);

void init_cnt();

int increase_cnt(uint8_t button_num);

void convert_button_to_symbol(int button_num, char* el);


/*
0| 1|2
3| 4|5
6| 7|8
9|10|11
*/


#endif /* INC_KB_UP_H_ */
