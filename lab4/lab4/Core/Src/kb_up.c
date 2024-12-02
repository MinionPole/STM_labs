#include "kb_up.h"

int click_cnt[13];
uint8_t checkButtons(){
	int cnt = 0;
	uint8_t mem = 0;
    uint8_t key1 = Check_Row(ROW1);
    if (key1 != 0x0D){
        cnt++;
        mem = key1;
    }
    if (key1 == 0x0F){
    	return key1;
    }
    uint8_t key2 = Check_Row(ROW2);
    if (key2 != 0x0D){
        cnt++;
    	mem = key2;
    }
    if (key2 == 0x0F){
    	return key2;
    }

    uint8_t key3 = Check_Row(ROW3);
    if (key3 != 0x0D){
        cnt++;
    	mem = key3;
    }
    if (key3 == 0x0F){
    	return key3;
    }
    uint8_t key4 = Check_Row(ROW4);
    if (key4 != 0x0D){
        cnt++;
    	mem = key4;
    }
    if (key4 == 0x0F){
    	return key4;
    }
    if(cnt == 0)
    	return 0x0D;
    if(cnt > 1)
    	return 0x0F;
    return mem;
}

void init_cnt(){
	for(int i = 0;i < 13;i++){
		click_cnt[i] = 0;
	}
}

int increase_cnt(uint8_t button_num){
	for(int i = 0;i < 13;i++){
		if(i != button_num){
			click_cnt[i] = 0;
		}
		else{
			if(click_cnt[i] <= DREBLIM)
				click_cnt[i] = click_cnt[i] + 1;
		}
	}
	if(click_cnt[button_num] == DREBLIM){
		return button_num;
	}
	return -1;
}

void convert_button_to_symbol(int button_num, char* el){
	if(button_num == 0){
		(*el) = '\r';
	}
	if(button_num == 1){
		(*el) = '1';
	}
	if(button_num == 2){
		(*el) = '2';
	}
	if(button_num == 3){
		(*el) = '3';
	}
	if(button_num == 4){
		(*el) = '4';
	}
	if(button_num == 5){
		(*el) = '5';
	}
	if(button_num == 6){
		(*el) = '6';
	}
	if(button_num == 7){
		(*el) = '7';
	}
	if(button_num == 8){
		(*el) = '+';
	}
	if(button_num == 9){
		(*el) = '-';
	}
	if(button_num == 11){
		(*el) = 'A';
	}
	if(button_num == 12){
		(*el) = 'a';
	}
}

int checkKB(char* el){
	uint8_t res = checkButtons();
	if(res == 0x0F) {
		return 0;
		// если несколько нажатых кнопок, просто игнорируем итерацию
	}
	if(res == 0x0D) {
		init_cnt();
		return 0;
	}
	int pressed_button = increase_cnt(res);
	if(pressed_button == -1){
		return 0;
		// кнопка не готова
	}
	convert_button_to_symbol(pressed_button, el);
	return 1;
}
