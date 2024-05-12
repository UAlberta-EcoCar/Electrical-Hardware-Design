/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "packets.h"
#include "can.h"
#include "i2c.h"
#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "ssd1306.h"
#include "ssd1306.h"
#include "stdio.h"
#include "bme280/bme280.h"
#include <stdlib.h>
#include <string.h>
#include "packets.h"
#include "MQ8/mq8calibration.h"
#include "tim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
CAN_TxHeaderTypeDef TxHeader;

uint32_t TxMailbox;

CAN_RxHeaderTypeDef RxHeader;

bool h2_leak = false;

uint16_t h2_conc_ppm = 0;

uint16_t adc_buf[ADC_BUF_LEN];
uint16_t raw;

char h2_conc_str[32] = { 0 };
char bme_data_str[1024] = { 0 };

float temperature;
float humidity;
float pressure;
volatile int lockout = 0;
H2_SYSTEM_STATE system_state = H2_SYSTEM_STATE_INACTIVE;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SensorReadT */
osThreadId_t SensorReadTHandle;
uint32_t SensorReadTBuffer[ 256 ];
osStaticThreadDef_t SensorReadTControlBlock;
const osThreadAttr_t SensorReadT_attributes = {
  .name = "SensorReadT",
  .cb_mem = &SensorReadTControlBlock,
  .cb_size = sizeof(SensorReadTControlBlock),
  .stack_mem = &SensorReadTBuffer[0],
  .stack_size = sizeof(SensorReadTBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for AuxilaryPrintT */
osThreadId_t AuxilaryPrintTHandle;
uint32_t AuxilaryPrintTBuffer[ 256 ];
osStaticThreadDef_t AuxilaryPrintTControlBlock;
const osThreadAttr_t AuxilaryPrintT_attributes = {
  .name = "AuxilaryPrintT",
  .cb_mem = &AuxilaryPrintTControlBlock,
  .cb_size = sizeof(AuxilaryPrintTControlBlock),
  .stack_mem = &AuxilaryPrintTBuffer[0],
  .stack_size = sizeof(AuxilaryPrintTBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SensorTransmitT */
osThreadId_t SensorTransmitTHandle;
uint32_t SensorTransmitTBuffer[ 256 ];
osStaticThreadDef_t SensorTransmitTControlBlock;
const osThreadAttr_t SensorTransmitT_attributes = {
  .name = "SensorTransmitT",
  .cb_mem = &SensorTransmitTControlBlock,
  .cb_size = sizeof(SensorTransmitTControlBlock),
  .stack_mem = &SensorTransmitTBuffer[0],
  .stack_size = sizeof(SensorTransmitTBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LeakWatchdogT */
osThreadId_t LeakWatchdogTHandle;
uint32_t LeakWatchdogTBuffer[ 256 ];
osStaticThreadDef_t LeakWatchdogTControlBlock;
const osThreadAttr_t LeakWatchdogT_attributes = {
  .name = "LeakWatchdogT",
  .cb_mem = &LeakWatchdogTControlBlock,
  .cb_size = sizeof(LeakWatchdogTControlBlock),
  .stack_mem = &LeakWatchdogTBuffer[0],
  .stack_size = sizeof(LeakWatchdogTBuffer),
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for SensorCalibrate */
osThreadId_t SensorCalibrateHandle;
uint32_t SensorCalibrateBuffer[ 256 ];
osStaticThreadDef_t SensorCalibrateControlBlock;
const osThreadAttr_t SensorCalibrate_attributes = {
  .name = "SensorCalibrate",
  .cb_mem = &SensorCalibrateControlBlock,
  .cb_size = sizeof(SensorCalibrateControlBlock),
  .stack_mem = &SensorCalibrateBuffer[0],
  .stack_size = sizeof(SensorCalibrateBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CAN_HighPro_Q */
osMessageQueueId_t CAN_HighPro_QHandle;
uint8_t CAN_HighPro_QBuffer[ 16 * sizeof( IntrimPacket ) ];
osStaticMessageQDef_t CAN_HighPro_QControlBlock;
const osMessageQueueAttr_t CAN_HighPro_Q_attributes = {
  .name = "CAN_HighPro_Q",
  .cb_mem = &CAN_HighPro_QControlBlock,
  .cb_size = sizeof(CAN_HighPro_QControlBlock),
  .mq_mem = &CAN_HighPro_QBuffer,
  .mq_size = sizeof(CAN_HighPro_QBuffer)
};
/* Definitions for I2CBusControl */
osMutexId_t I2CBusControlHandle;
const osMutexAttr_t I2CBusControl_attributes = {
  .name = "I2CBusControl"
};
/* Definitions for calibrateSensor */
osSemaphoreId_t calibrateSensorHandle;
osStaticSemaphoreDef_t calibrateSensorControlBlock;
const osSemaphoreAttr_t calibrateSensor_attributes = {
  .name = "calibrateSensor",
  .cb_mem = &calibrateSensorControlBlock,
  .cb_size = sizeof(calibrateSensorControlBlock),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/**
 * Send 8 bytes at a time, with standard id size.
 */
int CAN_Transmit(uint32_t _device_address, uint8_t *_buffer_pointer,
		int _buffer_length, uint32_t _RTR) {

	TxHeader.StdId = _device_address;
	TxHeader.RTR = _RTR;
	if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, _buffer_pointer, &TxMailbox)
			!= HAL_OK) {
		printf(
				"Can transmission error on packet id: %hu and containing data: %u\r\n",
				_device_address, _buffer_pointer);
		Error_Handler();
	}

	return 1;
}

/**
 * Send 8 bytes at a time, with standard id size.
 */
int CAN_Transmit_Intrim(IntrimPacket _to_send) {

	TxHeader.StdId = _to_send.id;
	TxHeader.RTR = _to_send.rtr_type;
	TxHeader.DLC = _to_send.dlc;
	if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, _to_send.data, &TxMailbox)
			!= HAL_OK) {
		printf("Can transmission error on packet id: %hu\r\n", _to_send.id);
		printf(
				"[!CAN TRANSMIT FUNCTION]: TX MAILBOX FULL (CHECK CAN BUS CONNECTION)!\r\n");
		HAL_GPIO_WritePin(LED_D2_PWM_GPIO_Port, LED_D2_PWM_Pin, GPIO_PIN_SET);
		//Error_Handler();
	}

	return 1;
}

int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	if (HAL_I2C_Master_Transmit(&hi2c1, (id << 1), &reg_addr, 1, 10) != HAL_OK)
		return -1;
	if (HAL_I2C_Master_Receive(&hi2c1, (id << 1) | 0x01, data, len, 10)
			!= HAL_OK)
		return -1;

	return 0;
}

void user_delay_ms(uint32_t period) {
	osDelay(period);
}

int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	uint8_t buf[len + 1];
	buf[0] = reg_addr;
	memcpy(buf + 1, data, len);
	while (HAL_I2C_Master_Transmit(&hi2c1, (id << 1), (uint8_t*) buf, len + 1,
	HAL_MAX_DELAY) != HAL_OK)
		printf("I2C Error\r\n");
//	int8_t *buf;
//	buf = malloc(len + 1);
//	buf[0] = reg_addr;
//	memcpy(buf + 1, data, len);
//
//	if (HAL_I2C_Master_Transmit(&hi2c1, (id << 1), (uint8_t*) buf, len + 1,
//	HAL_MAX_DELAY) != HAL_OK)
//		return -1;
//
//	free(buf);
	return 0;
}

IntrimPacket packet; // Declared as a temporary variable outside ISR as this is being allocated and re allocated,
// therefore to save ISR time declaring it out side. Also volatile since only used in ISR.

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, packet.data)
			!= HAL_OK) {
		Error_Handler();
	}
	// Just check if there is atleast 1 spot open
	// If we dont and call get message we will loose that message since it wont fit and the ISR will exit.
	if (osMessageQueueGetSpace(CAN_HighPro_QHandle) >= 1) {

		printf("[CAN RX Inturrupt] Got message\r\n");
		// Mask the packet ID out.
		packet.id = RxHeader.StdId;	// & 0x0FF;
		packet.rtr_type = RxHeader.RTR;
		//memcpy(packet.data, RxData, 8); // this is always 8 bytes therefore there
		// is pretty much no added iteration time. The compiler will optimize the loop away.

		osMessageQueuePut(CAN_HighPro_QHandle, &packet, 0, 0);
	}

}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
}
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
	printf("[!TxMailbox0CompleteCallback] Sent\r\n");
}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
	printf("[!TxMailbox1CompleteCallback] Sent\r\n");
}
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
	printf("[!TxMailbox2CompleteCallback] Sent\r\n");
}
IntrimPacket empack;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == H2_TARE_Pin) {
		static int debaunce_tick = 0;
		if (!debaunce_tick) {
			debaunce_tick += HAL_GetTick();
		} else {
			if ((debaunce_tick - HAL_GetTick() > 50)) {
				//system_state = H2_SYSTEM_STATE_CALIBRATE;
				//osSemaphoreRelease(calibrateSensorHandle);
				system_state = H2_SYSTEM_STATE_ALARM_TRIPPED;
				//system_state = H2_SYSTEM_STATE_ALARM_TRIPPED;
				CAN_Transmit_Intrim(empack);
			}
		}
	}
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void startSensorReadThread(void *argument);
void startAuxilaryPrintThread(void *argument);
void startSensorTransmitThread(void *argument);
void startLeakWatchdogThread(void *argument);
void startSensorCalibrateThread(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
	 called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	empack.rtr_type = CAN_RTR_DATA;
	empack.id = RESPOND_EMERGENCY_H2_ALERT;
	empack.dlc = 1;
	empack.data[0] = 1;
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of I2CBusControl */
  I2CBusControlHandle = osMutexNew(&I2CBusControl_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of calibrateSensor */
  calibrateSensorHandle = osSemaphoreNew(1, 0, &calibrateSensor_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of CAN_HighPro_Q */
  CAN_HighPro_QHandle = osMessageQueueNew (16, sizeof(IntrimPacket), &CAN_HighPro_Q_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of SensorReadT */
  SensorReadTHandle = osThreadNew(startSensorReadThread, NULL, &SensorReadT_attributes);

  /* creation of AuxilaryPrintT */
  AuxilaryPrintTHandle = osThreadNew(startAuxilaryPrintThread, NULL, &AuxilaryPrintT_attributes);

  /* creation of SensorTransmitT */
  SensorTransmitTHandle = osThreadNew(startSensorTransmitThread, NULL, &SensorTransmitT_attributes);

  /* creation of LeakWatchdogT */
  LeakWatchdogTHandle = osThreadNew(startLeakWatchdogThread, NULL, &LeakWatchdogT_attributes);

  /* creation of SensorCalibrate */
  SensorCalibrateHandle = osThreadNew(startSensorCalibrateThread, NULL, &SensorCalibrate_attributes);

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
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	for (;;) {
		//printf("[!Default task]: s\r\n");
		osDelay(100);
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_startSensorReadThread */
/**
 * @brief Function implementing the SensorReadT thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startSensorReadThread */
void startSensorReadThread(void *argument)
{
  /* USER CODE BEGIN startSensorReadThread */
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buf, ADC_BUF_LEN);
	//	printf("Calibrating H2... \r\n");
	//	Ro = MQ8_Calibration(); //Calibrating the sensor. Please make sure the sensor is in clean air
	//							//when you perform the calibration
	//	printf("Done. \r\n");

	struct bme280_dev dev;
	struct bme280_data comp_data;
	int8_t rslt;

	osMutexAcquire(I2CBusControlHandle, osWaitForever);
	dev.dev_id = BME280_I2C_ADDR_SEC;
	dev.intf = BME280_I2C_INTF;
	dev.read = user_i2c_read;
	dev.write = user_i2c_write;
	dev.delay_ms = user_delay_ms;
	rslt = bme280_init(&dev);

	dev.settings.osr_h = BME280_OVERSAMPLING_1X;
	dev.settings.osr_p = BME280_OVERSAMPLING_16X;
	dev.settings.osr_t = BME280_OVERSAMPLING_2X;
	dev.settings.filter = BME280_FILTER_COEFF_16;
	rslt = bme280_set_sensor_settings(
			BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL
					| BME280_FILTER_SEL, &dev);
	rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);
	osDelay(50);
	osMutexRelease(I2CBusControlHandle);
	/* Infinite loop */
	// \033[2J\033[0;0H
	for (;;) {

		//HAL_GPIO_WritePin(BEEPER_GPIO_Port, BEEPER_Pin, GPIO_PIN_SET);
		printf("[Main Thread] ");
		printf("H2 ADC: ");
		printf("%f\r\n", adc_buf[ADC_BUF_LEN - 1] * 0.8f);
		//		TxHeader.StdId = 0x102;
		printf("[!Main Thread] Calibrated H2 Sensor Reading: %d\r\n",
				(h2_conc_ppm = MQ8_GetH2Percentage(
						MQ8_Compensate(adc_buf[ADC_BUF_LEN - 1]) / Ro)));
		if (osMutexAcquire(I2CBusControlHandle, osWaitForever) == osOK) {
			rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
			osMutexRelease(I2CBusControlHandle);
			if (rslt == BME280_OK) {
				temperature = comp_data.temperature / 100.0; /* °C  */
				humidity = comp_data.humidity / 1024.0; /* %   */
				pressure = comp_data.pressure / 10000.0; /* hPa */
			}

		} else {
			printf("[!Main Thread] Failed to aquire control of I2C Bus\r\n");
		}

		printf("[!Main Thread] Humidity: %03.1f\r\n", humidity);
		printf("[!Main Thread] Temprature: %03.1f\r\n", temperature);
		printf("[!Main Thread] Pressure: %03.1f\r\n", pressure);

		//		IntrimPacket packet;
		//		//packet.data = {1, 2, 3, 4, 5, 6, 7, 8};
		//		packet.rtr_type = CAN_RTR_REMOTE;
		//		TxHeader.DLC = 0;
		//		packet.id = 0x111;
		//		CAN_Transmit_Intrim(packet);

		osDelay(500);

	}
  /* USER CODE END startSensorReadThread */
}

/* USER CODE BEGIN Header_startAuxilaryPrintThread */
/**
 * @brief Function implementing the AuxilaryPrint thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startAuxilaryPrintThread */
void startAuxilaryPrintThread(void *argument)
{
  /* USER CODE BEGIN startAuxilaryPrintThread */
	osDelay(500);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	htim2.Instance->CCR2 = 0;
	if (osMutexAcquire(I2CBusControlHandle, osWaitForever) == osOK) {
		ssd1306_Init();
		osMutexRelease(I2CBusControlHandle);
	} else {
		printf("[!Secondary Thread] I2C Busy\r\n");
	}

	int temp_blink = 1, temp_blink2 = 0;
	/* Infinite loop */
	for (;;) {
		htim2.Instance->CCR2 = 1 * (temp_blink2 = !temp_blink2);

		ssd1306_Fill(Black);
		ssd1306_Line(0, 0, SSD1306_WIDTH, 0, White);
		ssd1306_SetCursor(0, 2);
		sprintf(h2_conc_str, "H2 Conc: %0.2f (mV)",
				adc_buf[ADC_BUF_LEN - 1] * 0.8f);
		ssd1306_WriteString(h2_conc_str, Font_7x10, White);

		ssd1306_SetCursor(0, 12);
		sprintf(bme_data_str, "Temperature: %2.0fC", temperature);
		ssd1306_WriteString(bme_data_str, Font_7x10, White);

		ssd1306_SetCursor(0, 22);
		sprintf(bme_data_str, "Humidity: %2.0f%%", humidity);
		ssd1306_WriteString(bme_data_str, Font_7x10, White);

		ssd1306_SetCursor(0, 32);
		sprintf(bme_data_str, "Pressure: %3.0f hPa", pressure);
		ssd1306_WriteString(bme_data_str, Font_7x10, White);

		ssd1306_SetCursor(0, 42);
		sprintf(bme_data_str,
				(system_state != H2_SYSTEM_STATE_CALIBRATE) ?
						"H2 Conc: %d ppm" : "Calibrating ppm", h2_conc_ppm);
		ssd1306_WriteString(bme_data_str, Font_7x10, White);
		ssd1306_Line(0, 62, SSD1306_WIDTH, 62, White);

		ssd1306_SetCursor(0, 52);

		switch (system_state) {
		case H2_SYSTEM_STATE_INACTIVE:
			sprintf(bme_data_str, "%s", "-ALARM INACTIVE-");
			break;
		case H2_SYSTEM_STATE_ALARM_ACTIVE:
			sprintf(bme_data_str, "%s", "-ALARM ACTIVE-");
			break;
		case H2_SYSTEM_STATE_ALARM_TEST:
			sprintf(bme_data_str, "%s", "-ALARM TEST-");
			break;
		case H2_SYSTEM_STATE_ALARM_TEST_SILENT:
			sprintf(bme_data_str, "%s", "-ALARM TEST SILENT-");
			break;
		case H2_SYSTEM_STATE_CALIBRATE:
			sprintf(bme_data_str, "%s", "-CALIBRATE-");
			break;
		case H2_SYSTEM_STATE_ALARM_TRIPPED:
			sprintf(bme_data_str, "%s", "-!ALARM TRIPPED!-");
			temp_blink = !temp_blink;
			break;
		}

		ssd1306_WriteString(bme_data_str, Font_7x10,
				temp_blink ? White : Black);

		ssd1306_Line(0, 62, SSD1306_WIDTH, 62, White);

		osMutexAcquire(I2CBusControlHandle, osWaitForever);
		ssd1306_UpdateScreen();
		osMutexRelease(I2CBusControlHandle);
		osDelay(100);
	}
  /* USER CODE END startAuxilaryPrintThread */
}

/* USER CODE BEGIN Header_startSensorTransmitThread */
/**
 * @brief Function implementing the SensorTransmitT thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startSensorTransmitThread */
void startSensorTransmitThread(void *argument)
{
  /* USER CODE BEGIN startSensorTransmitThread */
	EmergencyBroadCastPacket empack;
	IntrimPacket response_packet;
	RESPONSE_PACKET_ID response_id;
	float temp_h2;
	/* Infinite loop */
	for (;;) {

		//if (osMessageQueueGetCount(CAN_HighPro_QHandle) > 0) {
		if (osMessageQueueGet(CAN_HighPro_QHandle, &response_packet, 0,
		osWaitForever) == osOK) {
			response_packet.dlc = 8;
			if (response_packet.rtr_type == CAN_RTR_REMOTE) {
				switch (response_packet.id) {
				case GET_H2_CONC_MV:
					response_packet.id = RESPOND_H2_CONC;
					temp_h2 = adc_buf[ADC_BUF_LEN - 1] * 0.80f;
					//					sprintf(&response_packet.data, "%f",
					//							adc_buf[ADC_BUF_LEN - 1] * ADC_CONST);
					//					response_packet.data[0] = 1;
					//					response_packet.data[1] = 2;
					//					response_packet.data[2] = 3;
					//					response_packet.data[3] = 4;
					//					response_packet.data = &a;
					//					sprintf(&response_packet.data, "%d",
					//												a);
					//					TxHeader.DLC = 4;
					//					TxHeader.StdId = RESPOND_H2_CONC;
					//					TxHeader.RTR = CAN_RTR_DATA;
					//					if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &a, &TxMailbox)
					//							!= HAL_OK) {
					////						printf("Can transmission error on packet id: %hu\r\n",
					////								_to_send.id);
					//						Error_Handler();
					//					}
					response_packet.dlc = 4;
					memcpy(&response_packet.data, (uint8_t*) &temp_h2, 8);
					printf("[!SensorTransmitThread]Sending CAN H2 Conc\r\n");
					break;
				case GET_HUMIDITY:
					response_packet.id = RESPOND_HUMIDITY;
					sprintf(&response_packet.data, "%f", humidity);
					//memcpy(, humidity, 8);
					printf("[!SensorTransmitThread]Sending CAN Humidity\r\n");
					break;
				case GET_PRESSURE:
					response_packet.id = RESPOND_PRESSURE;
					sprintf(&response_packet.data, "%f", pressure);
					//memcpy(&response_packet.data, pressure, 8);
					printf("[!SensorTransmitThread]Sending CAN Pressure\r\n");
					break;
				case GET_TEMP:
					response_packet.id = RESPOND_TEMP;
					sprintf(&response_packet.data, "%f", temperature);
					//memcpy(&response_packet.data, temperature, 8);
					printf("[!SensorTransmitThread]Sending CAN Temprature\r\n");
					break;
				}
				response_packet.rtr_type = CAN_RTR_DATA;
				CAN_Transmit_Intrim(response_packet);
			}
			//			} else {
			//				printf(
			//						"[!Sensor Transmit Thread] Error getting Message from HIGH priority thread.\r\n");
			//			}
		}

		osDelay(500);
	}
  /* USER CODE END startSensorTransmitThread */
}

/* USER CODE BEGIN Header_startLeakWatchdogThread */
/**
 * @brief Function implementing the LeakWatchdogT thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startLeakWatchdogThread */
void startLeakWatchdogThread(void *argument)
{
  /* USER CODE BEGIN startLeakWatchdogThread */
	IntrimPacket empack;
	empack.rtr_type = CAN_RTR_DATA;
	empack.id = RESPOND_EMERGENCY_H2_ALERT;
	empack.dlc = 1;
	float h2_conc_mv = 0;

	// Beeper Prescaler = 80MHz/ 100^2 = 8000 (8000 - 1 cuz MX )
	// Max Freq = 80MHz/(100*8000) = 1000
	//lockout = 1;
	empack.data[0] = 1;
	system_state = H2_SYSTEM_STATE_INACTIVE;
	osDelay(500);
	system_state = H2_SYSTEM_STATE_ALARM_ACTIVE;
//	system_state = H2_SYSTEM_STATE_ALARM_TRIPPED;
//	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
//	htim2.Instance->CCR2 = 0; // Duty Cycle we want this to be 0% so that when needed we can turn the beeeper on.
	/* Infinite loop */
	for (;;) {
		if ((system_state == H2_SYSTEM_STATE_ALARM_TRIPPED)
				|| ((adc_buf[ADC_BUF_LEN - 1] * 0.8f) >= 3000.00f)) {
			system_state = H2_SYSTEM_STATE_ALARM_TRIPPED;
			static int temp = 0;
			h2_conc_mv = adc_buf[ADC_BUF_LEN - 1] * 0.8f;
			//memcpy(&empack.data, &h2_conc_mv, sizeof(float));
//			htim2.Instance->CCR2 = 100 * (temp = !temp);

			printf("[!LeakWatchDogThread] Leak detected.\r\n");
			osDelay(50);
			HAL_GPIO_TogglePin(LED_D3_PWM_GPIO_Port, LED_D3_PWM_Pin);
			CAN_Transmit_Intrim(empack);
			osDelay(50);
		}
		//		if (osSemaphoreAcquire(SensorAlertH2Handle, osWaitForever) == osOK) {
		//			while (1) {
		//				htim2.Instance->CCR1 = 0;
		//				sprintf(empack.data, "%f", adc_buf[ADC_BUF_LEN - 1] * ADC_CONST);
		//				// If we have a JERMA Alert we transmit a emergency broadcast every half second.
		//				CAN_Transmit_Intrim(empack);
		//				osDelay(100);
		//				htim2.Instance->CCR1 = 50;
		//				osDelay(100);
		//
		//			}
		//		}
		// Else transmit if message queue has request for data.
		osDelay(50);
	}
  /* USER CODE END startLeakWatchdogThread */
}

/* USER CODE BEGIN Header_startSensorCalibrateThread */
/**
 * @brief Function implementing the SensorCalibrate thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startSensorCalibrateThread */
void startSensorCalibrateThread(void *argument)
{
  /* USER CODE BEGIN startSensorCalibrateThread */
	IntrimPacket empack;
	empack.rtr_type = CAN_RTR_DATA;
	empack.id = RESPOND_EMERGENCY_H2_ALERT;
	empack.dlc = 4;
	empack.id = RESPOND_EMERGENCY_H2_ALERT;
	/* Infinite loop */
	for (;;) {
		osSemaphoreAcquire(calibrateSensorHandle, osWaitForever);
		printf("[!Calibrate Sensor Thread] Starting Calibration.\r\n");
		//		Ro = MQ8_Calibration();

		int i;
		for (i = 0; i < CALIBARAION_SAMPLE_TIMES; i++) { //take multiple samples
//			Ro += MQ8_ResistanceCalculation(adc_buf[ADC_BUF_LEN - 1] * 0.8f);

			Ro += MQ8_ResistanceCalculation(adc_buf[ADC_BUF_LEN - 1]);
			osDelay(CALIBRATION_SAMPLE_INTERVAL);
		}
		Ro = Ro / CALIBARAION_SAMPLE_TIMES;      //calculate the average value

		Ro = Ro / RO_CLEAN_AIR_FACTOR; //divided by RO_CLEAN_AIR_FACTOR yields the Ro
									   //according to the chart in the datasheet
		osDelay(500);
		system_state = H2_SYSTEM_STATE_INACTIVE;

//		if (h2_conc_ppm > 1) {
//			response_packet.rtr_type = CAN_RTR_DATA;
//			CAN_Transmit_Intrim(empack);
//		}

	}
  /* USER CODE END startSensorCalibrateThread */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

