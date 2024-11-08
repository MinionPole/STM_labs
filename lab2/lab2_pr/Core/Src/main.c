/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "traffic_light.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
MechData fsm_obj;
char el;
char in_buffer_IT[50];
volatile static int len_in_buf;
volatile static int send_req;

char datas[1000];
static int already_printed;
static int last_data;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Function to initialize variables for interrupt mode */
void init_vals_IT()
{
    len_in_buf = 0;
    memset(in_buffer_IT, 0, sizeof(in_buffer_IT));
    send_req = 0;
    already_printed = 0;
    last_data = 0;
}

/* Function to collect data safely */
void data_collecter(char *buf, int len)
{
    if (buf == NULL)
        return; // Error checking
    for (int i = 0; i < len && last_data < (sizeof(datas) - 1); i++)
    {
        datas[last_data++] = buf[i];
    }
    datas[last_data] = '\0'; // Null-terminate
}

/* Function to transmit data via interrupt */
void transend_data_IT(char *buf, int len)
{
    if (buf == NULL)
        return; // Error checking
    if (send_req)
    {
        data_collecter(buf, len);
        return;
    }
    send_req = 1;
    HAL_UART_Transmit_IT(&huart6, (uint8_t *)buf, len);
}

/* Function to print collected data */
void print_collected_data()
{
    while (already_printed < last_data)
    {
        transend_data_IT(&datas[already_printed], 1);
        already_printed++;
    }
    if (already_printed >= last_data)
    {
        already_printed = 0;
        last_data = 0;
    }
}

/* UART Receive Complete Callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART6)
    {
        if (len_in_buf < (sizeof(in_buffer_IT) - 1))
        {
            in_buffer_IT[len_in_buf++] = el;
            transend_data_IT(&el, 1);
            if (in_buffer_IT[len_in_buf - 1] == '\r')
            {
                in_buffer_IT[len_in_buf - 1] = '\0'; // Replace carriage return with null terminator
                parser(in_buffer_IT, &len_in_buf, &fsm_obj);
                len_in_buf = 0;
            }
        }
        else
        {
            // Buffer overflow attempt, reset buffer
            len_in_buf = 0;
        }
    }
}

/* UART Transmit Complete Callback */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART6)
    {
        send_req = 0;
        print_collected_data();
    }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART6_UART_Init();
    /* USER CODE BEGIN 2 */
    fsm_obj.button_cnt = 0;
    fsm_obj.button_pressed_flag = 0;
    fsm_obj.red_time = 6000;
    fsm_obj.other_time = fsm_obj.red_time / 4;
    fsm_obj.current_red = fsm_obj.red_time;
    fsm_obj.start_time = 0;
    fsm_obj.wait_time = 0;
    fsm_obj.state = RED;
    fsm_obj.fsm_flag = 0;
    fsm_obj.interupt_enable = 0;
    fsm_obj.work_mode = 1;

    char in_buffer[50];
    int in_curlen = 0;
    int first_iter = -1;
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        nextStateMachine(&fsm_obj.state, fsm_obj.red_time, fsm_obj.other_time, &fsm_obj.current_red, &fsm_obj.fsm_flag,
                         &fsm_obj.wait_time, &fsm_obj.button_pressed_flag, &fsm_obj.start_time);

        if (fsm_obj.work_mode)
        {
            check_button(&fsm_obj.button_cnt, fsm_obj.other_time, &fsm_obj.current_red, &fsm_obj.button_pressed_flag);
        }

        if (!fsm_obj.interrupt_enable)
        {
            first_iter = 0;
            if (HAL_OK == HAL_UART_Receive(&huart6, (uint8_t *)&in_buffer[in_curlen], 1, 10))
            {
                HAL_UART_Transmit(&huart6, (uint8_t *)&in_buffer[in_curlen], 1, 10);
                if (in_buffer[in_curlen] == '\r')
                {
                    in_buffer[in_curlen] = '\0';
                    parser(in_buffer, &in_curlen, &fsm_obj);
                    in_curlen = 0;
                }
                else
                {
                    if (in_curlen < (sizeof(in_buffer) - 1))
                    {
                        in_curlen++;
                    }
                    else
                    {
                        // Prevent buffer overflow
                        in_curlen = 0;
                    }
                }
            }
        }
        else
        {
            if (first_iter == 0)
            {
                init_vals_IT();
                first_iter = 1;
            }
            else
            {
                HAL_UART_Receive_IT(&huart6, (uint8_t *)&el, 1);
            }
            // Interrupt-driven UART handling
        }
        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
