/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TACH1_Pin GPIO_PIN_1
#define TACH1_GPIO_Port GPIOA
#define TACH1_EXTI_IRQn EXTI1_IRQn
#define TACH2_Pin GPIO_PIN_2
#define TACH2_GPIO_Port GPIOA
#define TACH2_EXTI_IRQn EXTI2_IRQn
#define CURR_12V_Pin GPIO_PIN_3
#define CURR_12V_GPIO_Port GPIOA
#define VOLT_12V_Pin GPIO_PIN_4
#define VOLT_12V_GPIO_Port GPIOA
#define CURR_5V_Pin GPIO_PIN_5
#define CURR_5V_GPIO_Port GPIOA
#define VOLT_5V_Pin GPIO_PIN_6
#define VOLT_5V_GPIO_Port GPIOA
#define FTDI_NRST_Pin GPIO_PIN_1
#define FTDI_NRST_GPIO_Port GPIOB
#define CAN_STBY_Pin GPIO_PIN_15
#define CAN_STBY_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
