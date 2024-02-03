/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
#include "can.h"
#include <stdio.h>
#include <rfm95.h>

#include "spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
volatile int can_spam = 1;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
CAN_RxHeaderTypeDef RxHeader;
uint8_t TxData[8] = { 0 };
uint8_t RxData[8] = { 0 };
/* USER CODE END Variables */
/* Definitions for MainThread */
osThreadId_t MainThreadHandle;
const osThreadAttr_t MainThread_attributes = {
  .name = "MainThread",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for CAN_TX_Thread */
osThreadId_t CAN_TX_ThreadHandle;
const osThreadAttr_t CAN_TX_Thread_attributes = {
  .name = "CAN_TX_Thread",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CAN_RX_Thread */
osThreadId_t CAN_RX_ThreadHandle;
const osThreadAttr_t CAN_RX_Thread_attributes = {
  .name = "CAN_RX_Thread",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for RF_RXTX */
osThreadId_t RF_RXTXHandle;
const osThreadAttr_t RF_RXTX_attributes = {
  .name = "RF_RXTX",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal1,
};
/* Definitions for CAN_Transmission_Queue */
osMessageQueueId_t CAN_Transmission_QueueHandle;
const osMessageQueueAttr_t CAN_Transmission_Queue_attributes = {
  .name = "CAN_Transmission_Queue"
};
/* Definitions for CAN_Transmission_Complete */
osSemaphoreId_t CAN_Transmission_CompleteHandle;
const osSemaphoreAttr_t CAN_Transmission_Complete_attributes = {
  .name = "CAN_Transmission_Complete"
};
/* Definitions for CAN_Recieve_Complete */
osSemaphoreId_t CAN_Recieve_CompleteHandle;
const osSemaphoreAttr_t CAN_Recieve_Complete_attributes = {
  .name = "CAN_Recieve_Complete"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/* USER CODE END FunctionPrototypes */

void Start_MainT(void *argument);
void StartCAN_TX_Thread(void *argument);
void StartCAN_RX_Thread(void *argument);
void StartRF_RXTX(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void) {

}

__weak unsigned long getRunTimeCounterValue(void) {
	return 0;
}
void __attribute__ ((noinline)) breakpoint() {
	__asm("NOP");
	// <---- set a hardware breakpoint here!
	// hello, please Step Out to go to caller location (ex: press Shift-F11)
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	CAN_Initialize();

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of CAN_Transmission_Complete */
  CAN_Transmission_CompleteHandle = osSemaphoreNew(1, 0, &CAN_Transmission_Complete_attributes);

  /* creation of CAN_Recieve_Complete */
  CAN_Recieve_CompleteHandle = osSemaphoreNew(1, 0, &CAN_Recieve_Complete_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of CAN_Transmission_Queue */
  CAN_Transmission_QueueHandle = osMessageQueueNew (1, 8, &CAN_Transmission_Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of MainThread */
  MainThreadHandle = osThreadNew(Start_MainT, NULL, &MainThread_attributes);

  /* creation of CAN_TX_Thread */
  CAN_TX_ThreadHandle = osThreadNew(StartCAN_TX_Thread, NULL, &CAN_TX_Thread_attributes);

  /* creation of CAN_RX_Thread */
  CAN_RX_ThreadHandle = osThreadNew(StartCAN_RX_Thread, NULL, &CAN_RX_Thread_attributes);

  /* creation of RF_RXTX */
  RF_RXTXHandle = osThreadNew(StartRF_RXTX, NULL, &RF_RXTX_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Start_MainT */
/**
 * @brief  Function implementing the MainThread thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_Start_MainT */
void Start_MainT(void *argument)
{
  /* USER CODE BEGIN Start_MainT */

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	float count = 0.12f;

	/* Infinite loop */
	for (;;) {
		//printf("Test %f \r\n", count++);

		osDelay(500);
		htim1.Instance->CCR3 = 5;
		osDelay(500);
		htim1.Instance->CCR3 = 0;
	}
  /* USER CODE END Start_MainT */
}

/* USER CODE BEGIN Header_StartCAN_TX_Thread */
/**
 * @brief Function implementing the CAN_TX_Thread thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCAN_TX_Thread */
void StartCAN_TX_Thread(void *argument)
{
  /* USER CODE BEGIN StartCAN_TX_Thread */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	/* Infinite loop */
	TxData[0] = 0xA;
	TxData[1] = 0xA;
	TxData[2] = 0xA;
	TxData[3] = 0xA;
	TxData[4] = 0xA;
	TxData[5] = 0xA;
	TxData[6] = 0xA;
	TxData[7] = 0xA;
	int a = 0;
	for (;;) {
		if (can_spam) {
			//printf("Waiting to transmit CAN Packet.\r\n");
			osMessageQueueGet(CAN_Transmission_QueueHandle, &TxData, NULL,
			osWaitForever);
			//printf("Transmitting CAN Packet.\r\n");
			CAN_Transmit(TxData, sizeof(TxData));
			htim1.Instance->CCR2 = 5;
//			if (osSemaphoreAcquire(CAN_Transmission_CompleteHandle,
//			osWaitForever) == osOK) {
//				printf("Transmission Complete of CAN Packet\r\n");
//			} else {
//				printf("Error Transmission");
//			}

			osDelay(100);
			htim1.Instance->CCR2 = 0;
			osDelay(100);
		}
		osDelay(100);
	}
  /* USER CODE END StartCAN_TX_Thread */
}

/* USER CODE BEGIN Header_StartCAN_RX_Thread */
/**
 * @brief Function implementing the CAN_RX_Thread thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCAN_RX_Thread */
void StartCAN_RX_Thread(void *argument)
{
  /* USER CODE BEGIN StartCAN_RX_Thread */
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	uint32_t count = 0;
	/* Infinite loop */
	for (;;) {
		//printf("Waiting for CAN Packet.\r\n");
		osSemaphoreAcquire(CAN_Recieve_CompleteHandle, osWaitForever);
		htim2.Instance->CCR2 = 5;
		osDelay(100);
//		printf("Packet: %04X %04X %04X %04X %04X %04X %04X %04X\r\n", RxData[0], RxData[1],
//				RxData[2], RxData[3], RxData[4], RxData[5], RxData[6],
//				RxData[7]);
		//printf("Packet Content: ");
//		int i;
//		for (i = 0; i < 8; i++) {
//			printf("%d ", RxData[i]);
//		}
		//printf("Received CAN Packet. Sending to Queue\r\n");
		osMessageQueuePut(CAN_Transmission_QueueHandle, &RxData, NULL,
		osWaitForever);
		htim2.Instance->CCR2 = 0;
		osDelay(100);
		//htim2.Instance->CCR2 = 0;
	}
  /* USER CODE END StartCAN_RX_Thread */
}

/* USER CODE BEGIN Header_StartRF_RXTX */
/**
 * @brief Function implementing the RF_RXTX thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartRF_RXTX */
void StartRF_RXTX(void *argument)
{
  /* USER CODE BEGIN StartRF_RXTX */

	// Create the handle for the RFM95 module.
	rfm95_handle_t rfm95_handle = { .spi_handle = &hspi1, .nss_port =
	GPIO_SPI_NSS_GPIO_Port, .nss_pin = GPIO_SPI_NSS_Pin, .nrst_port =
	RF_NRST_GPIO_Port, .nrst_pin = RF_NRST_Pin, .device_address = { 0x00, 0x00,
			0x00, 0x00 }, .application_session_key = { 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00 }, .network_session_key = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
			.receive_mode = RFM95_RECEIVE_MODE_NONE };

	// Initialise RFM95 module.
	if (!rfm95_init(&rfm95_handle)) {
		printf("RFM95 init failed\n\r");
	}
//
//	uint8_t[] data_packet = {
//		0x01, 0x02, 0x03, 0x4
//	};
//
//	if (!rfm95_send_receive_cycle(&rfm95_handle, data_packet,
//			sizeof(data_packet))) {
//		printf("RFM95 send failed\n\r");
//	} else {
//		printf("RFM95 send success\n\r");
//	}

	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
  /* USER CODE END StartRF_RXTX */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
		Error_Handler();
	}
	osSemaphoreRelease(CAN_Recieve_CompleteHandle);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
	osSemaphoreRelease(CAN_Transmission_CompleteHandle);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
	osSemaphoreRelease(CAN_Transmission_CompleteHandle);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
	osSemaphoreRelease(CAN_Transmission_CompleteHandle);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_3) {
		can_spam = !can_spam;
	}
}

/* USER CODE END Application */

