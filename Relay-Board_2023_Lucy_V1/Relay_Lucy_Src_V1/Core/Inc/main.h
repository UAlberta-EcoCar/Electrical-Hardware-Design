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
#include "stm32f4xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FC_VOLT_Pin GPIO_PIN_4
#define FC_VOLT_GPIO_Port GPIOA
#define MOTOR_CURR_Pin GPIO_PIN_5
#define MOTOR_CURR_GPIO_Port GPIOA
#define CAP_VOLT_Pin GPIO_PIN_6
#define CAP_VOLT_GPIO_Port GPIOA
#define CAP_CURR_Pin GPIO_PIN_7
#define CAP_CURR_GPIO_Port GPIOA
#define MTR_VOLT_Pin GPIO_PIN_4
#define MTR_VOLT_GPIO_Port GPIOC
#define FC_CURR_Pin GPIO_PIN_5
#define FC_CURR_GPIO_Port GPIOC
#define DSCHRGE_RELAY_Pin GPIO_PIN_0
#define DSCHRGE_RELAY_GPIO_Port GPIOB
#define RES_RELAY_Pin GPIO_PIN_1
#define RES_RELAY_GPIO_Port GPIOB
#define CAP_RELAY_Pin GPIO_PIN_2
#define CAP_RELAY_GPIO_Port GPIOB
#define MTR_RELAY_Pin GPIO_PIN_10
#define MTR_RELAY_GPIO_Port GPIOB
#define DSCHRGE_LED_Pin GPIO_PIN_12
#define DSCHRGE_LED_GPIO_Port GPIOB
#define RES_LED_Pin GPIO_PIN_13
#define RES_LED_GPIO_Port GPIOB
#define MTR_LED_Pin GPIO_PIN_14
#define MTR_LED_GPIO_Port GPIOB
#define CAP_LED_Pin GPIO_PIN_15
#define CAP_LED_GPIO_Port GPIOB
#define CAN_STBY_Pin GPIO_PIN_7
#define CAN_STBY_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
