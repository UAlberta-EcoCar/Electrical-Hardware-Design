/*
 * rf-rfm95.h
 *
 *  Created on: Dec 24, 2024
 *      Author: abina
 */

#ifndef INC_RF_RFM95_H_
#define INC_RF_RFM95_H_

#include <stm32l4xx_hal.h>
#include <stdint.h>
#include <stdio.h>
/**
 * LoRa mode register definitions
 */

// FIFO read/write access
#define RegFifo 0x00
// Operating mode & LoRaTM / FSK selection
#define RegOpMode 0x01
// RF Carrier Frequency, Most Significant Bits
#define RegFrfMsb 0x06
// RF Carrier Frequency, Intermediate Bits
#define RegFrfMid 0x07
// RF Carrier Frequency, Least Significant Bits
#define RegFrfLsb 0x08
// PAselection and Output Power control
#define RegPaConfig 0x09
// Control of PAramp time, low phase noisePLL
#define RegPaRamp 0x0A
// Over Current Protection control
#define RegOcp 0x0B
// LNA settings
#define RegLna 0x0C
// LNA settings
#define RegFifoAddrPtr 0x0D
// Start Tx data
#define RegFifoTxBaseAddr 0x0E
// Start Rx data
#define RegFifoRxBaseAddr 0x0F
// Start address of last packet received
#define FifoRxCurrentAddr 0x10
// Optional IRQ flag mask
#define RegIrqFlagsMask 0x11
// IRQ flags
#define RegIrqFlags 0x12
// Number of received bytes
#define RegRxNbBytes 0x13
// Number of validheaders received
#define RegRxHeaderCntValueMsb 0x14
#define RegRxHeaderCntValueLsb 0x15
// Number of validpackets received
#define RegRxPacketCntValueMsb 0x16
#define RegRxPacketCntValueLsb 0x17
// Live LoRaTMmodemstatus
#define RegModemSta 0x18
// Estimation of last packet SNR
#define RegPktSnrValue 0x19
// RSSI of last packet
#define RegPktRssiValue 0x1A
// Current RSSI
#define RegRssiValue 0x1B
// FHSS start channel
#define RegHopChannel 0x1C
// ModemPHYconfig1
#define RegModemConfig1 0x1D
// ModemPHYconfig2
#define RegModemConfig2 0x1E
// ModemPHYconfig2
#define RegSymbTimeoutLsb 0x1F
// Size of preamble
#define RegPreambleMsb 0x20

#define RegPreambleLsb 0x21
// LoRaTM payload length
#define RegPayloadLength 0x22
// LoRaTM maximum payload length
#define RegMaxPayloadLength 0x23
// FHSS Hop period
#define RegHopPeriod 0x24
// Address of last byte written in FIFO
#define RegFifoRxByteAddr 0x25
// ModemPHYconfig3
#define RegModemConfig3 0x26
// Estimated frequency error
#define RegFeiMsb 0x28
#define RegFeiMid 0x29
#define RegFeiLsb 0x2A
// WidebandRSSI measurement
#define RegRssiWideband 0x2C
// Optimize receiver
#define RegIfFreq1 0x2F
#define RegIfFreq2 0x30
// LoRa detection Optimize for SF6
#define RegDetectOptimize 0x31
// Invert LoRa I and Q signals
#define RegInvertIQ 0x33
// Sensitivity optimisation for 500kHz bandwidth
#define RegHighBwOptimize1 0x36
// LoRa detectionthresholdfor SF6
#define RegDetectionThreshold 0x37
// LoRa SyncWord
#define RegSyncWord 0x39
// Sensitivity optimisation for 500kHz bandwidth
#define RegHighBwOptimize2 0x3A
// Optimize for inverted IQ
#define RegInvertIQ2 0x3B
// Mapping of pins DIO0 to DIO3
#define RegDioMapping1 0x40
// Mapping of pins DIO4 and DIO5, ClkOut frequency
#define RegDioMapping2 0x41
// Semtech ID relating the silicon revision
#define RegVersion 0x42
// TCXO or XTAL input setting
#define RegTcxo 0x4B
// Higher power settings of thePA
#define RegPaDac 0x4D
// Stored temperature during the former IQ Calibration
#define RegFormerTemp 0x5B
// Adjustment of the AGC thresholds
#define RegAgcRef 0x61
#define RegAgcThresh1 0x62
#define RegAgcThresh2 0x63
#define RegAgcThresh3 0x64
// Control of the PLLbandwidth
#define RegPll 0x70

// PA config
#define PA_BOOST                 0x80

// IRQ masks
#define IRQ_TX_DONE_MASK           0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20
#define IRQ_RX_DONE_MASK           0x40
#define IRQ_CAD_DONE_MASK          0x04
#define IRQ_CAD_DETECTED_MASK      0x01

#define RF_MID_BAND_THRESHOLD    525E6
#define RSSI_OFFSET_HF_PORT      157
#define RSSI_OFFSET_LF_PORT      164

#define MAX_PKT_LENGTH           255

typedef enum {
	RFM95_INTERRUPT_DIO0, RFM95_INTERRUPT_DIO1, RFM95_INTERRUPT_DIO5

} rf_interrupt_t;

typedef enum {
	RFM95_RECEIVE_MODE_NONE,
	RFM95_RECEIVE_MODE_RX1_ONLY,
	RFM95_RECEIVE_MODE_RX12,
} rf_receive_mode_t;

#define RFM95_INTERRUPT_COUNT 3

typedef void (*rf_delay_func_t)(uint32_t);

typedef void (*rf_post_init_func_t)();

typedef struct {
	SPI_HandleTypeDef *rf_spi_handle;

	uint32_t rf_spi_timeout;

	GPIO_TypeDef *rf_nss_port;

	uint16_t rf_nss_pin;

	GPIO_TypeDef *rf_nreset_port;

	uint16_t rf_nreset_pin;

	rf_post_init_func_t rf_post_init_clbk;

	rf_delay_func_t rf_delay_func;

	uint8_t rf_module_identifier;

	uint32_t rf_carrier_frequency;

} rf_handle_t;

int rf_initialize_radio(rf_handle_t *rf_handle);

int rf_set_tx_power(rf_handle_t *rf_handle, uint8_t rf_power_dbm);

int rf_inturrupt_clbk(rf_handle_t *rf_handle, rf_interrupt_t rf_inturrupt);

int rf_spi_read_register(rf_handle_t *rf_handle, uint8_t rf_register_address,
		uint8_t *rf_register_result);

int rf_spi_write_register(rf_handle_t *rf_handle, uint8_t rf_register_address,
		uint8_t rf_register_value);

int rf_reset(rf_handle_t *rf_handle);

int rf_set_frequency(rf_handle_t *rf_handle, uint32_t rf_carrier_frequency);

int rf_send(rf_handle_t *rf_handle, uint8_t *buffer, uint8_t length_bytes);

#endif /* INC_RF_RFM95_H_ */
