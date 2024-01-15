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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */
typedef struct
{
  uint8_t x, y, z;
} accData_t;

typedef struct  
{
  uint8_t purge_state, supply_state;
  float internal_stack_temp, internal_stack_pressure;
} fcData_t;

typedef struct 
{
  struct FCData* pFC;
  uint8_t FCData_ID;
} canPack_t;

typedef struct
{
  uint8_t H2_OK;
  float cap_voltage;
} canData_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FULL_CAP_CHARGE_V 18

#define FUEL_CELL_OFF_STATE 0x00
#define FUEL_CELL_STRTUP_STATE 0x01
#define FUEL_CELL_CHRGE_STATE 0x02
#define FUEL_CELL_RUN_STATE 0x04

#define CAN_TX_MAILBOX_NONE 0x00000000U // Remove reference to tx mailbox
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* Definitions for CanTask */
osThreadId_t CanTaskHandle;
uint32_t CanTaskBuffer[128];
osStaticThreadDef_t CanTaskControlBlock;
const osThreadAttr_t CanTask_attributes = {
    .name = "CanTask",
    .cb_mem = &CanTaskControlBlock,
    .cb_size = sizeof(CanTaskControlBlock),
    .stack_mem = &CanTaskBuffer[0],
    .stack_size = sizeof(CanTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal1,
};
/* Definitions for I2cTask */
osThreadId_t I2cTaskHandle;
uint32_t I2cTaskBuffer[128];
osStaticThreadDef_t I2cTaskControlBlock;
const osThreadAttr_t I2cTask_attributes = {
    .name = "I2cTask",
    .cb_mem = &I2cTaskControlBlock,
    .cb_size = sizeof(I2cTaskControlBlock),
    .stack_mem = &I2cTaskBuffer[0],
    .stack_size = sizeof(I2cTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal2,
};
/* Definitions for FuelCellTask */
osThreadId_t FuelCellTaskHandle;
uint32_t FuelCellTaskBuffer[128];
osStaticThreadDef_t FuelCellTaskControlBlock;
const osThreadAttr_t FuelCellTask_attributes = {
    .name = "FuelCellTask",
    .cb_mem = &FuelCellTaskControlBlock,
    .cb_size = sizeof(FuelCellTaskControlBlock),
    .stack_mem = &FuelCellTaskBuffer[0],
    .stack_size = sizeof(FuelCellTaskBuffer),
    .priority = (osPriority_t)osPriorityNormal3,
};
/* Definitions for canMsgOkSem */
osSemaphoreId_t canMsgOkSemHandle;
const osSemaphoreAttr_t canMsgOkSem_attributes = {
    .name = "canMsgOkSem"};
/* USER CODE BEGIN PV */
uint8_t fc_state = FUEL_CELL_OFF_STATE;

canData_t canData;

CAN_TxHeaderTypeDef TxHeader, TxHeaderFuelCellTask;
CAN_RxHeaderTypeDef RxHeader;

uint32_t TxMailbox, TxMailboxFuelCellTask;
uint8_t TxData[8];
uint8_t RxData[8];

uint8_t RxUARTbuff;
uint8_t RxUARTData[32];
uint8_t UartIndex = 0;

uint8_t uselessvariable = 90; 

uint32_t button_debounce;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void StartCanTask(void *argument);
void StartI2cTask(void *argument);
void StartFuelCellTask(void *argument);

/* USER CODE BEGIN PFP */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);
}

/* Transmit Completed Callbacks for Message Sent Confirmations */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
  if (TxMailboxFuelCellTask == CAN_TX_MAILBOX0)
  {
    TxMailboxFuelCellTask = CAN_TX_MAILBOX_NONE;
    osSemaphoreRelease(canMsgOkSemHandle);
  }
}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
  if (TxMailboxFuelCellTask == CAN_TX_MAILBOX1)
  {
    TxMailboxFuelCellTask = CAN_TX_MAILBOX_NONE;
    osSemaphoreRelease(canMsgOkSemHandle);
  }
}
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
  if (TxMailboxFuelCellTask == CAN_TX_MAILBOX2)
  {
    TxMailboxFuelCellTask = CAN_TX_MAILBOX_NONE;
    osSemaphoreRelease(canMsgOkSemHandle);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (RxUARTbuff == (char)0x04)
  {
    // RxUARTData[UartIndex++] = '\r';
    // RxUARTData[UartIndex] = '\n';
    HAL_UART_Transmit_DMA(huart, RxUARTData, UartIndex);
    UartIndex = 0;
  }
  else
  {
    RxUARTData[UartIndex] = RxUARTbuff;
    if (UartIndex == (sizeof(RxUARTData) - 1))
    {
      uint8_t msg[] = "\r\n\tBuffer Overflowed - Message Lost\r\n";
      HAL_UART_Transmit_DMA(huart, msg, sizeof(msg));
      UartIndex = 0;
    }
    else
    {
      UartIndex++;
    }
  }
  HAL_UART_Receive_DMA(huart, &RxUARTbuff, 1U);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
  case BRD_STRT_Pin:
    if (HAL_GetTick() - button_debounce > 1000)
    {
      if (fc_state & (FUEL_CELL_STRTUP_STATE | FUEL_CELL_CHRGE_STATE | FUEL_CELL_RUN_STATE)) // If fc_state is non-zero
      {
        button_debounce = HAL_GetTick();
        fc_state = FUEL_CELL_OFF_STATE;
      }
      else
      {
        button_debounce = HAL_GetTick();
        fc_state = FUEL_CELL_STRTUP_STATE;
      }
    }
    break;
  case BRD_PRGE_Pin:
    /* Do something */
    break;
  case EXT_STRT_Pin:
    if (fc_state & (FUEL_CELL_STRTUP_STATE | FUEL_CELL_CHRGE_STATE | FUEL_CELL_RUN_STATE))
    {
      fc_state = FUEL_CELL_OFF_STATE;
    }
    else
    {
      fc_state = FUEL_CELL_STRTUP_STATE;
    }
    break;
  case ACC_INT1_Pin:
    /* Do something */
    break;
  case ACC_INT2_Pin:
    /* Do something */
    break;
  case EXT_STOP_Pin:
    /* Do something */
    fc_state = FUEL_CELL_OFF_STATE;
    break;
  default:
    /* Should never happen */
    break;
  }
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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_CAN_Start(&hcan1);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);

  button_debounce = HAL_GetTick();
  HAL_GPIO_WritePin(FTDI_NRST_GPIO_Port, FTDI_NRST_Pin, GPIO_PIN_SET);

  HAL_UART_Receive_DMA(&huart1, &RxUARTbuff, 1U);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of canMsgOkSem */
  canMsgOkSemHandle = osSemaphoreNew(1, 0, &canMsgOkSem_attributes);

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
  /* creation of CanTask */
  CanTaskHandle = osThreadNew(StartCanTask, NULL, &CanTask_attributes);

  /* creation of I2cTask */
  I2cTaskHandle = osThreadNew(StartI2cTask, NULL, &I2cTask_attributes);

  /* creation of FuelCellTask */
  FuelCellTaskHandle = osThreadNew(StartFuelCellTask, NULL, &FuelCellTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  if (CanTaskHandle == NULL || I2cTaskHandle == NULL || FuelCellTaskHandle == NULL)
  {
    while (1)
      ;
  }
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
  while (1)
  {
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_8);
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
  hcan1.Init.Prescaler = 16;
  hcan1.Init.Mode = CAN_MODE_LOOPBACK;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
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
  CAN_FilterTypeDef sf;
  // Accept StdID's 0x100 through 0x1FF
  sf.FilterIdHigh = 0x100 << 5;
  sf.FilterMaskIdHigh = 0x700 << 5;
  sf.FilterIdLow = 0x0000;
  sf.FilterMaskIdLow = 0x0000;
  sf.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  sf.FilterBank = 0;
  sf.FilterMode = CAN_FILTERMODE_IDMASK;
  sf.FilterScale = CAN_FILTERSCALE_32BIT;
  sf.FilterActivation = CAN_FILTER_ENABLE;
  if (HAL_CAN_ConfigFilter(&hcan1, &sf) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END CAN1_Init 2 */
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10909CEC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
   */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
   */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */
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
  huart1.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
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
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
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
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SUPPLY_VLVE_Pin | PURGE_VLVE_Pin | CAN_STBY_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FTDI_NRST_GPIO_Port, FTDI_NRST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BRD_STRT_Pin BRD_PRGE_Pin */
  GPIO_InitStruct.Pin = BRD_STRT_Pin | BRD_PRGE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SUPPLY_VLVE_Pin PURGE_VLVE_Pin CAN_STBY_Pin */
  GPIO_InitStruct.Pin = SUPPLY_VLVE_Pin | PURGE_VLVE_Pin | CAN_STBY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : EXT_STRT_Pin EXT_STOP_Pin */
  GPIO_InitStruct.Pin = EXT_STRT_Pin | EXT_STOP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ACC_INT1_Pin ACC_INT2_Pin */
  GPIO_InitStruct.Pin = ACC_INT1_Pin | ACC_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : FTDI_NRST_Pin */
  GPIO_InitStruct.Pin = FTDI_NRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FTDI_NRST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PH3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartCanTask */
/**
 * @brief  Function implementing the CanTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanTask */
void StartCanTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for (;;)
  {
    osDelay(10);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartI2cTask */
/**
 * @brief Function implementing the I2cTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartI2cTask */
void StartI2cTask(void *argument)
{
  /* USER CODE BEGIN StartI2cTask */
  /* Infinite loop */
  for (;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartI2cTask */
}

/* USER CODE BEGIN Header_StartFuelCellTask */
/**
 * @brief Function implementing the FuelCellTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartFuelCellTask */
void StartFuelCellTask(void *argument)
{
  /* USER CODE BEGIN StartFuelCellTask */
#define CAN_MESSAGE_TIMEOUT_MS 5000
#define CAN_ADD_TX_TIMEOUT_MS 5000
  osStatus_t os_stat;
  uint32_t fc_tick;
  HAL_StatusTypeDef hal_stat;

  TxHeaderFuelCellTask.IDE = CAN_ID_STD;
  TxHeaderFuelCellTask.RTR = CAN_RTR_DATA;
  TxHeaderFuelCellTask.ExtId = 0;
  TxHeaderFuelCellTask.TransmitGlobalTime = DISABLE;
  /* Infinite loop */
  for (;;)
  {
    switch (fc_state)
    {
    case FUEL_CELL_OFF_STATE:
      HAL_GPIO_WritePin(PURGE_VLVE_GPIO_Port, PURGE_VLVE_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(SUPPLY_VLVE_GPIO_Port, SUPPLY_VLVE_Pin, GPIO_PIN_RESET);
      // CAN message to relay board all relays off
      break;
    case FUEL_CELL_STRTUP_STATE:
      TxHeaderFuelCellTask.StdId = 0x103;
      TxHeaderFuelCellTask.DLC = 8;
      uint8_t mymsg[8] = {1, 2, 3, 4, 5, 6, 7, 8};

      // Try to add tx message
      fc_tick = HAL_GetTick();
      do
      {
        hal_stat = HAL_CAN_AddTxMessage(&hcan1, &TxHeaderFuelCellTask, mymsg, &TxMailboxFuelCellTask);
      } while (hal_stat != HAL_OK && (HAL_GetTick() - fc_tick < CAN_ADD_TX_TIMEOUT_MS));

      // Wait for tx message to be sent. If never successfully sent, timeout and return to off state.
      if (osSemaphoreAcquire(canMsgOkSemHandle, CAN_MESSAGE_TIMEOUT_MS) == osOK)
      {
        HAL_GPIO_WritePin(SUPPLY_VLVE_GPIO_Port, SUPPLY_VLVE_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(PURGE_VLVE_GPIO_Port, PURGE_VLVE_Pin, GPIO_PIN_SET);
        osDelay(500);
        HAL_GPIO_WritePin(PURGE_VLVE_GPIO_Port, PURGE_VLVE_Pin, GPIO_PIN_RESET);
        fc_state = FUEL_CELL_CHRGE_STATE;
      }
      else
      {
        // Tx never added or didn't send succesfully in CAN_MESSAGE_TIMEOUT_MS
        fc_state = FUEL_CELL_OFF_STATE;
      }
      break;
    case FUEL_CELL_CHRGE_STATE:
      if (canData.cap_voltage >= FULL_CAP_CHARGE_V)
      {
        fc_state = FUEL_CELL_RUN_STATE;
      }
      // CAN message to relay board cap charge mode
      // Wait for caps to charge up
      // Move to run mode
      break;
    case FUEL_CELL_RUN_STATE:
      // CAN message to relay board run mode

      break;
    }
  }
  /* USER CODE END StartFuelCellTask */
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
  if (htim->Instance == TIM1)
  {
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
  while (1)
  {
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
