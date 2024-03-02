#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h> // include libraries
#include <stm32l432xx.h>

byte msgCount = 0;     // count of outgoing messages
int interval = 2000;   // interval between sends
long lastSendTime = 0; // time of last packet send

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
