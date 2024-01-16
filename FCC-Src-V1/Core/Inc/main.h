/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
<<<<<<< HEAD
typedef struct
{
  uint8_t x, y, z;
} accData_t;

typedef struct
{
  uint8_t purge_state, supply_state;
  float internal_stack_temp, internal_stack_pressure;
  // Potentially some more data here
} fcData_t;

typedef struct
{
  // Data received from CAN
  uint8_t H2_OK;
  float cap_voltage;
} canData_t;
=======
typedef struct FuelCellData FCData;
>>>>>>> bcfaaaa94a4a245e26cd3efa2117dbda2653f00b
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
#define BRD_STRT_Pin GPIO_PIN_14
#define BRD_STRT_GPIO_Port GPIOC
#define BRD_STRT_EXTI_IRQn EXTI15_10_IRQn
#define BRD_PRGE_Pin GPIO_PIN_15
#define BRD_PRGE_GPIO_Port GPIOC
#define BRD_PRGE_EXTI_IRQn EXTI15_10_IRQn
#define SUPPLY_VLVE_Pin GPIO_PIN_1
#define SUPPLY_VLVE_GPIO_Port GPIOA
#define PURGE_VLVE_Pin GPIO_PIN_2
#define PURGE_VLVE_GPIO_Port GPIOA
#define EXT_STRT_Pin GPIO_PIN_3
#define EXT_STRT_GPIO_Port GPIOA
#define EXT_STRT_EXTI_IRQn EXTI3_IRQn
#define ACC_INT1_Pin GPIO_PIN_4
#define ACC_INT1_GPIO_Port GPIOA
#define ACC_INT1_EXTI_IRQn EXTI4_IRQn
#define ACC_INT2_Pin GPIO_PIN_5
#define ACC_INT2_GPIO_Port GPIOA
#define ACC_INT2_EXTI_IRQn EXTI9_5_IRQn
#define EXT_STOP_Pin GPIO_PIN_6
#define EXT_STOP_GPIO_Port GPIOA
#define EXT_STOP_EXTI_IRQn EXTI9_5_IRQn
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
