/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/
#include <SPI.h>              // include libraries
#include <LoRa.h>
HardwareSerial Serial2(USART2);
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PA10, OUTPUT);
  // Serial.begin(9600);               // initialize serial
  // while (!Serial);
  Serial2.begin(115200);               // initialize serial
  // while (!serial1);
  Serial2.println("LoRa Dump Registers");
  SPI.setSSEL(PA4);
  SPI.setMISO(PA6);
  SPI.setMOSI(PA7);
  SPI.setSCLK(PA5);
  LoRa.setSPI(SPI);
  pinMode(PB5, OUTPUT);
  pinMode(PA4, OUTPUT);
  digitalWrite(PA4, HIGH);
  // perform reset

  digitalWrite(PB5, HIGH);
  delay(100);
  digitalWrite(PB5, LOW);
  delay(10);
  digitalWrite(PB5, HIGH);
  delay(100);
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(PA4, PB5, PB0); // set CS, reset, IRQ pin
  digitalWrite(PA10, HIGH);
  // if (!LoRa.begin(868E6)) {         // initialize ratio at 915 MHz
  //   Serial2.println("LoRa init failed. Check your connections.");
  //   while (true);                   // if failed, do nothing
  // }
  //digitalWrite(PB5, LOW);
  digitalWrite(PA10, LOW);
  
  digitalWrite(PB5, LOW);
  delay(10);
  digitalWrite(PB5, HIGH);
  delay(100);

  SPI.begin();
  //LoRa.dumpRegisters(Serial2);
}
uint8_t res = 0, ver = 0, address = 0x42 & 0x7f;
// the loop function runs over and over again forever
void loop() {
  
 
  digitalWrite(PA4, LOW);
  SPI.beginTransaction(SPISettings(1E6, MSBFIRST, SPI_MODE0));
  res = SPI.transfer(address);
  //res = SPI.transfer(0);
  Serial2.print("Version: ");
  Serial2.println(res, HEX);
  res = 0;
  //Serial2.println(ver, HEX);
  SPI.endTransaction();
  digitalWrite(PA4, HIGH);
  //LoRa.dumpRegisters(Serial2);

  digitalWrite(PB5, LOW);
  delay(10);
  digitalWrite(PB5, HIGH);
  delay(100);

  digitalWrite(PA10, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(500);                      // wait for a second
  digitalWrite(PA10, LOW);   // turn the LED off by making the voltage LOW
  delay(500);                      // wait for a second
}

// /**
//  * @brief System Clock Configuration
//  * @retval None
//  */
// void SystemClock_Config(void) {
// 	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
// 	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

// 	/** Configure the main internal regulator output voltage
// 	 */
// 	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
// 			!= HAL_OK) {
// 		Error_Handler();
// 	}

// 	/** Initializes the RCC Oscillators according to the specified parameters
// 	 * in the RCC_OscInitTypeDef structure.
// 	 */
// 	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
// 	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
// 	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
// 	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
// 	RCC_OscInitStruct.PLL.PLLM = 1;
// 	RCC_OscInitStruct.PLL.PLLN = 20;
// 	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
// 	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
// 	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
// 	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
// 		Error_Handler();
// 	}

// 	/** Initializes the CPU, AHB and APB buses clocks
// 	 */
// 	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
// 			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
// 	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
// 	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
// 	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
// 	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

// 	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
// 		Error_Handler();
// 	}
// 	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_16);
// }