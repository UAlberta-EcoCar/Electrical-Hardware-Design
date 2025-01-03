/*
 * rf-rfm95-resgisters.h
 *
 *  Created on: Dec 30, 2024
 *      Author: abina
 */

#ifndef INC_RF_RFM95_RESGISTERS_H_
#define INC_RF_RFM95_RESGISTERS_H_

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
#define RegFifoRxCurrentAddr 0x10
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
// Sensitivity optimization for 500kHz bandwidth
#define RegHighBwOptimize1 0x36
// LoRa detection threshold for SF6
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

#define RFM9x_VER 0x12

#define RFM95_REGISTER_OP_MODE_SLEEP                            0x00
#define RFM95_REGISTER_OP_MODE_LORA_SLEEP                       0x80
#define RFM95_REGISTER_OP_MODE_LORA_STANDBY                     0x81
#define RFM95_REGISTER_OP_MODE_LORA_TX                          0x83
#define RFM95_REGISTER_OP_MODE_LORA_RX_SINGLE                   0x86

#define RFM95_REGISTER_PA_DAC_LOW_POWER                         0x84
#define RFM95_REGISTER_PA_DAC_HIGH_POWER                        0x87

#define RFM95_REGISTER_DIO_MAPPING_1_IRQ_FOR_TXDONE             0x40
#define RFM95_REGISTER_DIO_MAPPING_1_IRQ_FOR_RXDONE             0x00

#define RFM95_REGISTER_INVERT_IQ_1_TX                    		0x27
#define RFM95_REGISTER_INVERT_IQ_2_TX							0x1d

#define RFM95_REGISTER_INVERT_IQ_1_RX                    		0x67
#define RFM95_REGISTER_INVERT_IQ_2_RX							0x19

typedef struct {
	union {
		struct {
			uint8_t output_power :4;
			uint8_t max_power :3;
			uint8_t pa_select :1;
		};
		uint8_t pa_config;
	};
} rf_register_pa_config_t;

typedef struct {
	union {
		struct {
			uint8_t LnaBoostHf :2;
			uint8_t reserved :1;
			uint8_t LnaBoostLf :2;
			uint8_t LnaGain :3;
		};
		uint8_t lna_config;
	};
} rf_register_lna_config_t;

typedef struct {
	union {
		struct {
			uint8_t reserved :2;
			uint8_t agc_auto_on :1;
			uint8_t low_data_rate_optimize :1;
		// 7-4 unused.
		};
		uint8_t modem_config_3;
	};
} rf_register_modem_config_3_t;

typedef struct {
	union {
		struct {
			uint8_t symb_timeout :2;
			uint8_t rx_payload_crc_on :1;
			uint8_t tx_continous_mode :1;
			uint8_t spreading_factor :4;
		};
		uint8_t modem_config_2;
	};
} rf_register_modem_config_2_t;

typedef enum {
	RF_BW_7K8 = 0b0000,
	RF_BW_10K4 = 0b0001,
	RF_BW_15K6 = 0b0010,
	RF_BW_20K8 = 0b0011,
	RF_BW_31K25 = 0b0100,
	RF_BW_41K7 = 0b0101,
	RF_BW_62K5 = 0b0110,
	RF_BW_125K = 0b0111,
	RF_BW_250K = 0b1000,
	RF_BW_500K = 0b1001,
} rf_bandwidth_t;

typedef struct {
	union {
		struct {
			uint8_t implicit_header_mode_on :1;
			uint8_t coding_rate :3;
			uint8_t bandwidth :4;
		};
		uint8_t modem_config_1;
	};
} rf_register_modem_config_1_t;

typedef enum {
	RF_OP_MODE_SLEEP = 0b000,
	RF_OP_MODE_STDBY = 0b001,
	RF_OP_MODE_FSTX = 0b010,
	RF_OP_MODE_TX = 0b011,
	RF_OP_MODE_FSRX = 0b100,
	RF_OP_MODE_RXCONTINUOUS = 0b101,
	RF_OP_MODE_RX_SINGLE = 0b110,
	RF_OP_MODE_CAD = 0b111
} rf_op_mode_t;

typedef struct {
	union {
		struct {
			/**
			 * Device modes
			 * 000 -> SLEEP
			 * 001 -> STDBY
			 * 010 -> Frequency synthesis TX (FSTX)
			 * 011 -> Transmit (TX)
			 * 100 -> Frequency synthesis RX (FSRX)
			 * 101 -> Receive continuous (RXCONTINUOUS)
			 * 110 -> receive single (RXSINGLE)
			 * 111 -> Channel activity detection (CAD)
			 */
			uint8_t mode :3;
			uint8_t low_frequency_mode_on :1;
			uint8_t reserved :2;
			uint8_t access_shared_reg :1;
			uint8_t long_range_mode :1;
		};
		uint8_t op_mode;
	};
} rf_register_op_mode_config_t;

typedef struct {
	union {
		struct {
			uint8_t cad_detected :1;
			uint8_t fhss_change_channel :1;
			uint8_t cad_done :1;
			uint8_t tx_done :1;
			uint8_t valid_header :1;
			uint8_t payload_crc_error :1;
			uint8_t rx_done :1;
			uint8_t rx_timeout :1;
		};
		uint8_t irq_flags;
	};
} rf_register_irq_flags_t;

typedef struct {
	union {
		struct {
			uint8_t dio_3_mapping :2;
			uint8_t dio_2_mapping :2;
			uint8_t dio_1_mapping :2;
			uint8_t dio_0_mapping :2;
		};
		uint8_t dio_mapping_1;
	};
} rf_register_dio_mapping_1_config_t;

typedef struct {
	union {
		struct {
			uint8_t map_preamble_detect :1;
			uint8_t reserved :3;
			uint8_t dio_5_mapping :2;
			uint8_t dio_4_mapping :2;
		};
		uint8_t dio_mapping_2;
	};
} rf_register_dio_mapping_2_config_t;

typedef struct {
	union {
		struct {
			/**
			 * LoRa Detection Optimize
			 * 0x03 -> SF7 to SF12
			 * 0x05 -> SF6
			 */
			uint8_t detection_optimize :3;
			uint8_t reserved :5;
		};
		uint8_t reg_detection_optimize;
	};
} rf_register_detect_optimize_config_t;

#endif /* INC_RF_RFM95_RESGISTERS_H_ */
