/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

typedef struct {
  uint32_t batt_volt;
  uint32_t batt_cur;
  uint32_t output_volt; // 12 volt output voltage
  uint32_t output_cur;  // 12 volt output current
  uint32_t buck_cur;    // 5 volt buck current
} bbData_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_TX_MAILBOX_NONE 0x00000000UL
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan1;

UART_HandleTypeDef huart1;

/* Definitions for pull_sensor */
osThreadId_t pull_sensorHandle;
uint32_t pull_sensorBuffer[ 128 ];
osStaticThreadDef_t pull_sensorControlBlock;
const osThreadAttr_t pull_sensor_attributes = {
  .name = "pull_sensor",
  .cb_mem = &pull_sensorControlBlock,
  .cb_size = sizeof(pull_sensorControlBlock),
  .stack_mem = &pull_sensorBuffer[0],
  .stack_size = sizeof(pull_sensorBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for can_manger */
osThreadId_t can_mangerHandle;
uint32_t can_mangerBuffer[ 128 ];
osStaticThreadDef_t can_mangerControlBlock;
const osThreadAttr_t can_manger_attributes = {
  .name = "can_manger",
  .cb_mem = &can_mangerControlBlock,
  .cb_size = sizeof(can_mangerControlBlock),
  .stack_mem = &can_mangerBuffer[0],
  .stack_size = sizeof(can_mangerBuffer),
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for canRxQueue */
osMessageQueueId_t canRxQueueHandle;
uint8_t canRxQueueBuffer[ 128 * sizeof( uint32_t ) ];
osStaticMessageQDef_t canRxQueueControlBlock;
const osMessageQueueAttr_t canRxQueue_attributes = {
  .name = "canRxQueue",
  .cb_mem = &canRxQueueControlBlock,
  .cb_size = sizeof(canRxQueueControlBlock),
  .mq_mem = &canRxQueueBuffer,
  .mq_size = sizeof(canRxQueueBuffer)
};
/* USER CODE BEGIN PV */
bbData_t battery_board_data[10];

volatile uint32_t adcResults[5];

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
uint32_t TxMailboxCanTask;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
void AdcTask(void *argument);
void CanTask(void *argument);

/* USER CODE BEGIN PFP */

uint32_t conversion_completed = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  if (hadc->Instance == hadc1.Instance) {
    conversion_completed++;
  }
}

// NICK: I edited how my Callbacks work to be a little safer, check it out
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);
  if (osMessageQueueGetSpace(canRxQueueHandle) == 0) {
    Error_Handler();
  }
  osMessageQueuePut(canRxQueueHandle, &RxHeader.StdId, 0U, 0UL);
  // need condition for either 4 byte or 8 byte
  if (RxHeader.DLC == 0UL) {
    // Data request, don't add anything else to queue
  } else if (RxHeader.DLC <= 4UL) {
    osMessageQueuePut(canRxQueueHandle, RxData, 0U, 0UL);
  } else {
    osMessageQueuePut(canRxQueueHandle, RxData, 0U, 0UL);
    osMessageQueuePut(canRxQueueHandle, RxData + 4, 0U, 0UL);
  }
}

/* Transmit Completed Callbacks for Message Sent Confirmations */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
  if (TxMailboxCanTask == CAN_TX_MAILBOX0) {
    //osSemaphoreRelease(canMsgReceivedSemHandle);
    TxMailboxCanTask = CAN_TX_MAILBOX_NONE;
  }
}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
  if (TxMailboxCanTask == CAN_TX_MAILBOX1) {
    //osSemaphoreRelease(canMsgReceivedSemHandle);
    TxMailboxCanTask = CAN_TX_MAILBOX_NONE;
  }
}
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
  if (TxMailboxCanTask == CAN_TX_MAILBOX2) {
    //osSemaphoreRelease(canMsgReceivedSemHandle);
    TxMailboxCanTask = CAN_TX_MAILBOX_NONE;
  }
}

HAL_StatusTypeDef HAL_CAN_SafeAddTxMessage(uint8_t *msg, uint32_t msg_id,
                                           uint32_t msg_length,
                                           uint32_t *TxMailbox, uint32_t rtr) {
#define CAN_ADD_TX_TIMEOUT_MS 5000

  uint32_t fc_tick;
  HAL_StatusTypeDef hal_stat;
  CAN_TxHeaderTypeDef TxHeader;

  // These will never change
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.ExtId = 0;
  TxHeader.TransmitGlobalTime = DISABLE;

  // User will give us this information
  TxHeader.RTR = rtr;
  TxHeader.StdId = msg_id;
  TxHeader.DLC = msg_length;

  // Start a timer to check timeout conditions
  fc_tick = HAL_GetTick();

  /* Try to add a Tx message. Returns HAL_ERROR if there are no avail
   * mailboxes or if the peripheral is not initialized. */
  do {
    hal_stat = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, msg, TxMailbox);
  } while (hal_stat != HAL_OK &&
           ((HAL_GetTick() - fc_tick) < CAN_ADD_TX_TIMEOUT_MS));

  return hal_stat;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
  MX_DMA_Init();
  MX_CAN1_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

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
  /* creation of canRxQueue */
  canRxQueueHandle = osMessageQueueNew (128, sizeof(uint32_t), &canRxQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of pull_sensor */
  pull_sensorHandle = osThreadNew(AdcTask, NULL, &pull_sensor_attributes);

  /* creation of can_manger */
  can_mangerHandle = osThreadNew(CanTask, NULL, &can_manger_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 5;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
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

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPLED1_Pin|GPLED2_Pin|GPLED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : CAN_STBY_Pin */
  GPIO_InitStruct.Pin = CAN_STBY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(CAN_STBY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPLED1_Pin GPLED2_Pin GPLED3_Pin */
  GPIO_InitStruct.Pin = GPLED1_Pin|GPLED2_Pin|GPLED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_AdcTask */
/**
 * @brief  Function implementing the pull_sensor thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_AdcTask */
void AdcTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  const float ADC_VOLT_REF = 3.3f / 4096.0f;
  const float VOLT_TO_CURR = 2.5f; // A/V
  const float BATTERY_VOLT_DIVIDER =
      10.8f; // voltage divider input 18.5v/ output 1.7123412v
  const float OUTPUT_VOLT_DIVIDER =
      7.1f; // voltage divider input 12v/ output 1.68862275v
  // memset(adcResults, 0, 5);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcResults, 5);
  int i = 0;

  /* Infinite loop */
  for (;;) {
    if (conversion_completed) {
      battery_board_data[i].batt_volt =
          adcResults[0] * ADC_VOLT_REF * BATTERY_VOLT_DIVIDER;
      battery_board_data[i].batt_cur =
          adcResults[1] * ADC_VOLT_REF * VOLT_TO_CURR;
      battery_board_data[i].output_cur =
          adcResults[2] * ADC_VOLT_REF * VOLT_TO_CURR;
      battery_board_data[i].output_volt =
          adcResults[3] * ADC_VOLT_REF * OUTPUT_VOLT_DIVIDER;
      battery_board_data[i].output_cur =
          adcResults[4] * ADC_VOLT_REF * VOLT_TO_CURR;
      HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcResults, 5);
      conversion_completed = 0;

      // NICK: I am guessing this is for debugging
      i++;
      if (i >= 10) {
        i = 0;
      }
    }
    osDelay(10);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_CanTask */
/**
 * @brief Function implementing the can_manger thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_CanTask */
void CanTask(void *argument)
{
  /* USER CODE BEGIN CanTask */
  
  // NICK: You never start the CAN peripheral or any of the notifications
  HAL_CAN_Start(&hcan1);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);
  /* Infinite loop */

  HAL_StatusTypeDef hal_stat1, hal_stat2, hal_stat3;

  uint32_t queueData;
  uint32_t floatPackage[5];

  for (;;) {
    if (osMessageQueueGet(canRxQueueHandle, &queueData, 0U, osWaitForever) ==
        osOK) {
      floatPackage[0] = battery_board_data[9].batt_cur;
      floatPackage[1] = battery_board_data[9].batt_volt;
      floatPackage[2] = battery_board_data[9].buck_cur;
      floatPackage[3] = battery_board_data[9].output_cur;
      floatPackage[4] = battery_board_data[9].output_volt;
      // Need to make a switch depending on the data requested
      // Only going to send all
      //
      // NICK: queueData is not a CAN message ID
      // also, each 'package' can only have 8 bytes of data
      // look more at my code. There is a reason why I am only
      // sending up to 8 bytes of data at a time
      // Also, where is hal_stat 1,2,3 defined? Do they serve a purpose?
      hal_stat1 =
          HAL_CAN_SafeAddTxMessage((uint8_t *)&floatPackage, queueData, 8UL,
                                   &TxMailboxCanTask, CAN_RTR_DATA);
      hal_stat2 =
          HAL_CAN_SafeAddTxMessage((uint8_t *)&floatPackage + 8, queueData, 8UL,
                                   &TxMailboxCanTask, CAN_RTR_DATA);
      hal_stat3 =
          HAL_CAN_SafeAddTxMessage((uint8_t *)&floatPackage + 12, queueData,
                                   4UL, &TxMailboxCanTask, CAN_RTR_DATA);
      // send all the data
    }
    osDelay(10);
  }
  /* USER CODE END CanTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
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
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
