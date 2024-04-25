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
#include "cmsis_os2.h"
#include "stm32f4xx_hal_def.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "canid.h"
#include "usbd_cdc_if.h"
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */
typedef enum {
  ALL_RELAY_OFF = 0x00,
  CAP_RELAY = 0x01,
  RES_RELAY = 0x02,
  DSCHRGE_RELAY = 0x04,
  MTR_RELAY = 0x08,
} relayBit_t;

typedef enum {
  RELAY_STBY = ALL_RELAY_OFF,
  RELAY_STRTP = RES_RELAY | DSCHRGE_RELAY,
  RELAY_CHRGE = RES_RELAY,
  RELAY_RUN = CAP_RELAY | DSCHRGE_RELAY | MTR_RELAY,
} rbState_t;

typedef struct {
  float fc_volt;
  float fc_curr;
  float mtr_volt;
  float mtr_curr;
  float cap_volt;
  float cap_curr;
} rbData_t;

typedef struct {
  uint8_t H2_OK;
} canData_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_MESSAGE_SENT_TIMEOUT_MS 500
#define CAN_ADD_TX_TIMEOUT_MS 500

#define FULL_CAP_CHARGE_V 20

#define CAN_TX_MAILBOX_NONE 0x00000000U // Remove reference to tx mailbox
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

CAN_HandleTypeDef hcan1;

/* Definitions for RelayTask */
osThreadId_t RelayTaskHandle;
uint32_t RelayTaskBuffer[256];
osStaticThreadDef_t RelayTaskControlBlock;
const osThreadAttr_t RelayTask_attributes = {
    .name = "RelayTask",
    .cb_mem = &RelayTaskControlBlock,
    .cb_size = sizeof(RelayTaskControlBlock),
    .stack_mem = &RelayTaskBuffer[0],
    .stack_size = sizeof(RelayTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for CanRtrTask */
osThreadId_t CanRtrTaskHandle;
uint32_t CanRtrTaskBuffer[256];
osStaticThreadDef_t CanRtrTaskControlBlock;
const osThreadAttr_t CanRtrTask_attributes = {
    .name = "CanRtrTask",
    .cb_mem = &CanRtrTaskControlBlock,
    .cb_size = sizeof(CanRtrTaskControlBlock),
    .stack_mem = &CanRtrTaskBuffer[0],
    .stack_size = sizeof(CanRtrTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for AdcTask */
osThreadId_t AdcTaskHandle;
uint32_t AdcTaskBuffer[256];
osStaticThreadDef_t AdcTaskControlBlock;
const osThreadAttr_t AdcTask_attributes = {
    .name = "AdcTask",
    .cb_mem = &AdcTaskControlBlock,
    .cb_size = sizeof(AdcTaskControlBlock),
    .stack_mem = &AdcTaskBuffer[0],
    .stack_size = sizeof(AdcTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for IndicateLedTask */
osThreadId_t IndicateLedTaskHandle;
uint32_t IndicateLedTaskBuffer[256];
osStaticThreadDef_t IndicateLedTaskControlBlock;
const osThreadAttr_t IndicateLedTask_attributes = {
    .name = "IndicateLedTask",
    .cb_mem = &IndicateLedTaskControlBlock,
    .cb_size = sizeof(IndicateLedTaskControlBlock),
    .stack_mem = &IndicateLedTaskBuffer[0],
    .stack_size = sizeof(IndicateLedTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal4,
};
/* Definitions for CanDataTask */
osThreadId_t CanDataTaskHandle;
uint32_t CanDataTaskBuffer[256];
osStaticThreadDef_t CanDataTaskControlBlock;
const osThreadAttr_t CanDataTask_attributes = {
    .name = "CanDataTask",
    .cb_mem = &CanDataTaskControlBlock,
    .cb_size = sizeof(CanDataTaskControlBlock),
    .stack_mem = &CanDataTaskBuffer[0],
    .stack_size = sizeof(CanDataTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal4,
};
/* Definitions for canRxRtrMsgQueue */
osMessageQueueId_t canRxRtrMsgQueueHandle;
uint8_t canRxRtrMsgQueueBuffer[128 * sizeof(uint32_t)];
osStaticMessageQDef_t canRxRtrMsgQueueControlBlock;
const osMessageQueueAttr_t canRxRtrMsgQueue_attributes = {
    .name = "canRxRtrMsgQueue",
    .cb_mem = &canRxRtrMsgQueueControlBlock,
    .cb_size = sizeof(canRxRtrMsgQueueControlBlock),
    .mq_mem = &canRxRtrMsgQueueBuffer,
    .mq_size = sizeof(canRxRtrMsgQueueBuffer)};
/* Definitions for canRxDataMsgQueue */
osMessageQueueId_t canRxDataMsgQueueHandle;
uint8_t canRxDataMsgQueueBuffer[16 * sizeof(uint16_t)];
osStaticMessageQDef_t canRxDataMsgQueueControlBlock;
const osMessageQueueAttr_t canRxDataMsgQueue_attributes = {
    .name = "canRxDataMsgQueue",
    .cb_mem = &canRxDataMsgQueueControlBlock,
    .cb_size = sizeof(canRxDataMsgQueueControlBlock),
    .mq_mem = &canRxDataMsgQueueBuffer,
    .mq_size = sizeof(canRxDataMsgQueueBuffer)};
/* Definitions for canMsgReceivedSem */
osSemaphoreId_t canMsgReceivedSemHandle;
const osSemaphoreAttr_t canMsgReceivedSem_attributes = {
    .name = "canMsgReceivedSem"};
/* USER CODE BEGIN PV */
rbState_t rb_state = RELAY_STBY;
rbData_t relay_board_data;

volatile uint16_t adc1Results[3];
volatile uint16_t adc2Results[3];

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
uint32_t TxMailboxCanTask;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
void StartRelayTask(void *argument);
void StartCanRtrTask(void *argument);
void StartAdcTask(void *argument);
void StartIndicateLedTask(void *argument);
void StartCanDataTask(void *argument);

/* USER CODE BEGIN PFP */
int _write(int file, char *ptr, int len) {
  CDC_Transmit_FS((uint8_t *)ptr, (uint16_t)len);
  return len;
}

/**
 * The canRxMsqQueue is 32 items each slot with 4 bytes
 * in it. The StdID is a uint32_t according to HAL and the
 * RxData is 8 items of 1 byte each. Thus osMessageQueuePut
 * will add 4 items from RxData when you pass the pointer
 * to it.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);
  if (RxHeader.RTR == CAN_RTR_REMOTE) {
    osMessageQueuePut(canRxRtrMsgQueueHandle, &RxHeader.StdId, 0U, 0UL);
  } else {
    osMessageQueuePut(canRxDataMsgQueueHandle, &RxHeader.StdId, 0U, 0UL);
    if (RxHeader.DLC <= 4UL) {
      osMessageQueuePut(canRxDataMsgQueueHandle, RxData, 0U, 0UL);
    } else {
      osMessageQueuePut(canRxDataMsgQueueHandle, RxData, 0U, 0UL);
      osMessageQueuePut(canRxDataMsgQueueHandle, &RxData[4], 0U, 0UL);
    }
  }
}

// void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
//   HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData);
//   osMessageQueuePut(canRxMsgQueueHandle, &RxHeader.StdId, 0U, 0UL);
//   // need condition for either 4 byte or 8 byte
//   if (RxHeader.DLC == 0UL) {
//     // Data request, don't add anything else to queue
//   } else if (RxHeader.DLC <= 4UL) {
//     osMessageQueuePut(canRxMsgQueueHandle, RxData, 0U, 0UL);
//   } else {
//     osMessageQueuePut(canRxMsgQueueHandle, RxData, 0U, 0UL);
//     osMessageQueuePut(canRxMsgQueueHandle, RxData + 4, 0U, 0UL);
//   }
// }

/* Transmit Completed Callbacks for Message Sent Confirmations */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
  if (TxMailboxCanTask == CAN_TX_MAILBOX0) {
    osSemaphoreRelease(canMsgReceivedSemHandle);
    TxMailboxCanTask = CAN_TX_MAILBOX_NONE;
  }
}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
  if (TxMailboxCanTask == CAN_TX_MAILBOX1) {
    osSemaphoreRelease(canMsgReceivedSemHandle);
    TxMailboxCanTask = CAN_TX_MAILBOX_NONE;
  }
}
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
  if (TxMailboxCanTask == CAN_TX_MAILBOX2) {
    osSemaphoreRelease(canMsgReceivedSemHandle);
    TxMailboxCanTask = CAN_TX_MAILBOX_NONE;
  }
}

// Counter for both DMA conversions to be Completed
// Utilized in ADC Thread below
uint32_t conversion_completed = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  if (hadc->Instance == hadc1.Instance) {
    conversion_completed++;
  }
  if (hadc->Instance == hadc2.Instance) {
    conversion_completed++;
  }
}

HAL_StatusTypeDef HAL_CAN_SafeAddTxMessage(uint8_t *msg, uint32_t msg_id,
                                           uint32_t msg_length,
                                           uint32_t *TxMailbox, uint32_t rtr) {
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
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
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
  MX_ADC1_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of canMsgReceivedSem */
  canMsgReceivedSemHandle = osSemaphoreNew(1, 0, &canMsgReceivedSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of canRxRtrMsgQueue */
  canRxRtrMsgQueueHandle =
      osMessageQueueNew(128, sizeof(uint32_t), &canRxRtrMsgQueue_attributes);

  /* creation of canRxDataMsgQueue */
  canRxDataMsgQueueHandle =
      osMessageQueueNew(16, sizeof(uint16_t), &canRxDataMsgQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of RelayTask */
  RelayTaskHandle = osThreadNew(StartRelayTask, NULL, &RelayTask_attributes);

  /* creation of CanRtrTask */
  CanRtrTaskHandle = osThreadNew(StartCanRtrTask, NULL, &CanRtrTask_attributes);

  /* creation of AdcTask */
  AdcTaskHandle = osThreadNew(StartAdcTask, NULL, &AdcTask_attributes);

  /* creation of IndicateLedTask */
  IndicateLedTaskHandle =
     osThreadNew(StartIndicateLedTask, NULL, &IndicateLedTask_attributes);

  /* creation of CanDataTask */
  CanDataTaskHandle =
      osThreadNew(StartCanDataTask, NULL, &CanDataTask_attributes);

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
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
   */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
  HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_SYSCLK, RCC_MCODIV_5);
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data
   * Alignment and number of conversion)
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC2_Init(void) {

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data
   * Alignment and number of conversion)
   */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = ENABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 3;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */
}

/**
 * @brief CAN1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN1_Init(void) {

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 3;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_10TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
  CAN_FilterTypeDef sFilterConfig;

  // Accept 0x003
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x003 << 5;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x7FF << 5;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
    /* Filter configuration Error */
    Error_Handler();
  }

  // Accept 0x101 to 0x103
  sFilterConfig.FilterBank = 1;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x101 << 5;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x7FC << 5;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
    /* Filter configuration Error */
    Error_Handler();
  }

  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
    /* Start Error */
    Error_Handler();
  }

  /* USER CODE END CAN1_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA,
                    LED_OFF_STATE_Pin | LED_CHARGE_STATE_Pin | LED_ON_STATE_Pin,
                    GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB,
                    DSCHRGE_RELAY_Pin | RES_RELAY_Pin | CAP_RELAY_Pin |
                        MTR_RELAY_Pin | DSCHRGE_LED_Pin | RES_LED_Pin |
                        MTR_LED_Pin | CAP_LED_Pin | CAN_STBY_Pin,
                    GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 PC15 PC0
                           PC1 PC2 PC3 PC6
                           PC7 PC8 PC10 PC11
                           PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 |
                        GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 |
                        GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_11 |
                        GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_OFF_STATE_Pin LED_CHARGE_STATE_Pin
   * LED_ON_STATE_Pin */
  GPIO_InitStruct.Pin =
      LED_OFF_STATE_Pin | LED_CHARGE_STATE_Pin | LED_ON_STATE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA3 PA9 PA10 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DSCHRGE_RELAY_Pin RES_RELAY_Pin CAP_RELAY_Pin
     MTR_RELAY_Pin DSCHRGE_LED_Pin RES_LED_Pin MTR_LED_Pin CAP_LED_Pin
                           CAN_STBY_Pin */
  GPIO_InitStruct.Pin = DSCHRGE_RELAY_Pin | RES_RELAY_Pin | CAP_RELAY_Pin |
                        MTR_RELAY_Pin | DSCHRGE_LED_Pin | RES_LED_Pin |
                        MTR_LED_Pin | CAP_LED_Pin | CAN_STBY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartRelayTask */
/**
 * @brief  Function implementing the RelayTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartRelayTask */
void StartRelayTask(void *argument) {
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 5 */
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);
  /* Infinite loop */
  rb_state = RELAY_STBY;
  for (;;) {
    switch (rb_state) {
    case RELAY_STBY:
      HAL_GPIO_WritePin(GPIOB,
                        DSCHRGE_RELAY_Pin | RES_RELAY_Pin | CAP_RELAY_Pin |
                            MTR_RELAY_Pin | DSCHRGE_LED_Pin | RES_LED_Pin |
                            MTR_LED_Pin | CAP_LED_Pin,
                        GPIO_PIN_RESET);
      break;
    case RELAY_STRTP:
      HAL_GPIO_WritePin(
          GPIOB, CAP_RELAY_Pin | MTR_RELAY_Pin | MTR_LED_Pin | CAP_LED_Pin,
          GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB,
                        DSCHRGE_RELAY_Pin | RES_RELAY_Pin | DSCHRGE_LED_Pin |
                            RES_LED_Pin,
                        GPIO_PIN_SET);
      break;
    case RELAY_CHRGE:
      HAL_GPIO_WritePin(GPIOB,
                        DSCHRGE_RELAY_Pin | CAP_RELAY_Pin | MTR_RELAY_Pin |
                            DSCHRGE_LED_Pin | MTR_LED_Pin | CAP_LED_Pin,
                        GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, RES_RELAY_Pin | RES_LED_Pin, GPIO_PIN_SET);
      break;
    case RELAY_RUN:
      HAL_GPIO_WritePin(GPIOB, RES_RELAY_Pin | RES_LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB,
                        DSCHRGE_RELAY_Pin | CAP_RELAY_Pin | MTR_RELAY_Pin |
                            DSCHRGE_LED_Pin | MTR_LED_Pin | CAP_LED_Pin,
                        GPIO_PIN_SET);
      break;
    }
    osDelay(10);
  }
  /* USER CODE END 5 */
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
  float floatPackage[2] = {0.0F};
  uint32_t queueData = 0;
  HAL_StatusTypeDef hal_stat;
  /* Infinite loop */
  for (;;) {
    if (osMessageQueueGet(canRxRtrMsgQueueHandle, &queueData, 0U,
                          osWaitForever) == osOK) {
      switch (queueData) {
      case FUEL_CELL_PACKET:
        floatPackage[0] = relay_board_data.fc_volt;
        floatPackage[1] = relay_board_data.fc_curr;
        hal_stat =
            HAL_CAN_SafeAddTxMessage((uint8_t *)&floatPackage, FUEL_CELL_PACKET,
                                     8UL, &TxMailboxCanTask, CAN_RTR_DATA);
        break;
      case MOTOR_PACKET:
        floatPackage[0] = relay_board_data.mtr_volt;
        floatPackage[1] = relay_board_data.mtr_curr;
        hal_stat =
            HAL_CAN_SafeAddTxMessage((uint8_t *)&floatPackage, MOTOR_PACKET,
                                     8UL, &TxMailboxCanTask, CAN_RTR_DATA);
        break;
      case CAPACITOR_PACKET:
        floatPackage[0] = relay_board_data.cap_volt;
        floatPackage[1] = relay_board_data.cap_curr;
        hal_stat =
            HAL_CAN_SafeAddTxMessage((uint8_t *)&floatPackage, CAPACITOR_PACKET,
                                     8UL, &TxMailboxCanTask, CAN_RTR_DATA);
        break;
      default:
        // this shouldn't happen
        break;
      }
    }
    osDelay(10);
  }
  /* USER CODE END StartCanRtrTask */
}

/* USER CODE BEGIN Header_StartAdcTask */
/** @brief Function implementing the AdcTask thread. @param argument: Not used
 * @retval None */
/* USER CODE END Header_StartAdcTask */
void StartAdcTask(void *argument) {
  /* USER CODE BEGIN StartAdcTask */
  const float ADC_VOLT_REF = 3.3F / 4096.0F;
  const float CURR_TRANSFER = 0.667F;
  const float VOLT_TRANSFER = 0.107F;
  const float VOLT_TO_CURR_UNI = 12.5F; // A/V
  const float VOLT_TO_CURR_BI = 25.0F;  // A/V
  /* Infinite loop */
  adc1Results[0] = adc1Results[1] = adc1Results[2] = 0;
  adc2Results[0] = adc2Results[1] = adc2Results[2] = 0;
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1Results, 3);
  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc2Results, 3);

  for (;;) {
    if (conversion_completed == 2) {
      relay_board_data.fc_volt = adc1Results[0] * ADC_VOLT_REF / VOLT_TRANSFER;
      relay_board_data.cap_volt = adc1Results[1] * ADC_VOLT_REF / VOLT_TRANSFER;
      relay_board_data.fc_curr =
          ((adc1Results[2] * ADC_VOLT_REF / CURR_TRANSFER) - 0.5) *
          VOLT_TO_CURR_UNI;
      relay_board_data.mtr_curr =
          ((adc2Results[0] * ADC_VOLT_REF / CURR_TRANSFER) - 0.5) *
          VOLT_TO_CURR_UNI;
      relay_board_data.cap_curr =
          ((adc2Results[1] * ADC_VOLT_REF / CURR_TRANSFER) - 2.5) *
          VOLT_TO_CURR_BI;
      relay_board_data.mtr_volt = adc2Results[2] * ADC_VOLT_REF / VOLT_TRANSFER;

      conversion_completed = 0;

      HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1Results, 3);
      HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc2Results, 3);
    }
    osDelay(10);
  }
  /* USER CODE END StartAdcTask */
}

/* USER CODE BEGIN Header_StartIndicateLedTask */
/**
 * @brief Function implementing the IndicateLedTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartIndicateLedTask */
void StartIndicateLedTask(void *argument) {
  /* USER CODE BEGIN StartIndicateLedTask */
  /* Infinite loop */
  for (;;) {
    switch (rb_state) {
    case RELAY_STBY:
      // Turn off other indicator LEDs
      HAL_GPIO_WritePin(LED_ON_STATE_GPIO_Port, LED_ON_STATE_Pin,
                        GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                        GPIO_PIN_RESET);

      // Flash LED off state
      HAL_GPIO_WritePin(LED_OFF_STATE_GPIO_Port, LED_OFF_STATE_Pin,
                        GPIO_PIN_SET);
      osDelay(500);
      HAL_GPIO_WritePin(LED_OFF_STATE_GPIO_Port, LED_OFF_STATE_Pin,
                        GPIO_PIN_RESET);
      osDelay(500);
      break;
    case RELAY_STRTP:
      // Turn off other indicator LEDs
      HAL_GPIO_WritePin(LED_ON_STATE_GPIO_Port, LED_ON_STATE_Pin,
                        GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                        GPIO_PIN_RESET);

      // Flash faster during startup phase as this section is very short
      HAL_GPIO_WritePin(LED_OFF_STATE_GPIO_Port, LED_OFF_STATE_Pin,
                        GPIO_PIN_SET);
      osDelay(50);
      HAL_GPIO_WritePin(LED_OFF_STATE_GPIO_Port, LED_OFF_STATE_Pin,
                        GPIO_PIN_RESET);
      osDelay(50);
      break;
    case RELAY_CHRGE:
      // Turn off other indicator LEDs
      HAL_GPIO_WritePin(LED_OFF_STATE_GPIO_Port, LED_OFF_STATE_Pin,
                        GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_ON_STATE_GPIO_Port, LED_ON_STATE_Pin,
                        GPIO_PIN_RESET);

      // Progressively increase the flashing speed to indicating caps are
      // charging
      if (relay_board_data.cap_volt < 10.0f) {
        HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                          GPIO_PIN_SET);
        osDelay(500 );
        HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                          GPIO_PIN_RESET);
        osDelay(500);
      } else if (relay_board_data.cap_volt < 17.0f) {
        HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                          GPIO_PIN_SET);
        osDelay(250);
        HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                          GPIO_PIN_RESET);
        osDelay(250);
      } else if (relay_board_data.cap_volt <= 25.0f) {

        HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                          GPIO_PIN_SET);
        osDelay(100);
        HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                          GPIO_PIN_RESET);
        osDelay(100);
      }
      break;
    case RELAY_RUN:
      // Turn off other indicator LEDs
      HAL_GPIO_WritePin(LED_CHARGE_STATE_GPIO_Port, LED_CHARGE_STATE_Pin,
                        GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_OFF_STATE_GPIO_Port, LED_OFF_STATE_Pin,
                        GPIO_PIN_RESET);

      // Turn on the run LED to indicate driver can apply power
      HAL_GPIO_WritePin(LED_ON_STATE_GPIO_Port, LED_ON_STATE_Pin, GPIO_PIN_SET);
      osDelay(100);
      break;
    default:
      break;
    }
  }
}
/* USER CODE END StartIndicateLedTask */

/* USER CODE BEGIN Header_StartCanDataTask */
/**
 * @brief Function implementing the CanDataTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanDataTask */
void StartCanDataTask(void *argument) {
  /* USER CODE BEGIN StartCanDataTask */
  uint32_t queueData = 0;
  HAL_StatusTypeDef hal_stat;

  const uint32_t CONFIRM_RELAY_CONFIG = 1;
  /* Infinite loop */
  for (;;) {
    if (osMessageQueueGet(canRxDataMsgQueueHandle, &queueData, 0U,
                          osWaitForever) == osOK) {
      switch (queueData) {
      case RELAY_CONFIGURATION_PACKET:
        osMessageQueueGet(canRxDataMsgQueueHandle, &queueData, 0U, 10UL);
        rb_state = queueData;
        hal_stat = HAL_CAN_SafeAddTxMessage((uint8_t *)&CONFIRM_RELAY_CONFIG,
                                            RELAY_CONFIGURATION_PACKET, 4UL,
                                            &TxMailboxCanTask, CAN_RTR_DATA);
        break;
      default:
        break;
      }
      osDelay(10);
    }
  }
  /* USER CODE END StartCanDataTask */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
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
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state
   */
  __disable_irq();
  while (1) {
    HAL_GPIO_WritePin(GPIOB,
                      DSCHRGE_LED_Pin | RES_LED_Pin | MTR_LED_Pin | CAP_LED_Pin,
                      GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOB,
                      DSCHRGE_LED_Pin | RES_LED_Pin | MTR_LED_Pin | CAP_LED_Pin,
                      GPIO_PIN_SET);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
     file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
