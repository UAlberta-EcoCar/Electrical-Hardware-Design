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
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "rf-rfm95.h"
#include "debug-log.h"
#include "spi.h"
#include "lucy-can-ids.h"
#include "can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.stack_size = 128 * 12, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for CanRxTask */
osThreadId_t CanRxTaskHandle;
uint32_t CanRxTaskBuffer[128];
osStaticThreadDef_t CanRxTaskControlBlock;
const osThreadAttr_t CanRxTask_attributes = { .name = "CanRxTask", .cb_mem =
		&CanRxTaskControlBlock, .cb_size = sizeof(CanRxTaskControlBlock),
		.stack_mem = &CanRxTaskBuffer[0], .stack_size = sizeof(CanRxTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal1, };
/* Definitions for CanRtrTask */
osThreadId_t CanRtrTaskHandle;
uint32_t CanRtrTaskBuffer[128];
osStaticThreadDef_t CanRtrTaskControlBlock;
const osThreadAttr_t CanRtrTask_attributes = { .name = "CanRtrTask", .cb_mem =
		&CanRtrTaskControlBlock, .cb_size = sizeof(CanRtrTaskControlBlock),
		.stack_mem = &CanRtrTaskBuffer[0], .stack_size =
				sizeof(CanRtrTaskBuffer), .priority =
				(osPriority_t) osPriorityNormal2, };
/* Definitions for canRxDataQueue */
osMessageQueueId_t canRxDataQueueHandle;
uint8_t canRxDataQueueBuffer[128 * sizeof(uint32_t)];
osStaticMessageQDef_t canRxDataQueueControlBlock;
const osMessageQueueAttr_t canRxDataQueue_attributes = { .name =
		"canRxDataQueue", .cb_mem = &canRxDataQueueControlBlock, .cb_size =
		sizeof(canRxDataQueueControlBlock), .mq_mem = &canRxDataQueueBuffer,
		.mq_size = sizeof(canRxDataQueueBuffer) };
/* Definitions for canRxRtrQueue */
osMessageQueueId_t canRxRtrQueueHandle;
uint8_t canRtrDataQueueBuffer[128 * sizeof(uint32_t)];
osStaticMessageQDef_t canRtrDataQueueControlBlock;
const osMessageQueueAttr_t canRxRtrQueue_attributes =
		{ .name = "canRxRtrQueue", .cb_mem = &canRtrDataQueueControlBlock,
				.cb_size = sizeof(canRtrDataQueueControlBlock), .mq_mem =
						&canRtrDataQueueBuffer, .mq_size =
						sizeof(canRtrDataQueueBuffer) };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

typedef struct {
	union {
		struct {
			can_lucy_fc_vi packet_fc;
			can_lucy_motor_vi packet_mtr;
		};
		uint8_t packet_raw[16];
	};
} rf_packet_test;

rf_packet_test test_packet = { 0 };

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartCanRxTask(void *argument);
void StartCanRtrTask(void *argument);

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
	/* creation of canRxDataQueue */
//  canRxDataQueueHandle = osMessageQueueNew (128, sizeof(uint32_t), &canRxDataQueue_attributes);
	/* creation of canRxRtrQueue */
//  canRxRtrQueueHandle = osMessageQueueNew (128, sizeof(uint32_t), &canRxRtrQueue_attributes);
	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* creation of CanRxTask */
	CanRxTaskHandle = osThreadNew(StartCanRxTask, NULL, &CanRxTask_attributes);

	/* creation of CanRtrTask */
	CanRtrTaskHandle = osThreadNew(StartCanRtrTask, NULL,
			&CanRtrTask_attributes);
	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
	/* USER CODE BEGIN StartDefaultTask */
	//	rfm95_init();
	//	uint8_t version = 0, temp = 0;
	rf_handle_t rfm95 = { .rf_nreset_port = RF_NRST_GPIO_Port, .rf_nreset_pin =
	RF_NRST_Pin, .rf_nss_port = SPI1_NSS_GPIO_Port, .rf_nss_pin =
	SPI1_NSS_Pin, .rf_spi_handle = &hspi1, .rf_delay_func = osDelay,
			.rf_spi_timeout = HAL_MAX_DELAY, .rf_carrier_frequency = 868000000 };

	rf_initialize_radio(&rfm95);
	rf_set_tx_power(&rfm95, 5);

	rf_set_frequency(&rfm95, 868000000);
	uint8_t testdata = 347u;

	//	rf_set_op_mode(&rfm95, RF_OP_MODE_RX_SINGLE);

	//	rf_listen_implicit(&rfm95, 1);
	//	rf_listen(&rfm95);

	const char test[] = "Hello.78\0";

	uint8_t rec_legth = 0;

	/* Infinite loop */
	for (;;) {
		//osDelay(100);
		log_info("Sending message %s", test);
		HAL_GPIO_WritePin(LED_D1_GPIO_Port, LED_D1_Pin, GPIO_PIN_SET);
		//rf_initialize_radio(&rfm95);
		rf_send(&rfm95, test_packet.packet_raw, 16);
		testdata += 1;
		HAL_GPIO_WritePin(LED_D1_GPIO_Port, LED_D1_Pin, GPIO_PIN_RESET);
		osDelay(100);
		// reciever
//		HAL_GPIO_WritePin(LED_D1_GPIO_Port, LED_D1_Pin, GPIO_PIN_SET);
//		osDelay(100);
//
//		testdata = 0;
//
//		rec_legth = 0;
//
//		rf_recieve_single(&rfm95, &rec_legth);
//		if (rec_legth > 0) {
//			char rec_data[10] = { 0 };
//			rf_read_packet(&rfm95, rec_legth, rec_data);
//
//			log_info("Byte: %s", rec_data);
//		}
//		HAL_GPIO_WritePin(LED_D1_GPIO_Port, LED_D1_Pin, GPIO_PIN_RESET);
//		osDelay(100);
	}
	/* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartCanRxTask */
uint32_t TxMailbox;
/**
 * @brief Function implementing the CanRxTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanRxTask */
void StartCanRxTask(void *argument) {
	/* USER CODE BEGIN StartCanRxTask */

	/* Infinite loop */
	for (;;) {

//		CAN_Transmit(CAN_LUCY_FC_VI, NULL, NULL, CAN_RTR_REMOTE);
		if (!HAL_CAN_SafeAddTxMessage(NULL, CAN_LUCY_FC_VI, 0, &TxMailbox,
		CAN_RTR_REMOTE)) {
			log_info("Sending CAN REQUEST");
		} else {
			log_info("CAN TIMED OUT");
		}

		if (!HAL_CAN_SafeAddTxMessage(NULL, CAN_LUCY_MOTOR_VI, 0, &TxMailbox,
		CAN_RTR_REMOTE)) {
			log_info("Sending CAN REQUEST");
		} else {
			log_info("CAN TIMED OUT");
		}
		osDelay(1000);
	}
	/* USER CODE END StartCanRxTask */
}

/* USER CODE BEGIN Header_StartCanRtrTask */
/**
 * @brief Function implementing the CanRtrTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanRtrTask */
void StartCanRtrTask(void *argument) {
	/* USER CODE BEGIN StartCanRtrTask */
	CAN_Initialize();
	/* Infinite loop */
	uint8_t RxData[8];
	HAL_StatusTypeDef hal_stat;
	CAN_RxHeaderTypeDef RxHeader = { 0 };
	for (;;) {
//		HAL_CAN_GetRxFifoFillLevel(&hcan1, RxFifo);
		hal_stat = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader,
				RxData);
		if (!hal_stat) {
			log_info("Got message");

			log_info("0x%x %d RTR: %d", RxHeader.StdId, RxHeader.DLC,
					RxHeader.RTR);

			if (RxHeader.StdId == CAN_LUCY_FC_VI && RxHeader.DLC != 0) {

				memcpy(test_packet.packet_fc.can_raw_lucy_fc_vi, RxData, 8);
				log_info("Copying 103");
			}

			if (RxHeader.StdId == CAN_LUCY_MOTOR_VI && RxHeader.DLC != 0) {

				memcpy(test_packet.packet_mtr.can_raw_lucy_motor_vi, RxData, 8);
				log_info("Copying 102");
			}

		}
		osDelay(5);
	}
	/* USER CODE END StartCanRtrTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

