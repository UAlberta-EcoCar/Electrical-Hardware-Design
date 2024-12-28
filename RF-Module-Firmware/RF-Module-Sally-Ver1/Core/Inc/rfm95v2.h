#pragma once

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
#define RegPktRssiValue 0x1A =
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