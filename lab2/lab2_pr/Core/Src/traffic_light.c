/*
 * traffic_light.c
 *
 *  Created on: Oct 1, 2024
 *      Author: romay
 */
#include "traffic_light.h"
#define BUTTON_PRESS_THRESHOLD 200
#define UART_TIMEOUT 30
#define OUT_BUFFER_SIZE 128
#define GREEN_BLINK_INTERVAL_DIVIDER 8

/**
 * @brief Function to initialize GPIOs
 */
void my_GPIO_INIT()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* USER CODE BEGIN MX_GPIO_Init_1 */
    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /* Disabled unnecessary pins */

    /* Configure GPIO pin Output Level */
    set_LED(GPIOD, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);

    /* Configure GPIO pin : PC15 as Input */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Configure GPIO pins : PD13 PD14 PD15 as Output */
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; // Using medium speed as per specifications
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
 * @brief Function to set LED state
 * @param GPIOx - Pointer to the GPIO port
 * @param GPIO_Pin - Pin number
 * @param PinState - State of the pin
 */
void set_LED(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    if (PinState != GPIO_PIN_RESET)
    {
        GPIOx->BSRR = GPIO_Pin;
    }
    else
    {
        GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;
    }
};

/**
 * @brief Function to read PIN state
 * @param GPIOx - Pointer to the GPIO port
 * @param GPIO_Pin - Pin number
 * @return GPIO_PinState - State of the pin
 */
GPIO_PinState read_PIN(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{

    GPIO_PinState bitstatus;
    if ((GPIOx->IDR & GPIO_Pin))
        bitstatus = GPIO_PIN_SET;
    else
        bitstatus = GPIO_PIN_RESET;
    return bitstatus;
}

/**
 * @brief Function to get color name
 * @param c - TrafficLight enum value
 * @return const char* - Color name
 */
const char *getColorName(TrafficLight c)
{
    switch (c)
    {
    case RED:
        return "RED";
    case GREEN:
        return "GREEN";
    case GREEN_BLINK:
        return "GREEN_BLINK";
    case YELLOW:
        return "YELLOW";
    default:
        return "UNKNOWN";
    }
}

/**
 * @brief to read button with debounce
 * @param cnt - pointer to the counter
 * @return 1 if the button is pressed, 0 otherwise
 */
int read_button(int *cnt)
{
    GPIO_PinState res = read_PIN(GPIOC, GPIO_PIN_15);
    if (res == 0)
    {
        (*cnt) = (*cnt) + 1;
    }
    else
        (*cnt) = 0;

    if ((*cnt) >= BUTTON_PRESS_THRESHOLD)
    {
        (*cnt) = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief Transmits data via UART with interrupt handling.
 *
 * @param buffer Data to transmit.
 * @param length Length of the data.
 * @param interrupt_enabled Flag to determine transmission method.
 */
void transmit_data(const char *buffer, int length, int interrupt_enabled)
{
    if (interrupt_enabled)
    {
        transmit_data_IT(buffer, length);
    }
    else
    {
        HAL_UART_Transmit(&huart6, (uint8_t *)buffer, length, UART_TIMEOUT);
    }
}

/**
 * @brief Function to reset LEDs
 */
void reset_LEDS(void)
{
    set_LED(GPIOD, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);
}

/**
 * @brief Function to update the state of the traffic light
 */
void nextStateMachine(TrafficLight *light, int red_t, int other_t, int *current_red, int *flag, int *waiting_time,
                      int *button_flag_button, int *start_time)
{
    static int prev = 0;
    if (light == NULL || current_red == NULL || flag == NULL || waiting_time == NULL || button_flag_button == NULL ||
        start_time == NULL)
    {
        return; // Error checking
    }
    switch (*light)
    {
    case RED:
        if (*flag == 1)
        { // Waiting
            if (HAL_GetTick() >= *waiting_time)
            {
                *light = GREEN;
                *flag = 0;
            }
            if (*button_flag_button)
            {
                *button_flag_button = 0;
                *waiting_time = *start_time + *current_red;
                *current_red = red_t;
            }
        }
        else
        {
            reset_LEDS();
            set_LED(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
            *start_time = HAL_GetTick();
            *waiting_time = *start_time + *current_red;
            *button_flag_button = 0;
            *flag = 1;
        }
        break;
    case GREEN:
        if (*flag == 1)
        { // Waiting
            if (HAL_GetTick() >= *waiting_time)
            {
                *light = GREEN_BLINK;
                *flag = 0;
            }
        }
        else
        {
            reset_LEDS();
            set_LED(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            *start_time = HAL_GetTick();
            *waiting_time = *start_time + other_t;
            *flag = 1;
        }
        break;

    case GREEN_BLINK:
        if (*flag == 1)
        { // Waiting
            if (HAL_GetTick() >= *waiting_time)
            {
                *light = YELLOW;
                *flag = 0;
            }
            if ((HAL_GetTick() - prev) >= (other_t / GREEN_BLINK_INTERVAL_DIVIDER))
            {
                prev = HAL_GetTick();
                // Toggle LED13
                GPIOD->ODR ^= GPIO_PIN_13;
            }
        }
        else
        {
            reset_LEDS();
            set_LED(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            *start_time = HAL_GetTick();
            *waiting_time = *start_time + other_t;
            prev = HAL_GetTick();
            *flag = 1;
        }
        break;
    case YELLOW:
        if (*flag == 1)
        { // Waiting
            if (HAL_GetTick() >= *waiting_time)
            {
                *light = RED;
                *flag = 0;
            }
        }
        else
        {
            reset_LEDS();
            set_LED(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
            *start_time = HAL_GetTick();
            *waiting_time = *start_time + other_t;
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

/**
 * @brief Function to check if the button is pressed
 * @param cnt - pointer to the counter
 * @param other_t - other time
 * @param current_red - pointer to the current red time
 * @param button_flag_button - pointer to the button flag
 */
void check_button(int* cnt, int other_t, int* current_red, int* button_flag_button){
    if(cnt == NULL || current_red == NULL || button_flag_button == NULL) return; // Error checking
    if(read_button(cnt)){
        *current_red = other_t;
        *button_flag_button = 1;
    }
}

void parser(char *in_buf, int *cur_length, struct mech_data *tfl_obj)
{
     char out_buffer[OUT_BUFFER_SIZE];
    if(in_buf == NULL || cur_length == NULL || tfl_obj == NULL) return; // Error checking

    if(strncmp(in_buf, "!", 1) == 0){
        int transmitted_data_len = snprintf(out_buffer, OUT_BUFFER_SIZE, "current_state is %s, timeout is %d, mode is %d, I is %d\r",
            getColorName(tfl_obj->state), tfl_obj->red_time, tfl_obj->work_mode, tfl_obj->interrupt_enable);
        transmit_data(out_buffer, transmitted_data_len, tfl_obj->interrupt_enable);
    }

    char* set_mode_flag = strstr(in_buf, "set mode ");
    if(set_mode_flag) {
        int val = (*(set_mode_flag + 9) - '0');
        tfl_obj->work_mode = val;
        if(val == 0)
            tfl_obj->button_pressed_flag = 0;
        int transmitted_data_len = snprintf(out_buffer, OUT_BUFFER_SIZE, "data change to %d\r", val);
        transmit_data(out_buffer, transmitted_data_len, tfl_obj->interrupt_enable);
    }

    char* set_red_time = strstr(in_buf, "set timeout ");
    if(set_red_time) {
        set_red_time += strlen("set timeout ");
        int val = 0;
        while(*set_red_time != '\0') {
            if(*set_red_time < '0' || *set_red_time > '9') break; // Validate input
            val = val * 10 + (*set_red_time - '0');
            set_red_time++;
        }
        if(tfl_obj->current_red == tfl_obj->red_time)
            tfl_obj->current_red = val;
        else
            tfl_obj->current_red = val / 4;
        tfl_obj->red_time = val;
        tfl_obj->other_time = tfl_obj->red_time / 4;
        int transmitted_data_len = snprintf(out_buffer, OUT_BUFFER_SIZE, "red timeout change to %d\r", tfl_obj->current_red);
        transmit_data(out_buffer, transmitted_data_len, tfl_obj->interrupt_enable);
    }

    if(strncmp(in_buf, "set interrupts on", 17) == 0) {
        int transmitted_data_len = snprintf(out_buffer, OUT_BUFFER_SIZE, "interrupt enabled\r");
        enable_interrupt(tfl_obj);
        transmit_data(out_buffer, transmitted_data_len, tfl_obj->interrupt_enable);
    }

    if(strncmp(in_buf, "set interrupts off", 18) == 0) {
        int transmitted_data_len = snprintf(out_buffer, OUT_BUFFER_SIZE, "interrupt disabled\r");
        disable_interrupt(tfl_obj);
        transmit_data(out_buffer, transmitted_data_len, tfl_obj->interrupt_enable);
    }
    return;
}

/**
 * @brief Function to enable interrupts
 * @param tfl_obj - pointer to the traffic light object
 */
void enable_interrupt(MechData* tfl_obj) {
    if(tfl_obj == NULL) return; // Error checking
    HAL_NVIC_EnableIRQ(USART6_IRQn);
    tfl_obj->interrupt_enable = 1;
}

/**
 * @brief Function to disable interrupts
 * @param tfl_obj - pointer to the traffic light object
 */
void disable_interrupt(MechData* tfl_obj) {
    if(tfl_obj == NULL) return; // Error checking
    HAL_UART_AbortReceive(&huart6);
    HAL_NVIC_DisableIRQ(USART6_IRQn);
    tfl_obj->interrupt_enable = 0;
}
