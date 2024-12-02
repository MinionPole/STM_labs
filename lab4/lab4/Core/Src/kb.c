#include "kb.h"

#define KBRD_ADDR 0xE2

HAL_StatusTypeDef Set_Keyboard( void ) {
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t buf;

	buf = 0;
	ret = PCA9538_Write_Register(KBRD_ADDR, POLARITY_INVERSION, &buf);
	if( ret != HAL_OK ) {
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "Error write polarity\r");
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
		goto exit;
	}

	buf = 0;
	ret = PCA9538_Write_Register(KBRD_ADDR, OUTPUT_PORT, &buf);
	if( ret != HAL_OK ) {
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "Error write output\r");
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	}

exit:
	return ret;
}

uint8_t Check_Row( uint8_t  Nrow ) {
	uint8_t Nkey = 0x00;
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t buf;
	uint8_t kbd_in;
	int cnt = 0;

	ret = Set_Keyboard();
	if( ret != HAL_OK ) {
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "Error write init\r");
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	}

	buf = Nrow;
	ret = PCA9538_Write_Register(KBRD_ADDR, CONFIG, &buf);
	if( ret != HAL_OK ) {
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "Error write config\r");
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	}

	ret = PCA9538_Read_Inputs(KBRD_ADDR, &buf);
	if( ret != HAL_OK ) {
		char out_buffer[100];
		int transmitted_data_len;
		transmitted_data_len = sprintf(out_buffer, "Read error\r");
		HAL_UART_Transmit( &huart6, (uint8_t *) out_buffer, transmitted_data_len, 30 );
	}

	kbd_in = buf & 0x70;
	Nkey = kbd_in;
	if( kbd_in != 0x70) {
		if( !(kbd_in & 0x10) ) {
			switch (Nrow) {
				case ROW1:
					cnt++;
					Nkey = 0x07;
					break;
				case ROW2:
					cnt++;
					Nkey = 0x04;
					break;
				case ROW3:
					cnt++;
					Nkey = 0x01;
					break;
				case ROW4:
					cnt++;
					Nkey = 0x00;
					break;
			}
		}
		if( !(kbd_in & 0x20) ) {
			switch (Nrow) {
				case ROW1:
					cnt++;
					Nkey = 0x08;
					break;
				case ROW2:
					cnt++;
					Nkey = 0x05;
					break;
				case ROW3:
					cnt++;
					Nkey = 0x02;
					break;
				case ROW4:
					cnt++;
					Nkey = 0x0B;
					break;
			}
		}
		if( !(kbd_in & 0x40) ) {
			switch (Nrow) {
				case ROW1:
					cnt++;
					Nkey = 0x09;
					break;
				case ROW2:
					cnt++;
					Nkey = 0x06;
					break;
				case ROW3:
					cnt++;
					Nkey = 0x03;
					break;
				case ROW4:
					cnt++;
					Nkey = 0x0C;
					break;
			}
		}
	}
	else Nkey = 0x0D;
	if(cnt > 1)
		Nkey =0x0F;
	return Nkey;
}
