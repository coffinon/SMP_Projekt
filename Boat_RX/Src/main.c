/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "NRF24.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IDLE_STATE 50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static const uint64_t rx_pipe_addr = 0x11223344AA;
static uint8_t my_rx_data[MAX_PAYLOAD_SIZE + 2];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  NRF24_init(&hspi2);

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

  TIM1->CCR1 = 0;
  TIM1->CCR2 = 0;

  NRF24_openReadingPipe(1, rx_pipe_addr);
  NRF24_startListening();

  uint32_t watchdog = HAL_GetTick();
  uint8_t error_msg[] = "Connection Lost\r\n";
  const uint32_t timeout = 1400;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(NRF24_available()){
		  NRF24_read(my_rx_data, PAYLOAD_SIZE);

		  my_rx_data[PAYLOAD_SIZE] = '\r';
		  my_rx_data[PAYLOAD_SIZE + 1] = '\n';
		  HAL_UART_Transmit(&huart2, my_rx_data, PAYLOAD_SIZE + 2, 100);

		  if((my_rx_data[0] >= 40) && (my_rx_data[0] <= 60)){
		  		// IDLE STATE - STOP
		  		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
		  		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
		  		TIM1->CCR1 = 0;
		  		TIM1->CCR2 = 0;
		  	}
		  	else if((my_rx_data[0] >= 0) && (my_rx_data[0] < 40)){
		  		// SAIL BACKWARD
		  		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
		  		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
		  		if((my_rx_data[1] >= 40) && (my_rx_data[1] <= 60)){
		  			// SAIL STRAIGHT
		  			TIM1->CCR1 = my_rx_data[0];
		  			TIM1->CCR2 = my_rx_data[0];
		  		}
		  		else if((my_rx_data[1] >= 0) && (my_rx_data[1] < 20)){
		  			// SAIL FULL LEFT
		  			TIM1->CCR1 = my_rx_data[0];
		  			TIM1->CCR2 = 0;
		  		}
		  		else if((my_rx_data[1] >= 20) && (my_rx_data[1] < 40)){
		  			// SAIL HALF LEFT
		  			TIM1->CCR1 = my_rx_data[0];
		  			TIM1->CCR2 = (uint8_t)(my_rx_data[0] / 2.0);
		  		}
		  		else if((my_rx_data[1] > 80) && (my_rx_data[1] <= 100)){
		  			// SAIL FULL RIGHT
		  			TIM1->CCR1 = 0;
		  			TIM1->CCR2 = my_rx_data[0];
		  		}
		  		else if((my_rx_data[1] > 60) && (my_rx_data[1] <= 80)){
		  			// SAIL HALF RIGHT
		  			TIM1->CCR1 = (uint8_t)(my_rx_data[0] / 2.0);
		  			TIM1->CCR2 = my_rx_data[0];
		  		}
		  	}
		  	else if((my_rx_data[0] > 60) && (my_rx_data[0] <= 100)){
		  		// SAIL FORWARD
		  		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
		  		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
		  		if((my_rx_data[1] >= 40) && (my_rx_data[1] <= 60)){
		  			// SAIL STRAIGHT
		  			TIM1->CCR1 = my_rx_data[0];
		  			TIM1->CCR2 = my_rx_data[0];
		  		}
		  		else if((my_rx_data[1] >= 0) && (my_rx_data[1] < 20)){
		  			// SAIL FULL LEFT
		  			TIM1->CCR1 = 0;
		  			TIM1->CCR2 = my_rx_data[0];
		  		}
		  		else if((my_rx_data[1] >= 20) && (my_rx_data[1] < 40)){
		  			// SAIL HALF LEFT
		  			TIM1->CCR1 = (uint8_t)(my_rx_data[0] / 2.0);
		  			TIM1->CCR2 = my_rx_data[0];
		  		}
		  		else if((my_rx_data[1] > 80) && (my_rx_data[1] <= 100)){
		  			// SAIL FULL RIGHT
		  			TIM1->CCR1 = my_rx_data[0];
		  			TIM1->CCR2 = 0;
		  		}
		  		else if((my_rx_data[1] > 60) && (my_rx_data[1] <= 80)){
		  			// SAIL HALF RIGHT
		  			TIM1->CCR1 = my_rx_data[0];
		  			TIM1->CCR2 = (uint8_t)(my_rx_data[0] / 2.0);
		  		}
		  	}

		  watchdog = HAL_GetTick();
	  }
	  else if((HAL_GetTick() - watchdog) > timeout ){
		  my_rx_data[0] = IDLE_STATE;
		  my_rx_data[1] = IDLE_STATE;
		  HAL_UART_Transmit(&huart2, error_msg, sizeof(error_msg), 100);

		  HAL_Delay(100);
	  }
    /* USER CODE END WHILE */

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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
