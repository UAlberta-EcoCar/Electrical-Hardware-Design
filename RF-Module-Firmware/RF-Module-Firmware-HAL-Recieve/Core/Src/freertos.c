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
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
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
		.stack_size = 512 * 4, .priority = (osPriority_t) osPriorityNormal, };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
typedef struct {
	union {
		struct {
			can_lucy_h2_conc packet_h2;
			can_lucy_h2_humidity packet_humd;
			can_lucy_h2_temp packet_temp;
		};
		uint8_t packet_raw[8 * 3];
	};
} rf_packet_test_h2;

rf_packet_test_h2 test_packet_h2 = { 0 };
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

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

	const char test[] = "Hello.7890";

	uint8_t rec_legth = 0;
	char rec_data[8 * 3 + 10] = { 0 };
	/* Infinite loop */
	for (;;) {
		HAL_GPIO_WritePin(LED_D1_GPIO_Port, LED_D1_Pin, GPIO_PIN_SET);
//		osDelay(100);

		testdata = 0;

		rec_legth = 0;

		rf_recieve_single(&rfm95, &rec_legth);
		if (rec_legth > 0) {
//			rf_read_packet(&rfm95, rec_legth, &test_packet.packet_raw);
			rf_read_packet(&rfm95, rec_legth, &test_packet_h2.packet_raw);

//			log_info("%u %u %u %u",
//					(uint32_t )(test_packet.packet_fc.fc_voltage * 1000),
//					(uint32_t )(test_packet.packet_fc.fc_current * 1000),
//					(uint32_t )(test_packet.packet_mtr.mtr_voltage * 1000),
//					(uint32_t )(test_packet.packet_mtr.mtr_current * 1000));

			log_info("%u %u %u",
					(uint32_t )(test_packet_h2.packet_h2.can_raw_lucy_h2_conc),
					(uint32_t )(test_packet_h2.packet_humd.h2_humidity),
					(uint32_t )(test_packet_h2.packet_temp.h2_temp));

		}
		HAL_GPIO_WritePin(LED_D1_GPIO_Port, LED_D1_Pin, GPIO_PIN_RESET);
		osDelay(100);
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

