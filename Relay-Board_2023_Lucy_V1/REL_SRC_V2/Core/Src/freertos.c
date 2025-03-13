/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "ecocar_can.h"
#include "main.h"
#include "stm32f4xx_hal_can.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc.h"
#include "can.h"
#include "debug-log.h"
#include "exported_typedef.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */
typedef struct {
  float fc_volt;
  float fc_curr;
  float mtr_volt;
  float mtr_curr;
  float cap_volt;
  float cap_curr;
} rbData_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
rbState_t relay_state = RELAY_STBY;
fetState_t fet_state = FET_STBY;
rbData_t relay_board_data;
bool lock_state = false;

volatile uint32_t adc1Results[3];
volatile uint32_t adc2Results[3];
/* USER CODE END Variables */
/* Definitions for relayTask */
osThreadId_t relayTaskHandle;
uint32_t RelayTaskBuffer[1024];
osStaticThreadDef_t RelayTaskControlBlock;
const osThreadAttr_t relayTask_attributes = {
    .name = "relayTask",
    .cb_mem = &RelayTaskControlBlock,
    .cb_size = sizeof(RelayTaskControlBlock),
    .stack_mem = &RelayTaskBuffer[0],
    .stack_size = sizeof(RelayTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for canReceiveTask */
osThreadId_t canReceiveTaskHandle;
uint32_t CanRtrTaskBuffer[1024];
osStaticThreadDef_t CanRtrTaskControlBlock;
const osThreadAttr_t canReceiveTask_attributes = {
    .name = "canReceiveTask",
    .cb_mem = &CanRtrTaskControlBlock,
    .cb_size = sizeof(CanRtrTaskControlBlock),
    .stack_mem = &CanRtrTaskBuffer[0],
    .stack_size = sizeof(CanRtrTaskBuffer),
    .priority = (osPriority_t)osPriorityAboveNormal,
};
/* Definitions for adcTask */
osThreadId_t adcTaskHandle;
uint32_t AdcTaskBuffer[1024];
osStaticThreadDef_t AdcTaskControlBlock;
const osThreadAttr_t adcTask_attributes = {
    .name = "adcTask",
    .cb_mem = &AdcTaskControlBlock,
    .cb_size = sizeof(AdcTaskControlBlock),
    .stack_mem = &AdcTaskBuffer[0],
    .stack_size = sizeof(AdcTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for indicatorLedTas */
osThreadId_t indicatorLedTasHandle;
uint32_t IndicateLedTaskBuffer[1024];
osStaticThreadDef_t IndicateLedTaskControlBlock;
const osThreadAttr_t indicatorLedTas_attributes = {
    .name = "indicatorLedTas",
    .cb_mem = &IndicateLedTaskControlBlock,
    .cb_size = sizeof(IndicateLedTaskControlBlock),
    .stack_mem = &IndicateLedTaskBuffer[0],
    .stack_size = sizeof(IndicateLedTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal4,
};
/* Definitions for canSendTask */
osThreadId_t canSendTaskHandle;
uint32_t CanDataTaskBuffer[1024];
osStaticThreadDef_t CanDataTaskControlBlock;
const osThreadAttr_t canSendTask_attributes = {
    .name = "canSendTask",
    .cb_mem = &CanDataTaskControlBlock,
    .cb_size = sizeof(CanDataTaskControlBlock),
    .stack_mem = &CanDataTaskBuffer[0],
    .stack_size = sizeof(CanDataTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal4,
};
/* Definitions for StartUsb */
osThreadId_t StartUsbHandle;
uint32_t StartUsbBuffer[1024];
osStaticThreadDef_t StartUsbControlBlock;
const osThreadAttr_t StartUsb_attributes = {
    .name = "StartUsb",
    .cb_mem = &StartUsbControlBlock,
    .cb_size = sizeof(StartUsbControlBlock),
    .stack_mem = &StartUsbBuffer[0],
    .stack_size = sizeof(StartUsbBuffer),
    .priority = (osPriority_t)osPriorityNormal5,
};
/* Definitions for canReceiveHeader */
osMessageQueueId_t canReceiveHeaderHandle;
uint8_t canReceiveHeaderBuffer[512 * sizeof(uint32_t)];
osStaticMessageQDef_t canReceiveHeaderControlBlock;
const osMessageQueueAttr_t canReceiveHeader_attributes = {
    .name = "canReceiveHeader",
    .cb_mem = &canReceiveHeaderControlBlock,
    .cb_size = sizeof(canReceiveHeaderControlBlock),
    .mq_mem = &canReceiveHeaderBuffer,
    .mq_size = sizeof(canReceiveHeaderBuffer)};
/* Definitions for canRxHeader */
osMessageQueueId_t canRxDataHandle;
uint8_t canRxHeaderBuffer[512 * sizeof(uint8_t)];
osStaticMessageQDef_t canRxHeaderControlBlock;
const osMessageQueueAttr_t canRxHeader_attributes = {
    .name = "canRxHeader",
    .cb_mem = &canRxHeaderControlBlock,
    .cb_size = sizeof(canRxHeaderControlBlock),
    .mq_mem = &canRxHeaderBuffer,
    .mq_size = sizeof(canRxHeaderBuffer)};
/* Definitions for usbQueReceive */
osMessageQueueId_t usbQueReceiveHandle;
uint8_t usbQueReceiveBuffer[2048 * sizeof(char)];
osStaticMessageQDef_t usbQueReceiveControlBlock;
const osMessageQueueAttr_t usbQueReceive_attributes = {
    .name = "usbQueReceive",
    .cb_mem = &usbQueReceiveControlBlock,
    .cb_size = sizeof(usbQueReceiveControlBlock),
    .mq_mem = &usbQueReceiveBuffer,
    .mq_size = sizeof(usbQueReceiveBuffer)};
/* Definitions for usbQueSend */
osMessageQueueId_t usbQueSendHandle;
uint8_t usbQueSendBuffer[2048 * sizeof(char)];
osStaticMessageQDef_t usbQueSendControlBlock;
const osMessageQueueAttr_t usbQueSend_attributes = {
    .name = "usbQueSend",
    .cb_mem = &usbQueSendControlBlock,
    .cb_size = sizeof(usbQueSendControlBlock),
    .mq_mem = &usbQueSendBuffer,
    .mq_size = sizeof(usbQueSendBuffer)};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN_RxHeaderTypeDef RxHeader;
  uint8_t RxData[8];

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) !=
      HAL_OK) {
    Error_Handler();
  }

  osMessageQueuePut(canReceiveHeaderHandle, &RxHeader.StdId, 0, 0);
  osMessageQueuePut(canReceiveHeaderHandle, &RxHeader.DLC, 0, 0);
  for (uint8_t i = 0; i < mapDlcToBytes(RxHeader.DLC); i++) {
    osMessageQueuePut(canRxDataHandle, &RxData[i], 0, 0);
  }
}

/* USER CODE END FunctionPrototypes */

void StartRelayTask(void *argument);
void StartCanReceive(void *argument);
void StartAdcTask(void *argument);
void StartIndicateLedTask(void *argument);
void StartCanDataTask(void *argument);
extern void StartUsb(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of canReceiveHeader */
  canReceiveHeaderHandle =
      osMessageQueueNew(512, sizeof(uint32_t), &canReceiveHeader_attributes);

  /* creation of canRxHeader */
  canRxDataHandle =
      osMessageQueueNew(512, sizeof(uint8_t), &canRxHeader_attributes);

  /* creation of usbQueReceive */
  usbQueReceiveHandle =
      osMessageQueueNew(2048, sizeof(char), &usbQueReceive_attributes);

  /* creation of usbQueSend */
  usbQueSendHandle =
      osMessageQueueNew(2048, sizeof(char), &usbQueSend_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of relayTask */
  relayTaskHandle = osThreadNew(StartRelayTask, NULL, &relayTask_attributes);

  /* creation of canReceiveTask */
  canReceiveTaskHandle =
      osThreadNew(StartCanReceive, NULL, &canReceiveTask_attributes);

  /* creation of adcTask */
  adcTaskHandle = osThreadNew(StartAdcTask, NULL, &adcTask_attributes);

  /* creation of indicatorLedTas */
  indicatorLedTasHandle =
      osThreadNew(StartIndicateLedTask, NULL, &indicatorLedTas_attributes);

  /* creation of canSendTask */
  canSendTaskHandle =
      osThreadNew(StartCanDataTask, NULL, &canSendTask_attributes);

  /* creation of StartUsb */
  StartUsbHandle = osThreadNew(StartUsb, NULL, &StartUsb_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartRelayTask */
/**
 * @brief  Function implementing the relayTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartRelayTask */
void StartRelayTask(void *argument) {
  /* USER CODE BEGIN StartRelayTask */
  /* Infinite loop */
  for (;;) {
    switch (relay_state) {
    case RELAY_STBY:
      HAL_GPIO_WritePin(GPIOB,
                        DSCHRGE_RELAY_Pin | RES_RELAY_Pin | CAP_RELAY_Pin |
                            MTR_RELAY_Pin | RES_LED_Pin | MTR_LED_Pin |
                            CAP_LED_Pin,
                        GPIO_PIN_RESET);
      break;
    case RELAY_STRTP:
      HAL_GPIO_WritePin(
          GPIOB, CAP_RELAY_Pin | MTR_RELAY_Pin | MTR_LED_Pin | CAP_LED_Pin,
          GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, DSCHRGE_RELAY_Pin | RES_RELAY_Pin | RES_LED_Pin,
                        GPIO_PIN_SET);
      break;
    case RELAY_CHRGE:
      HAL_GPIO_WritePin(GPIOB,
                        DSCHRGE_RELAY_Pin | CAP_RELAY_Pin | MTR_RELAY_Pin |
                            MTR_LED_Pin | CAP_LED_Pin,
                        GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, RES_RELAY_Pin | RES_LED_Pin, GPIO_PIN_SET);
      break;
    case RELAY_RUN:
      HAL_GPIO_WritePin(GPIOB, RES_RELAY_Pin | RES_LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB,
                        DSCHRGE_RELAY_Pin | CAP_RELAY_Pin | MTR_RELAY_Pin |
                            MTR_LED_Pin | CAP_LED_Pin,
                        GPIO_PIN_SET);
      break;
    }
    osDelay(10);
  }
  /* USER CODE END StartRelayTask */
}

/* USER CODE BEGIN Header_StartCanReceive */
/**
 * @brief Function implementing the canReceiveTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanReceive */
void StartCanReceive(void *argument) {
  /* USER CODE BEGIN StartCanReceive */
  UNUSED(argument);
  CAN_RxHeaderTypeDef localRxHeader = {0};
  uint8_t ret[64] = {0};
  /* Infinite loop */
  for (;;) {
    if (osMessageQueueGet(canRxDataHandle, &localRxHeader.StdId, 0,
                          osWaitForever) == osOK) {
      if (osMessageQueueGet(canRxDataHandle, &localRxHeader.DLC, 0, 0) !=
          osOK) {
        Error_Handler();
      }
      for (uint8_t i = 0; i < mapDlcToBytes(localRxHeader.DLC); i++) {
        if (osMessageQueueGet(canRxDataHandle, &ret[i], 0, 0) != osOK) {
          Error_Handler();
        }
      }
      switch (localRxHeader.StdId) {
      case FDCAN_H2ALARM_ID:
        // H2 ALARM
        if (ret[0] == 1) {
          lock_state = true;
          log_info("H2 Alarm received!");
        }
        break;
      case FDCAN_SYNCLED_ID:
        // CAN SYNC LED
        /*if (ret[0] == 1) {*/
        /*  htim2.Instance->CCR1 = SET_BRIGHTNESS(20);*/
        /*} else {*/
        /*  htim2.Instance->CCR1 = SET_BRIGHTNESS(0);*/
        /*}*/
        break;
      case FDCAN_UPDATESTATE_ID:
        relay_state = ret[0];
        log_info("FDCAN_UPDATESTATE_ID Received: 0x%x", relay_state);
      default:
        break;
      }
    }
  }
  osDelay(1);
  /* USER CODE END StartCanReceive */
}

/* USER CODE BEGIN Header_StartAdcTask */
/**
 * @brief Function implementing the adcTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartAdcTask */
void StartAdcTask(void *argument) {
  /* USER CODE BEGIN StartAdcTask */
  const float ADC_VOLT_REF = 3.3F / 4096.0F;
  const float CURR_TRANSFER = 0.667F;
  const float VOLT_TRANSFER = 0.107F;
  const float VOLT_TO_CURR_UNI = 12.5F; // A/V
  const float VOLT_TO_CURR_BI = 25.0F;  // A/V
  /* Infinite loop */
  adc1Results[0] = adc1Results[1] = adc1Results[2] = 0;
  adc2Results[0] = adc2Results[1] = adc2Results[2] = 0;
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1Results, 3);
  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc2Results, 3);

  for (;;) {
    relay_board_data.fc_volt = adc1Results[0] * ADC_VOLT_REF / VOLT_TRANSFER;
    relay_board_data.cap_volt = adc1Results[1] * ADC_VOLT_REF / VOLT_TRANSFER;
    relay_board_data.fc_curr =
        ((adc1Results[2] * ADC_VOLT_REF / CURR_TRANSFER) - 0.5) *
        VOLT_TO_CURR_UNI;
    relay_board_data.mtr_curr =
        ((adc2Results[0] * ADC_VOLT_REF / CURR_TRANSFER) - 0.5) *
        VOLT_TO_CURR_UNI;
    relay_board_data.cap_curr =
        ((adc2Results[1] * ADC_VOLT_REF / CURR_TRANSFER) - 2.5) *
        VOLT_TO_CURR_BI;
    relay_board_data.mtr_volt = adc2Results[2] * ADC_VOLT_REF / VOLT_TRANSFER;
    osDelay(10);
  }
  /* USER CODE END StartAdcTask */
}

/* USER CODE BEGIN Header_StartIndicateLedTask */
/**
 * @brief Function implementing the indicatorLedTas thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartIndicateLedTask */
void StartIndicateLedTask(void *argument) {
  /* USER CODE BEGIN StartIndicateLedTask */
  /* Infinite loop */
  for (;;) {
    osDelay(1);
  }
  /* USER CODE END StartIndicateLedTask */
}

/* USER CODE BEGIN Header_StartCanDataTask */
/**
 * @brief Function implementing the canSendTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanDataTask */
void StartCanDataTask(void *argument) {
  /* USER CODE BEGIN StartCanDataTask */
  UNUSED(argument);
  CAN_TxHeaderTypeDef localTxHeader;
  uint32_t localTxMailbox;
  const uint8_t msg_delay = 10;
  uint8_t can_sync_led = 0;
  uint32_t sync_led_last = 0;
  uint32_t sync_led_this;

  localTxHeader.IDE = CAN_ID_STD;
  localTxHeader.RTR = CAN_RTR_DATA;
  /* Infinite loop */
  for (;;) {
    // Sync LEDs
    sync_led_this = osKernelGetTickCount();
    if (sync_led_this - sync_led_last > 500) {
      sync_led_last = sync_led_this;
      localTxHeader.StdId = FDCAN_SYNCLED_ID;
      localTxHeader.DLC = 1;
      can_sync_led = (can_sync_led == 0) ? 1 : 0;
      if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 0) {
        if (HAL_CAN_AddTxMessage(&hcan1, &localTxHeader, &can_sync_led,
                                 &localTxMailbox) != HAL_OK) {
          Error_Handler();
        }
        if (can_sync_led == 1) {
          HAL_GPIO_WritePin(DSCHRGE_LED_GPIO_Port, DSCHRGE_LED_Pin,
                            GPIO_PIN_SET);
        } else {
          HAL_GPIO_WritePin(DSCHRGE_LED_GPIO_Port, DSCHRGE_LED_Pin,
                            GPIO_PIN_RESET);
        }
      } else {
        log_warn("Tx Buffer Full");
      }
    }
    osDelay(1);
  }
  /* USER CODE END StartCanDataTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
