/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "can.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "rf-rfm95.h"
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

/* USER CODE BEGIN PV */
//CAN_RxHeaderTypeDef RxHeader;
//uint8_t TxData[8];
//uint8_t RxData[8];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int _write(int file, char *ptr, int len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart2, (uint8_t*) ptr, len, HAL_MAX_DELAY);
	return len;
}
//#ifndef RFM95_SPI_TIMEOUT
//#define RFM95_SPI_TIMEOUT 10
//#endif
//
//#ifndef RFM95_WAKEUP_TIMEOUT
//#define RFM95_WAKEUP_TIMEOUT 10
//#endif
//
//#ifndef RFM95_SEND_TIMEOUT
//#define RFM95_SEND_TIMEOUT 100
//#endif
//
//#ifndef RFM95_RECEIVE_TIMEOUT
//#define RFM95_RECEIVE_TIMEOUT 1000
//#endif
//
//#define RFM95_EEPROM_CONFIG_MAGIC 0xab67
//#define RFM9x_VER 0x12
//typedef enum {
//	RFM95_REGISTER_FIFO_ACCESS = 0x00,
//	RFM95_REGISTER_OP_MODE = 0x01,
//	RFM95_REGISTER_FR_MSB = 0x06,
//	RFM95_REGISTER_FR_MID = 0x07,
//	RFM95_REGISTER_FR_LSB = 0x08,
//	RFM95_REGISTER_PA_CONFIG = 0x09,
//	RFM95_REGISTER_LNA = 0x0C,
//	RFM95_REGISTER_FIFO_ADDR_PTR = 0x0D,
//	RFM95_REGISTER_FIFO_TX_BASE_ADDR = 0x0E,
//	RFM95_REGISTER_FIFO_RX_BASE_ADDR = 0x0F,
//	RFM95_REGISTER_IRQ_FLAGS = 0x12,
//	RFM95_REGISTER_FIFO_RX_BYTES_NB = 0x13,
//	RFM95_REGISTER_PACKET_SNR = 0x19,
//	RFM95_REGISTER_MODEM_CONFIG_1 = 0x1D,
//	RFM95_REGISTER_MODEM_CONFIG_2 = 0x1E,
//	RFM95_REGISTER_SYMB_TIMEOUT_LSB = 0x1F,
//	RFM95_REGISTER_PREAMBLE_MSB = 0x20,
//	RFM95_REGISTER_PREAMBLE_LSB = 0x21,
//	RFM95_REGISTER_PAYLOAD_LENGTH = 0x22,
//	RFM95_REGISTER_MAX_PAYLOAD_LENGTH = 0x23,
//	RFM95_REGISTER_MODEM_CONFIG_3 = 0x26,
//	RFM95_REGISTER_INVERT_IQ_1 = 0x33,
//	RFM95_REGISTER_SYNC_WORD = 0x39,
//	RFM95_REGISTER_INVERT_IQ_2 = 0x3B,
//	RFM95_REGISTER_DIO_MAPPING_1 = 0x40,
//	RFM95_REGISTER_VERSION = 0x42,
//	RFM95_REGISTER_PA_DAC = 0x4D
//} rfm95_register_t;
//
//#define RFM95_REGISTER_OP_MODE_SLEEP                            0x00
//#define RFM95_REGISTER_OP_MODE_LORA_SLEEP                       0x80
//#define RFM95_REGISTER_OP_MODE_LORA_STANDBY                     0x81
//#define RFM95_REGISTER_OP_MODE_LORA_TX                          0x83
//#define RFM95_REGISTER_OP_MODE_LORA_RX_SINGLE                   0x86
//
//#define RFM95_REGISTER_PA_DAC_LOW_POWER                         0x84
//#define RFM95_REGISTER_PA_DAC_HIGH_POWER                        0x87
//
//#define RFM95_REGISTER_DIO_MAPPING_1_IRQ_FOR_TXDONE             0x40
//#define RFM95_REGISTER_DIO_MAPPING_1_IRQ_FOR_RXDONE             0x00
//
//#define RFM95_REGISTER_INVERT_IQ_1_TX                    		0x27
//#define RFM95_REGISTER_INVERT_IQ_2_TX							0x1d
//
//#define RFM95_REGISTER_INVERT_IQ_1_RX                    		0x67
//#define RFM95_REGISTER_INVERT_IQ_2_RX							0x19

//static bool read_register(rfm95_register_t reg, uint8_t *buffer, size_t length) {
//	HAL_GPIO_WritePin(SPI_NSS_GPIO_GPIO_Port, SPI_NSS_GPIO_Pin, GPIO_PIN_RESET);
//
//	uint8_t transmit_buffer = (uint8_t) reg & 0x7fu;
//
//	if (HAL_SPI_Transmit(&hspi1, &transmit_buffer, 1, RFM95_SPI_TIMEOUT)
//			!= HAL_OK) {
//		return false;
//	}
//
//	if (HAL_SPI_Receive(&hspi1, buffer, length, RFM95_SPI_TIMEOUT) != HAL_OK) {
//		return false;
//	}
//
//	HAL_GPIO_WritePin(SPI_NSS_GPIO_GPIO_Port, SPI_NSS_GPIO_Pin, GPIO_PIN_SET);
//
//	return true;
//}

//static bool write_register(rfm95_handle_t *handle, rfm95_register_t reg,
//		uint8_t value) {
//	HAL_GPIO_WritePin(handle->nss_port, handle->nss_pin, GPIO_PIN_RESET);
//
//	uint8_t transmit_buffer[2] = { ((uint8_t) reg | 0x80u), value };
//
//	if (HAL_SPI_Transmit(handle->spi_handle, transmit_buffer, 2,
//			RFM95_SPI_TIMEOUT) != HAL_OK) {
//		return false;
//	}
//
//	HAL_GPIO_WritePin(handle->nss_port, handle->nss_pin, GPIO_PIN_SET);
//
//	return true;
//}

//static void reset() {
//	HAL_GPIO_WritePin(RF_NRST_GPIO_Port, RF_NRST_Pin, GPIO_PIN_RESET);
//	HAL_Delay(1); // 0.1ms would theoretically be enough
//	HAL_GPIO_WritePin(RF_NRST_GPIO_Port, RF_NRST_Pin, GPIO_PIN_SET);
//	HAL_Delay(5);
//}

//int rfm95_init() {
//	reset();
//
//	// Check for correct version.
//	uint8_t version;
//	if (!read_register(RFM95_REGISTER_VERSION, &version, 1))
//		return 1;
//
//	printf("%d", version);
//
//	if (version != RFM9x_VER)
//		return 1;
//
//	return 0;
//}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_CAN1_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_SPI1_Init();
	/* USER CODE BEGIN 2 */
//	rfm95_init();
//	uint8_t version = 0, temp = 0;
	rf_handle_t rfm95 = { .rf_nreset_port = RF_NRST_GPIO_Port, .rf_nreset_pin =
	RF_NRST_Pin, .rf_nss_port = SPI1_NSS_GPIO_Port, .rf_nss_pin =
	SPI1_NSS_Pin, .rf_spi_handle = &hspi1, .rf_delay_func = HAL_Delay,
			.rf_spi_timeout = HAL_MAX_DELAY };

	rf_initialize_radio(&rfm95);
	uint8_t testdata = 347;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		HAL_Delay(500);

		//rf_initialize_radio(&rfm95);
		rf_send(&rfm95, &testdata, 1);
		//reset();
//		printf(
//						"\x1b[31;1;4m[RFlib] [ERROR] \x1b[0m\x1b[31m Module did not return a version; SPI Error\x1b[0m\n\r");

// Check for correct version.
//		if (!read_register(RFM95_REGISTER_VERSION, &version, 1))
//			;
//
//		printf("Version: 0x%x\n\r", version);
		//return 1;
//		for (int i = 0; i < 128; i++) {
//			printf("0x");
//			printf("%x: ", i);
//			printf(": 0x");
//			read_register(i, &temp, 1);
//			printf("%x\n\r", temp);
//		}

//		if (version != RFM9x_VER)
//			return 1;
		//CAN_Write_Hello();
		HAL_Delay(500);
//		HAL_GPIO_WritePin(GPIOA, LED_D1_Pin, GPIO_PIN_RESET);

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 20;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_16);

	/** Enables the Clock Security System
	 */
	HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
		for (int i = 0; i < HARD_FAULT_LED_DELAY; i++)
			;
		//HAL_GPIO_WritePin(GPIOA, LED_D1_PWM_Pin, GPIO_PIN_SET);
		for (int i = 0; i < HARD_FAULT_LED_DELAY; i++)
			;
		//HAL_GPIO_WritePin(GPIOA, LED_D2_Pin, GPIO_PIN_SET);
		for (int i = 0; i < HARD_FAULT_LED_DELAY; i++)
			;
		//HAL_GPIO_WritePin(GPIOA, LED_D3_Pin, GPIO_PIN_SET);
		for (int i = 0; i < HARD_FAULT_LED_DELAY; i++)
			;
		//HAL_GPIO_WritePin(GPIOA, LED_D1_Pin, GPIO_PIN_RESET);
		for (int i = 0; i < HARD_FAULT_LED_DELAY; i++)
			;
		//HAL_GPIO_WritePin(GPIOA, LED_D2_Pin, GPIO_PIN_RESET);
		for (int i = 0; i < HARD_FAULT_LED_DELAY; i++)
			;
		//HAL_GPIO_WritePin(GPIOA, LED_D3_Pin, GPIO_PIN_RESET);

	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
