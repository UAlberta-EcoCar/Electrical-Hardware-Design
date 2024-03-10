#include "hal_conf_extra.h"
#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h> // include libraries

#include <STM32_CAN.h>
#include <stdint.h>
#include <stm32l4xx.h>
#include <stm32l4xx_hal_can.h>
byte msgCount = 0;     // count of outgoing messages
int interval = 2000;   // interval between sends
long lastSendTime = 0; // time of last packet send
#include "packets.h"

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

static CAN_message_t CAN_outMsg_1;
static CAN_message_t CAN_outMsg_2;
static CAN_message_t CAN_outMsg_3;
static CAN_message_t CAN_inMsg;

// This will use PA11/12 pins for CAN1 and set RX-buffer size to 64-messages. TX-buffer size is kept at default 16.
STM32_CAN Can(CAN1, DEF, RX_SIZE_64, TX_SIZE_16);

uint8_t Counter;

void SendData() // Send can messages in 50Hz phase from timer interrupt.
{
  if (Counter >= 255)
  {
    Counter = 0;
  }

  // Only the counter value is updated to the 3 messages sent out.
  CAN_outMsg_1.buf[3] = Counter;
  Can.write(CAN_outMsg_1);

  CAN_outMsg_2.buf[5] = Counter;
  Can.write(CAN_outMsg_2);

  CAN_outMsg_3.buf[6] = Counter;
  Can.write(CAN_outMsg_3);

  Serial.print("Sent: ");
  Serial.println(Counter, HEX);
  Counter++;
}

void readCanMessage() // Read data from CAN bus and print out the messages to serial bus. Note that only message ID's that pass filters are read.
{
  Serial.print("Channel:");
  Serial.print(CAN_inMsg.bus);
  if (CAN_inMsg.flags.extended == false)
  {
    Serial.print(" Standard ID:");
  }
  else
  {
    Serial.print(" Extended ID:");
  }
  Serial.print(CAN_inMsg.id, HEX);

  Serial.print(" DLC: ");
  Serial.print(CAN_inMsg.len);
  if (CAN_inMsg.flags.remote == false)
  {
    Serial.print(" buf: ");
    for (int i = 0; i < CAN_inMsg.len; i++)
    {
      Serial.print("0x");
      Serial.print(CAN_inMsg.buf[i], HEX);
      if (i != (CAN_inMsg.len - 1))
        Serial.print(" ");
    }
    Serial.println();
  }
  else
  {
    Serial.println(" Data: REMOTE REQUEST FRAME");
  }
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

  if (!LoRa.begin(915E6))
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

  Counter = 0;

  Can.begin();
  Can.setBaudRate(1000000);
  Can.enableLoopBack(true);
  Can.setMBFilterProcessing(MB0, 0x153, 0x1FFFFFFF);
  Can.setMBFilterProcessing(MB1, 0x613, 0x1FFFFFFF);
  Can.setMBFilterProcessing(MB2, 0x615, 0x1FFFFFFF);
  Can.setMBFilterProcessing(MB3, 0x1F0, 0x1FFFFFFF);

  // We set the data that is static for the three different message structs once here.
  CAN_outMsg_1.id = (0x1A5);
  CAN_outMsg_1.len = 8;
  CAN_outMsg_1.buf[0] = 0x03;
  CAN_outMsg_1.buf[1] = 0x41;
  CAN_outMsg_1.buf[2] = 0x11;
  CAN_outMsg_1.buf[3] = 0x00;
  CAN_outMsg_1.buf[4] = 0x00;
  CAN_outMsg_1.buf[5] = 0x00;
  CAN_outMsg_1.buf[6] = 0x00;
  CAN_outMsg_1.buf[7] = 0x00;

  CAN_outMsg_2.id = (0x7E8);
  CAN_outMsg_2.len = 8;
  CAN_outMsg_2.buf[0] = 0x03;
  CAN_outMsg_2.buf[1] = 0x41;
  CAN_outMsg_2.buf[3] = 0x21;
  CAN_outMsg_2.buf[4] = 0x00;
  CAN_outMsg_2.buf[5] = 0x00;
  CAN_outMsg_2.buf[6] = 0x00;
  CAN_outMsg_2.buf[7] = 0xFF;

  CAN_outMsg_3.id = (0xA63);
  CAN_outMsg_3.len = 8;
  CAN_outMsg_3.buf[0] = 0x63;
  CAN_outMsg_3.buf[1] = 0x49;
  CAN_outMsg_3.buf[2] = 0x11;
  CAN_outMsg_3.buf[3] = 0x22;
  CAN_outMsg_3.buf[4] = 0x00;
  CAN_outMsg_3.buf[5] = 0x00;
  CAN_outMsg_3.buf[6] = 0x00;
  CAN_outMsg_3.buf[7] = 0x00;

// setup hardware timer to send data in 50Hz pace
#if defined(TIM1)
  TIM_TypeDef *Instance = TIM1;
#else
  TIM_TypeDef *Instance = TIM2;
#endif
  HardwareTimer *SendTimer = new HardwareTimer(Instance);
  SendTimer->setOverflow(50, HERTZ_FORMAT); // 50 Hz
#if (STM32_CORE_VERSION_MAJOR < 2)
  SendTimer->attachInterrupt(1, SendData);
  SendTimer->setMode(1, TIMER_OUTPUT_COMPARE);
#else // 2.0 forward
  SendTimer->attachInterrupt(SendData);
#endif
  SendTimer->resume();
}

void loop()
{
  // Serial.println("Running Sending and Recieving");
  digitalToggle(PA10);
  // The actual code that is being used will be done to main loop as usual.

  // We only read data from CAN bus if there is frames received, so that main code can do it's thing efficiently.
  while (Can.read(CAN_inMsg))
  {
    readCanMessage();
  }
  // if (millis() - lastSendTime > interval)
  // {
  //   String message = "FormulaSucks"; // send a message
  //   message += msgCount;
  //   sendMessage(message);
  //   Serial.println("Sending " + message);
  //   lastSendTime = millis();        // timestamp the message
  //   interval = random(2000) + 1000; // 2-3 seconds
  //   msgCount++;
  // }

  // parse for a packet, and call onReceive with the result:
  // onReceive(LoRa.parsePacket());
  // put your main code here, to run repeatedly:
  delay(500);
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
