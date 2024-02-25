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
#include "rfm95.h"
#include "rng.h"
#include <string.h>
#include "spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
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
CAN_RxHeaderTypeDef RxHeader_Reserved;
uint8_t TxData[8] = { 0 };
uint8_t RxData[8] = { 0 };
//uint8_t message2[1000] = { 0 };

uint8_t RxData_Reserved[8] = { 0 };

rfm95_handle_t rfm95_handle = { .spi_handle = &hspi1, .nss_port =
SPI_NSS_GPIO_GPIO_Port, .nss_pin = SPI_NSS_GPIO_Pin, .nrst_port =
RF_NRST_GPIO_Port, .nrst_pin = RF_NRST_Pin, .device_address = { 0x00, 0x00,
		0x00, 0x00 }, .application_session_key = { 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		.network_session_key = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		.receive_mode = RFM95_RECEIVE_MODE_RX1_ONLY, .random_int = 0 };

/* USER CODE END Variables */
/* Definitions for JERMAMainThread */
osThreadId_t JERMAMainThreadHandle;
const osThreadAttr_t JERMAMainThread_attributes = { .name = "JERMAMainThread",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityRealtime, };
/* Definitions for CANThread */
osThreadId_t CANThreadHandle;
const osThreadAttr_t CANThread_attributes = { .name = "CANThread", .stack_size =
		256 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for RFThread */
osThreadId_t RFThreadHandle;
const osThreadAttr_t RFThread_attributes = { .name = "RFThread", .stack_size =
		256 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for CAN_Transmission_Queue */
osMessageQueueId_t CAN_Transmission_QueueHandle;
const osMessageQueueAttr_t CAN_Transmission_Queue_attributes = { .name =
		"CAN_Transmission_Queue" };
/* Definitions for CAN_HIGHPrio_Reci_Queue */
osMessageQueueId_t CAN_HIGHPrio_Reci_QueueHandle;
const osMessageQueueAttr_t CAN_HIGHPrio_Reci_Queue_attributes = { .name =
		"CAN_HIGHPrio_Reci_Queue" };
/* Definitions for CAN_LOWPrio_Reci_Queue */
osMessageQueueId_t CAN_LOWPrio_Reci_QueueHandle;
const osMessageQueueAttr_t CAN_LOWPrio_Reci_Queue_attributes = { .name =
		"CAN_LOWPrio_Reci_Queue" };
/* Definitions for CAN_JERMA_GAS_LEAK */
osMessageQueueId_t CAN_JERMA_GAS_LEAKHandle;
const osMessageQueueAttr_t CAN_JERMA_GAS_LEAK_attributes = { .name =
		"CAN_JERMA_GAS_LEAK" };
/* Definitions for CAN_Transmission_Complete */
osSemaphoreId_t CAN_Transmission_CompleteHandle;
osStaticSemaphoreDef_t CAN_Transmission_CompleteControlBlock;
const osSemaphoreAttr_t CAN_Transmission_Complete_attributes = { .name =
		"CAN_Transmission_Complete", .cb_mem =
		&CAN_Transmission_CompleteControlBlock, .cb_size =
		sizeof(CAN_Transmission_CompleteControlBlock), };
/* Definitions for CAN_Recieve_Complete */
osSemaphoreId_t CAN_Recieve_CompleteHandle;
osStaticSemaphoreDef_t CAN_Recieve_CompleteControlBlock;
const osSemaphoreAttr_t CAN_Recieve_Complete_attributes = { .name =
		"CAN_Recieve_Complete", .cb_mem = &CAN_Recieve_CompleteControlBlock,
		.cb_size = sizeof(CAN_Recieve_CompleteControlBlock), };
/* Definitions for RELEASE_JERMA_GAS */
osSemaphoreId_t RELEASE_JERMA_GASHandle;
const osSemaphoreAttr_t RELEASE_JERMA_GAS_attributes = { .name =
		"RELEASE_JERMA_GAS" };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void rfm_precision_sleep(uint32_t target);

/* USER CODE END FunctionPrototypes */

void startMainThread(void *argument);
void startCANThread(void *argument);
void startRFThread(void *argument);

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
	CAN_Transmission_CompleteHandle = osSemaphoreNew(1, 0,
			&CAN_Transmission_Complete_attributes);

	/* creation of CAN_Recieve_Complete */
	CAN_Recieve_CompleteHandle = osSemaphoreNew(1, 0,
			&CAN_Recieve_Complete_attributes);

	/* creation of RELEASE_JERMA_GAS */
	RELEASE_JERMA_GASHandle = osSemaphoreNew(1, 0,
			&RELEASE_JERMA_GAS_attributes);

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* creation of CAN_Transmission_Queue */
	CAN_Transmission_QueueHandle = osMessageQueueNew(16, 8,
			&CAN_Transmission_Queue_attributes);

	/* creation of CAN_HIGHPrio_Reci_Queue */
	CAN_HIGHPrio_Reci_QueueHandle = osMessageQueueNew(16, 8,
			&CAN_HIGHPrio_Reci_Queue_attributes);

	/* creation of CAN_LOWPrio_Reci_Queue */
	CAN_LOWPrio_Reci_QueueHandle = osMessageQueueNew(16, sizeof(IntrimPacket),
			&CAN_LOWPrio_Reci_Queue_attributes);

	/* creation of CAN_JERMA_GAS_LEAK */
	CAN_JERMA_GAS_LEAKHandle = osMessageQueueNew(16, 8,
			&CAN_JERMA_GAS_LEAK_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of JERMAMainThread */
	JERMAMainThreadHandle = osThreadNew(startMainThread, NULL,
			&JERMAMainThread_attributes);

	/* creation of CANThread */
	CANThreadHandle = osThreadNew(startCANThread, NULL, &CANThread_attributes);

	/* creation of RFThread */
	RFThreadHandle = osThreadNew(startRFThread, NULL, &RFThread_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_startMainThread */
/**
 * @brief  Function implementing the MainThread thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_startMainThread */
void startMainThread(void *argument) {
	/* USER CODE BEGIN startMainThread */
	/* Infinite loop */
	for (;;) {
		//osSemaphoreWait(RELEASE_JERMA_GASHandle, 0);
		osDelay(1);
	}
	/* USER CODE END startMainThread */
}

/* USER CODE BEGIN Header_startCANThread */
/**
 * @brief Function implementing the CANThread thread.
 * @param argument: Not used
 * @retval None
 */

/* USER CODE END Header_startCANThread */
void startCANThread(void *argument) {
	/* USER CODE BEGIN startCANThread */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	//uint8_t message[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	uint8_t message2[8] = { 0 };
	struct Data  {
		float h2;
		float temp;
	} __attribute__((packed)) reciever;

	IntrimPacket intrim;

	/* Infinite loop */
	for (;;) {

		osMessageQueueGet(CAN_LOWPrio_Reci_QueueHandle, &intrim, NULL,
		osWaitForever);
		htim1.Instance->CCR2 = 5;
//		printf("Got message: ");
//		for (int i = 0; i < 8; i++)
//			printf("%u", data[i]);

		printf("\r\n");

		reciever = *((struct Data*) intrim.data);
		printf("Packet ID: %hx ", (uint8_t) intrim.id);
		printf("H2: %f temp: %f", reciever.h2, reciever.temp);
		//CAN_Transmit(0x101, &data, sizeof(data));
		osDelay(100);
		htim1.Instance->CCR2 = 0;
		osDelay(100);

	}
	/* USER CODE END startCANThread */
}

/* USER CODE BEGIN Header_startRFThread */
/**
 * @brief Function implementing the RFThread thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startRFThread */
void startRFThread(void *argument) {
	/* USER CODE BEGIN startRFThread */
	struct Data {
		float h2;
		float temp;
	} test_message;
	test_message.h2 = 1;
	test_message.temp = 1;

	rfm95_init(&rfm95_handle);

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	/* Infinite loop */
	for (;;) {
//		HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*) &message3.h2);
//		HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*) &message3.temp);
//		message3.h2 = message3.h2 / 1000 + 1;
		htim2.Instance->CCR2 = 5;
//		rfm95_send_receive_cycle(&rfm95_handle, &test_message,
//				sizeof(test_message));
//		CAN_Transmit(0x123, (uint32_t*) &message3, sizeof(message3),
//		CAN_RTR_DATA);
//		//osMessageQueuePut(CAN_Transmission_QueueHandle, &message, 0,
//		//osWaitForever);
		osDelay(1000);
		htim2.Instance->CCR2 = 0;

		osDelay(100);
	}
	/* USER CODE END startRFThread */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
volatile IntrimPacket packet; // Declared as a temporary variable outside ISR as this is being allocated and re allocated,
// therefore to save ISR time declaring it out side. Also volatile since only used in ISR.

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

	// Just check if there is atleast 1 spot open
	// If we dont and call get message we will loose that message since it wont fit and the ISR will exit.
	if (osMessageQueueGetSpace(CAN_LOWPrio_Reci_QueueHandle) >= 1) {
		if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData)
				!= HAL_OK) {
			Error_Handler();
		}
		// Mask the packet ID out.
		packet.id = RxHeader.StdId & 0x0FF;

		memcpy(packet.data, RxData, 8); // this is always 8 bytes therefore there
		// is pretty much no added iteration time. The compiler will optimize the loop away.

		osMessageQueuePut(CAN_LOWPrio_Reci_QueueHandle, &packet, 0, 0);
	}

}

/**
 * EMERGENCY TRANSMISSION RESERVED ID: 0x000
 * Packet manifest:
 *
 */
struct Hydrogen_Board_Leak_Emergency_Transmission {
	float h2_concentration;
};

/** HIGH PRIORITY ONLY TALKS TO H2 Board. */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader_Reserved,
			RxData_Reserved) != HAL_OK) {
		Error_Handler();
	}
	if (RxHeader_Reserved.StdId == 0x000) {
		// Emergency Release JERMA. GAS GAS GAS
		printf("[!HYDROGEN BOARD]: LEAK!!! Releasing JERMAAA");
		osSemaphoreRelease(RELEASE_JERMA_GASHandle);
		osMessageQueuePut(CAN_JERMA_GAS_LEAKHandle, RxData_Reserved, 0,
		osWaitForever);
	}

}

//void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
//	osSemaphoreRelease(CAN_Transmission_CompleteHandle);
//}
//
//void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
//	osSemaphoreRelease(CAN_Transmission_CompleteHandle);
//}
//
//void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
//	osSemaphoreRelease(CAN_Transmission_CompleteHandle);
//}
//
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//	if (GPIO_Pin == GPIO_PIN_3) {
//		can_spam = !can_spam;
//	}
//}

void rfm_precision_sleep(uint32_t target) {
	osDelay(target);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == DIO0_Pin) {
		rfm95_on_interrupt(&rfm95_handle, RFM95_INTERRUPT_DIO0);
	} else if (GPIO_Pin == DIO1_Pin) {
		rfm95_on_interrupt(&rfm95_handle, RFM95_INTERRUPT_DIO1);
	} else if (GPIO_Pin == DIO5_Pin) {
		rfm95_on_interrupt(&rfm95_handle, RFM95_INTERRUPT_DIO5);
	}
}

/* USER CODE END Application */

