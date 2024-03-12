#include "can.h"

CAN_TxHeaderTypeDef TxHeader;

uint32_t TxMailbox;
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

CAN_RxHeaderTypeDef RxHeader;
CAN_RxHeaderTypeDef RxHeader_Reserved;
uint8_t TxData[8] = {0};
uint8_t RxData[8] = {0};
// uint8_t message2[1000] = { 0 };

uint8_t RxData_Reserved[8] = {0};

void MX_CAN1_Init(void)
{

    /* USER CODE BEGIN CAN1_Init 0 */

    /* USER CODE END CAN1_Init 0 */

    /* USER CODE BEGIN CAN1_Init 1 */

    /* USER CODE END CAN1_Init 1 */
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 16;
    hcan1.Init.Mode = CAN_MODE_LOOPBACK;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_2TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
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

    /* USER CODE END CAN1_Init 2 */
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (canHandle->Instance == CAN1)
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
        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* CAN1 interrupt Init */
        // HAL_NVIC_SetPriority(CAN1_TX_IRQn, 5, 0);
        // HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
        // HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
        // HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
        /* USER CODE BEGIN CAN1_MspInit 1 */

        /* USER CODE END CAN1_MspInit 1 */
    }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle)
{

    if (canHandle->Instance == CAN1)
    {
        /* USER CODE BEGIN CAN1_MspDeInit 0 */

        /* USER CODE END CAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();

        /**CAN1 GPIO Configuration
        PA11     ------> CAN1_RX
        PA12     ------> CAN1_TX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

        /* CAN1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
        /* USER CODE BEGIN CAN1_MspDeInit 1 */

        /* USER CODE END CAN1_MspDeInit 1 */
    }
}

int CAN_Transmit(uint32_t _device_address, uint32_t *_buffer_pointer,
                 int _buffer_length, uint32_t _RTR)
{

    TxHeader.StdId = _device_address;
    TxHeader.RTR = _RTR;
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, (uint8_t *)_buffer_pointer, &TxMailbox) != HAL_OK)
    {
        printf(
            "Can transmission error on packet id: %hu and containing data: %u\r\n",
            _device_address, _buffer_pointer);
        Error_Handler();
    }

    return 1;
}



void CAN_Initialize()
{
    MX_CAN1_Init();
    /* USER CODE BEGIN 2 */
    CAN_FilterTypeDef high_priority_filter;
    high_priority_filter.FilterIdHigh = 0x100 << 5;
    high_priority_filter.FilterMaskIdHigh = 0x700 << 5;
    high_priority_filter.FilterIdLow = 0x0000;
    high_priority_filter.FilterMaskIdLow = 0x0000;
    high_priority_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    high_priority_filter.FilterBank = 0;
    high_priority_filter.FilterMode = CAN_FILTERMODE_IDMASK;
    high_priority_filter.FilterScale = CAN_FILTERSCALE_32BIT;
    high_priority_filter.FilterActivation = CAN_FILTER_ENABLE;

    if (HAL_CAN_ConfigFilter(&hcan1, &high_priority_filter) != HAL_OK)
    {
        /* Filter configuration Error */
        Error_Handler();
    }

    CAN_FilterTypeDef low_priority_filter;
    low_priority_filter.FilterIdHigh = 0x100 << 5;
    low_priority_filter.FilterMaskIdHigh = 0x700 << 5;
    low_priority_filter.FilterIdLow = 0x0000;
    low_priority_filter.FilterMaskIdLow = 0x0000;
    low_priority_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    low_priority_filter.FilterBank = 0;
    low_priority_filter.FilterMode = CAN_FILTERMODE_IDMASK;
    low_priority_filter.FilterScale = CAN_FILTERSCALE_32BIT;
    low_priority_filter.FilterActivation = CAN_FILTER_ENABLE;

    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        // printf("[!SYSTEM ERROR]CAN Initialization Error At CAN Start");
        Error_Handler();
    }

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        // printf(
        //     "[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT MESSAGE PENDING RX FIFO 0");
        Error_Handler();
    }

    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;
}