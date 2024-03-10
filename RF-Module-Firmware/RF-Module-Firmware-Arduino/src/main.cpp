#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h> // include libraries
#include <stdint.h>
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_can.h>
byte msgCount = 0;     // count of outgoing messages
int interval = 2000;   // interval between sends
long lastSendTime = 0; // time of last packet send
#include "packets.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define STD_ID 0x101
#define DATA_LENGTH 8
/* USER CODE END Private defines */

void MX_CAN1_Init(void);

/* USER CODE BEGIN Prototypes */
void CAN_Initialize();
int CAN_Transmit(uint32_t _device_address, uint32_t *_buffer_pointer,
                 int _buffer_length, uint32_t _RTR);

/* USER CODE END Prototypes */
/* USER CODE BEGIN 0 */
CAN_TxHeaderTypeDef TxHeader;

uint32_t TxMailbox;
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
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
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
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

/* USER CODE BEGIN 1 */

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
    printf("[!SYSTEM ERROR]CAN Initialization Error At CAN Start");
    Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    printf(
        "[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT MESSAGE PENDING RX FIFO 0");
    Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
  {
    printf(
        "[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT MESSAGE PENDING RX FIFO 1");
    Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_FULL) != HAL_OK)
  {
    printf(
        "[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT RX FIFO 0 FULL");
    Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_FULL) != HAL_OK)
  {
    printf(
        "[!SYSTEM ERROR]CAN Initialization Error At CAN INTURRUPT RX FIFO 1 FULL");
    Error_Handler();
  }

  //	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_ERROR))
  //	!= HAL_OK) {
  //		Error_Handler();
  //	}
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.DLC = 8;
  TxHeader.TransmitGlobalTime = DISABLE;
}

/**
 * Send 8 bytes at a time, with standard id size.
 */
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

void sendMessage(String outgoing)
{
  LoRa.beginPacket();   // start packet
  LoRa.print(outgoing); // add payload
  LoRa.endPacket();     // finish packet and send it
  msgCount++;           // increment message ID
}

void onReceive(int packetSize)
{
  if (packetSize == 0)
    return; // if there's no packet, return

  // read packet header bytes:
  String incoming = "";

  while (LoRa.available())
  {
    incoming += (char)LoRa.read();
  }

  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

void setup()
{
  HAL_Init();
  SystemClock_Config();
  // put your setup code here, to run once:
  pinMode(PA10, OUTPUT);
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("LoRa Duplex - Set sync word");
  // Serial.println("LoRa Dump Registers");
  SPI.setSSEL(PA4);
  SPI.setMISO(PA6);
  SPI.setMOSI(PA7);
  SPI.setSCLK(PA5);
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(PA4, PB5, PB0); // set CS, reset, IRQ pin
  // pinMode(PB5, OUTPUT);
  // digitalWrite(PB5, HIGH);
  LoRa.setSPI(SPI);

  if (!LoRa.begin(868E6))
  { // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ; // if failed, do nothing
  }
  // LoRa.setTxPower(20);
  // LoRa.setSpreadingFactor(7);
  // LoRa.setSignalBandwidth(500E3);
  // LoRa.setSyncWord(0xF3); // ranges from 0-0xFF, default 0x34, see API docs
  Serial.println("LoRa init succeeded.");
  // LoRa.dumpRegisters(Serial);
}

void loop()
{
  // Serial.println("Running Sending and Recieving");
  digitalToggle(PA10);
  if (millis() - lastSendTime > interval)
  {
    String message = "FormulaSucks"; // send a message
    message += msgCount;
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();        // timestamp the message
    interval = random(2000) + 1000; // 2-3 seconds
    msgCount++;
  }

  // parse for a packet, and call onReceive with the result:
  // onReceive(LoRa.parsePacket());
  // put your main code here, to run repeatedly:
}

#ifdef __cplusplus
extern "C"
{
#endif

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
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_16);

    /** Enables the Clock Security System
     */
    HAL_RCC_EnableCSS();
  }

#ifdef __cplusplus
}
#endif
