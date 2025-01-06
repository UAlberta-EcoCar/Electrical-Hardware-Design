/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    can.c
 * @brief   This file provides code for the configuration
 *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
CAN_TxHeaderTypeDef TxHeader;

//uint32_t TxMailbox;
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 5;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
//	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
//		printf("[!SYSTEM ERROR]CAN Initialization Error At CAN Start");
//		Error_Handler();
//	}
  /* USER CODE END CAN1_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void CAN_Initialize() {
//	MX_CAN1_Init();
	/* USER CODE BEGIN 2 */
	CAN_FilterTypeDef high_priority_filter;
	high_priority_filter.FilterIdHigh = 0x100 << 5;
	high_priority_filter.FilterMaskIdHigh = 0x000 << 5;
	high_priority_filter.FilterIdLow = 0x0000;
	high_priority_filter.FilterMaskIdLow = 0x0000;
	high_priority_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	high_priority_filter.FilterBank = 0;
	high_priority_filter.FilterMode = CAN_FILTERMODE_IDMASK;
	high_priority_filter.FilterScale = CAN_FILTERSCALE_32BIT;
	high_priority_filter.FilterActivation = CAN_FILTER_ENABLE;

	if (HAL_CAN_ConfigFilter(&hcan1, &high_priority_filter) != HAL_OK) {
		/* Filter configuration Error */
		Error_Handler();
	}

//	CAN_FilterTypeDef low_priority_filter;
//	low_priority_filter.FilterIdHigh = 0x200 << 5;
//	low_priority_filter.FilterMaskIdHigh = 0x700 << 5;
//	low_priority_filter.FilterIdLow = 0x0000;
//	low_priority_filter.FilterMaskIdLow = 0x0000;
//	low_priority_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
//	low_priority_filter.FilterBank = 0;
//	low_priority_filter.FilterMode = CAN_FILTERMODE_IDMASK;
//	low_priority_filter.FilterScale = CAN_FILTERSCALE_32BIT;
//	low_priority_filter.FilterActivation = CAN_FILTER_ENABLE;
////
//	if (HAL_CAN_ConfigFilter(&hcan1, &low_priority_filter) != HAL_OK) {
//		/* Filter configuration Error */
//		Error_Handler();
//	}

	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
		printf("[!SYSTEM ERROR]CAN Initialization Error At CAN Start");
		Error_Handler();
	}
//
//	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING)
//			!= HAL_OK) {
//		printf(
//				"[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT MESSAGE PENDING RX FIFO 0");
//		Error_Handler();
//	}
//	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_MSG_PENDING)
//			!= HAL_OK) {
//		printf(
//				"[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT MESSAGE PENDING RX FIFO 1");
//		Error_Handler();
//	}
//
//	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_FULL) != HAL_OK) {
//		printf(
//				"[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT RX FIFO 0 FULL");
//		Error_Handler();
//	}
//	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_FULL) != HAL_OK) {
//		printf(
//				"[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT RX FIFO 1 FULL");
//		Error_Handler();
//	}

//	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_ERROR))
//	!= HAL_OK) {
//		Error_Handler();
//	}
//	TxHeader.RTR = CAN_RTR_DATA;
//	TxHeader.IDE = CAN_ID_STD;
//	TxHeader.DLC = 8;
//	TxHeader.TransmitGlobalTime = DISABLE;
}

///**
// * Send 8 bytes at a time, with standard id size.
// */
//int CAN_Transmit(uint32_t _device_address, uint32_t *_buffer_pointer,
//		int _buffer_length, uint32_t _RTR) {
//
//	TxHeader.StdId = _device_address;
//	TxHeader.RTR = _RTR;
//	if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, _buffer_pointer, &TxMailbox)
//			!= HAL_OK) {
//		printf(
//				"Can transmission error on packet id: %hu and containing data: %u\r\n",
//				_device_address, _buffer_pointer);
//		Error_Handler();
//	}
//
//	return 1;
//}
HAL_StatusTypeDef HAL_CAN_SafeAddTxMessage(uint8_t *msg, uint32_t msg_id,
		uint32_t msg_length, uint32_t *TxMailbox, uint32_t rtr) {
	uint32_t fc_tick;
	HAL_StatusTypeDef hal_stat;
	CAN_TxHeaderTypeDef _TxHeader;

	// These will never change
	_TxHeader.IDE = CAN_ID_STD;
	_TxHeader.ExtId = 0;
	_TxHeader.TransmitGlobalTime = DISABLE;

	// User will give us this information
	_TxHeader.RTR = rtr;
	_TxHeader.StdId = msg_id;
	_TxHeader.DLC = msg_length;

	// Start a timer to check timeout conditions
	fc_tick = HAL_GetTick();

	/* Try to add a Tx message. Returns HAL_ERROR if there are no avail
	 * mailboxes or if the peripheral is not initialized. */
	do {
		hal_stat = HAL_CAN_AddTxMessage(&hcan1, &_TxHeader, msg, TxMailbox);
	} while (hal_stat != HAL_OK && (HAL_GetTick() - fc_tick < 500));

	return hal_stat;
}
/* USER CODE END 1 */
