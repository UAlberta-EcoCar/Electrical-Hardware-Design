#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h> // include libraries
#include <stdint.h>
#include <stm32l4xx.h>
#include <stm32l4xx_hal_can.h>
byte msgCount = 0;     // count of outgoing messages
int interval = 2000;   // interval between sends
long lastSendTime = 0; // time of last packet send
#include "packets.h"

#include "can.h"

uint8_t data[] = {1, 2, 3, 4};

struct __attribute__((packed)) Datasent
{
  // int h2_alarm : 1 = 0;
  // int shell_stop : 1 = 0;
  uint32_t relay_conf = 99;

  float cap_voltage = 99, cap_current = 99;

  float mtr_voltage = 99, mtr_current = 99;

  float fc_voltage = 99, fc_current = 99;

  float internal_stack_pressure = 99, internal_stack_temp = 99;

  float x_accel = 99, y_accel = 99, z_accel = 99, speed_magnitude = 99;

  float h2_voltage = 99;

  float h2_temp = 99;

  float h2_pressure = 99;

  float h2_humidity = 99;

} datasent, datarecieved;

void sendMessage(String outgoing)
{
  LoRa.beginPacket();   // start packet
  LoRa.print(outgoing); // add payload
  LoRa.endPacket(true); // finish packet and send it
  msgCount++;           // increment message ID
}

void onReceive(int packetSize)
{

  if (packetSize == 0)
    return; // if there's no packet, return

  // read packet header bytes:
  String incoming = "";
  digitalWrite(PA1, HIGH);
  // while (LoRa.available())
  // {
  //   incoming += (uint8_t)LoRa.read();
  // }
  incoming = LoRa.readString();
  // memcpy(&datarecieved, incoming.c_str(), sizeof(Datasent));
  Serial.print("[");
  Serial.print(millis());
  Serial.print("]");

  Serial.println(incoming);
  // Serial.println("RSSI: " + String(LoRa.packetRssi()));
  // Serial.println("Snr: " + String(LoRa.packetSnr()));
  //Serial.println();
  // Serial.printf("h2 hum: %f\r\n", datarecieved.cap_current);
  digitalWrite(PA1, LOW);
}

void setup()
{
  HAL_Init();
  // SystemClock_Config();

  // put your setup code here, to run once:
  // pinMode(PA10, OUTPUT);
  pinMode(PA9, OUTPUT);
  // pinMode(PA8, OUTPUT);
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
  // LoRa.setTxPower(5);
  //LoRa.setSignalBandwidth(500E3);
  // LoRa.setSignalBandwidth(10.4E3);
  // LoRa.setSpreadingFactor(7);
  if (!LoRa.begin(868E6))
  { // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ; // if failed, do nothing
  }
  MX_CAN1_Init();
  CAN_Initialize();
  LoRa.setTxPower(5);
  // LoRa.setSpreadingFactor(7);
  // LoRa.setSignalBandwidth(500E3);
  // LoRa.setSyncWord(0xF3); // ranges from 0-0xFF, default 0x34, see API docs
  Serial.println("LoRa init succeeded.");

  pinMode(PC15, OUTPUT);
  digitalWrite(PC15, LOW);
  // digitalToggle(PA10);
  pinMode(PA1, OUTPUT);

  TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(PA10), PinMap_PWM);
  uint32_t channel = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(PA10), PinMap_PWM));

  // Instantiate HardwareTimer object. Thanks to 'new' instantiation, HardwareTimer is not destructed when setup() function is finished.
  HardwareTimer *MyTim = new HardwareTimer(Instance);

  // Configure and start PWM
  // MyTim->setPWM(channel, pin, 5, 10, NULL, NULL); // No callback required, we can simplify the function call
  MyTim->setPWM(channel, PA10, 100, 10); // 5 Hertz, 10% dutycycle
  // LoRa.dumpRegisters(Serial);
}
uint8_t send = 0x01;
void loop()
{

  digitalWrite(PA1, HIGH);
  onReceive(LoRa.parsePacket());

  digitalWrite(PA1, LOW);

  // put your main code here, to run repeatedly:
  HAL_Delay(100);
  // onReceive(LoRa.parsePacket());
}
float f = 1.0;
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  // Just check if there is atleast 1 spot open
  // If we dont and call get message we will loose that message since it wont fit and the ISR will exit.
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
  {

    Serial.println("Error CAN RX");
    // Error_Handler();
  }
  char *message = "";
  if (RxHeader.RTR == CAN_RTR_REMOTE)
  {
    return;
  }
  switch (RxHeader.StdId)
  {
  case H2_ALARM:
    // datasent.h2_alarm = 1;
    break;
  case SHELL_EXT_STOP:
    // datasent.shell_stop = 1;
    break;
  case RELAY_CONF:
    memcpy(&datasent.relay_conf, &RxData, sizeof(datasent.relay_conf));
    break;
  case CAP_VOLT_CURR:
    memcpy(&datasent.cap_voltage, &RxData, sizeof(float));
    memcpy(&datasent.cap_current, &RxData[4], sizeof(float));
    break;
  case MTR_VOLT_CURR:
    memcpy(&datasent.mtr_voltage, &RxData, sizeof(float));
    memcpy(&datasent.mtr_current, &RxData[4], sizeof(float));
    break;
  case FC_VOLT_CURR:
    memcpy(&datasent.fc_voltage, &RxData, sizeof(float));
    memcpy(&datasent.fc_current, &RxData[4], sizeof(float));
    break;
  case INT_STACK_PRES_TEMP:
    memcpy(&datasent.internal_stack_pressure, &RxData, sizeof(float));
    memcpy(&datasent.internal_stack_temp, &RxData[4], sizeof(float));
    break;
  case ACCEL_X_Y:
    memcpy(&datasent.x_accel, &RxData, sizeof(float));
    memcpy(&datasent.y_accel, &RxData[4], sizeof(float));
    break;
  case ACCEL_Z_SPEED:
    memcpy(&datasent.z_accel, &RxData, sizeof(float));
    memcpy(&datasent.speed_magnitude, &RxData[4], sizeof(float));
    break;
  case H2_CONC_MV:
    memcpy(&datasent.h2_voltage, &RxData, sizeof(float));
    break;
  case H2_TEMP:
    memcpy(&datasent.h2_temp, &RxData, sizeof(float));
    break;
  case H2_PRESSURE:
    memcpy(&datasent.h2_pressure, &RxData, sizeof(float));
    break;
  case H2_HUMIDITY:
    memcpy(&datasent.h2_humidity, &RxData, sizeof(float));
    break;
  }
  Serial.println("Got CAN");
  // memcpy(&f, &RxData, sizeof(f));
  // sprintf(message, "%lu:%f", (long unsigned)RxHeader.StdId, f);
  // // message.concat(RxData[0]);
  // sendMessage(String(f));
  // Serial.println("Sending");
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
