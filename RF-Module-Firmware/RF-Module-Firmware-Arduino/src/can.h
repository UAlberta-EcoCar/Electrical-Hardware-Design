#pragma once
#include "packets.h"
#include <Arduino.h>
#include <stm32l4xx.h>
#include <stm32l4xx_hal_can.h>
#include <stm32l4xx_hal_gpio.h>
#include <stm32l4xx_hal_rcc.h>
extern CAN_TxHeaderTypeDef TxHeader;

extern uint32_t TxMailbox;
/* USER CODE END 0 */

extern CAN_HandleTypeDef hcan1;

extern CAN_RxHeaderTypeDef RxHeader;
extern CAN_RxHeaderTypeDef RxHeader_Reserved;
extern uint8_t TxData[8];
extern uint8_t RxData[8];
// uint8_t message2[1000] = { 0 };

extern uint8_t RxData_Reserved[8];

void MX_CAN1_Init(void);
void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle);
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle);
int CAN_Transmit(uint32_t _device_address, uint32_t *_buffer_pointer,
                 int _buffer_length, uint32_t _RTR);
void CAN_Initialize();