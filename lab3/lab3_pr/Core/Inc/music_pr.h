/*
 * music_pr.h
 *
 *  Created on: Nov 22, 2024
 *      Author: romay
 */

#ifndef INC_MUSIC_PR_H_
#define INC_MUSIC_PR_H_
#include "tim.h"
#include "usart.h"
#include <stdio.h>

int now_oktav = 4;
int music_time = 1000;

uint32_t oktava_info[9][7] = {
		{36781, 32690, 29124, 27489, 24490, 21818, 19438},
		{18391, 16345, 14562, 13745, 12245, 10909, 9719},
		{9195, 8172, 7281, 6872, 6122, 5455, 4859},
		{4598, 4086, 3640, 3436, 3061, 2727, 2430},
		{2299, 2043, 1820, 1718, 1531, 1364, 1215},
		{1149, 1022, 910, 859, 765, 682, 607},
		{575, 511, 455, 430, 383, 341, 304},
		{287, 255, 228, 215, 191, 170, 152},
		{144, 128, 114, 107, 96, 85, 76}
};

const char* noteNames[7] = {
    "DO",   // 0
    "RE",   // 1
    "MI",   // 2
    "FA",   // 3
    "SOL",  // 4
    "LYA",  // 5
    "SI"    // 6
};

void change_now_oktav(int val);
void change_music_time(int val);
void analize_symb(char el);
void set_pulse(int time);

void execute_one_oktav(int note_to_ring);
void execute_melody();

#endif /* INC_MUSIC_PR_H_ */
