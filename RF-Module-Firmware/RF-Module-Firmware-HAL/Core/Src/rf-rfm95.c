/*
 * rf-rfm95.c
 *
 *  Created on: Dec 25, 2024
 *      Author: abina
 */
#include "rf-rfm95.h"

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
} rf_op_mode;

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

int rf_initialize_radio(rf_handle_t *rf_handle) {
	if (rf_handle == NULL)
		return 0;

	if (rf_handle->rf_spi_timeout == NULL)
		rf_handle->rf_spi_timeout = 100;

	if (rf_handle->rf_module_identifier == NULL)
		rf_handle->rf_module_identifier = -1;

	rf_reset(rf_handle);

	// check version
	uint8_t version;
	if (!rf_spi_read_register(rf_handle, RegVersion, &version)) {
		printf(
				"\x1b[31;1;9m[RFlib] [%d] [ERROR] \x1b[0m\x1b[31m Module did not return a version; SPI Error\x1b[0m\n\r",
				rf_handle->rf_module_identifier);
		return 0;
	}

	if (RFM9x_VER != version) {

		printf(
				"\x1b[31;1;9m[RFlib] [%d] [ERROR] \x1b[0m\x1b[31m Module did not return the correct version\x1b[0m\n\r",
				rf_handle->rf_module_identifier);

		return 0;
	}

	printf(
			"\x1b[32;1;4m[RFlib] [%d] \x1b[0m Module Available; \x1b[34;1;4mVersion: 0x%x\x1b[0m\n\r",
			rf_handle->rf_module_identifier, version);

	// Module must be placed in sleep mode before switching to lora.
	if (!rf_spi_write_register(rf_handle, RegOpMode,
	RFM95_REGISTER_OP_MODE_SLEEP))
		return 0;
	if (!rf_spi_write_register(rf_handle, RegOpMode,
	RFM95_REGISTER_OP_MODE_LORA_SLEEP))
		return 0;

	// Default interrupt configuration, must be done to prevent DIO5 clock interrupts at 1Mhz
	if (!rf_spi_write_register(rf_handle, RegDioMapping1,
	RFM95_REGISTER_DIO_MAPPING_1_IRQ_FOR_RXDONE))
		return 0;

	if (rf_handle->rf_post_init_clbk != NULL) {
		rf_handle->rf_post_init_clbk();
	}

	// Set module power to 17dbm.
	if (!rf_set_tx_power(rf_handle, 15))
		return 0;

	// Set LNA to the highest gain with 150% boost.

	rf_register_lna_config_t reglna = { .LnaGain = 0b001, .LnaBoostLf = 0b00,
			.reserved = 0, .LnaBoostHf = 0b11 };
	if (!rf_spi_write_register(rf_handle, RegLna, reglna.lna_config))
		return 0;

	// Preamble set to 8 + 4.25 = 12.25 symbols.
	if (!rf_spi_write_register(rf_handle, RegPreambleMsb, 0x00))
		return 0;
	if (!rf_spi_write_register(rf_handle, RegPreambleLsb, 0x08))
		return 0;

	// Set TTN sync word 0x34.
	if (!rf_spi_write_register(rf_handle, RegSyncWord, 0x34))
		return 0;

	// Set up TX and RX FIFO base addresses.
	if (!rf_spi_write_register(rf_handle, RegFifoTxBaseAddr, 0x80))
		return 0;
	if (!rf_spi_write_register(rf_handle, RegFifoRxBaseAddr, 0x00))
		return 0;

	// Maximum payload length of the RFM95 is 64.
	if (!rf_spi_write_register(rf_handle, RegMaxPayloadLength, 64))
		return 0;

	// Let module sleep after initialization.
	if (!rf_spi_write_register(rf_handle, RegOpMode,
	RFM95_REGISTER_OP_MODE_LORA_SLEEP))
		return 0;

	// make sure this is defined
	rf_set_frequency(rf_handle, rf_handle->rf_carrier_frequency);

	rf_register_modem_config_3_t rf_modem_config_3 = { .agc_auto_on = 0b1,
			.low_data_rate_optimize = 0b0 };

	if (!rf_spi_write_register(rf_handle, RegModemConfig3,
			rf_modem_config_3.modem_config_3))
		return 0;

	rf_register_op_mode_config_t rf_op_mode_config = { .access_shared_reg = 0,
			.long_range_mode = 1, .low_frequency_mode_on = 0, .mode = 0b001 };

	// idle in lora mode
	if (!rf_spi_write_register(rf_handle, RegOpMode, rf_op_mode_config.op_mode))
		return 0;

	return 1;

}

int rf_send(rf_handle_t *rf_handle, uint8_t *buffer, uint8_t length_bytes) {

	rf_register_op_mode_config_t current_op_mode;
	if (!rf_spi_read_register(rf_handle, RegOpMode, &current_op_mode.op_mode))
		return 0;

	// check if in transmit mode.
	if (RF_OP_MODE_TX == current_op_mode.mode) {
		printf(
				"\x1b[33;3m[RFlib] [%d] [WARN] Module in TX unable to send.\x1b[0m\n\r",
				rf_handle->rf_module_identifier);
		return 0;
	}
	// here, since the module is not transmitting lets clear the inturrpt flags for tx done.
	rf_register_irq_flags_t irq_flags;
	if (!rf_spi_read_register(rf_handle, RegIrqFlags, &irq_flags.irq_flags))
		return 0;
	// if the flag is still set
	if (irq_flags.tx_done) {
		// then clear it.
		irq_flags.irq_flags = 0b0; // first set all else to 0
		irq_flags.tx_done = 1;
		if (!rf_spi_write_register(rf_handle, RegIrqFlags, irq_flags.irq_flags))
			return 0;
	}

	// put in idle lora
	rf_register_op_mode_config_t idle_op_mode;
	idle_op_mode.long_range_mode = 1;
	idle_op_mode.mode = RF_OP_MODE_STDBY;
	if (!rf_spi_write_register(rf_handle, RegOpMode, idle_op_mode.op_mode))
		return 0;

	// TODO: add a global that configures the implicit or explicit header mode.

	rf_register_modem_config_1_t modem_config_1;
	if (!rf_spi_read_register(rf_handle, RegModemConfig1,
			&modem_config_1.modem_config_1))
		return 0;

	// Default to implicit header.
	modem_config_1.implicit_header_mode_on = 1;
	if (!rf_spi_write_register(rf_handle, RegModemConfig1,
			modem_config_1.modem_config_1))
		return 0;

	// Reset fifo address and payload length

	if (!rf_spi_write_register(rf_handle, RegFifoAddrPtr, 0))
		return 0;

	if (!rf_spi_write_register(rf_handle, RegPayloadLength, 0)) // set to 0 for now.
		return 0;

	// add data

	// check size TODO: optimize this since it is constant.
	uint8_t max_payload_length = 0;
	if (!rf_spi_read_register(rf_handle, RegMaxPayloadLength,
			&max_payload_length))
		return 0;

	if (max_payload_length < length_bytes) {
		printf(
				"\x1b[31;4;3;1m[RFlib] [%d] [ERROR] Max payload length exceeded with %d; max is %d.\x1b[0m\n\r",
				rf_handle->rf_module_identifier, max_payload_length,
				length_bytes);
	}

	uint8_t current_payload_length = 0;
	if (!rf_spi_read_register(rf_handle, RegPayloadLength,
			&current_payload_length))
		return 0;

	// never should happen cuz we reset the payload length above
	if (current_payload_length + length_bytes > max_payload_length) {
		printf(
				"\x1b[31;4;3;1m[RFlib] [%d] [ERROR] Old buffer not clear and adding current bytes exceeds max payload %d, prev %d, new %d.\x1b[0m\n\r",
				rf_handle->rf_module_identifier, max_payload_length,
				current_payload_length, length_bytes);
	}

	// fill fifo
	// write data to module
	for (int i = 0; i < length_bytes; i++) {
		if (!rf_spi_write_register(rf_handle, RegFifo, buffer[i])) {
			printf(
					"\x1b[31;4;3;1m[RFlib] [%d] [ERROR] Buffer fifo write error\x1b[0m\n\r",
					rf_handle->rf_module_identifier);
			return 0;
		}
	}

	// update the payload length
	if (!rf_spi_write_register(rf_handle, RegPayloadLength, length_bytes))
		return 0;

	// end packet and send
	rf_register_dio_mapping_1_config_t dio_mapping_1;

	//TODO: use dio0 inturrupt

	// put in tx mode
	rf_register_op_mode_config_t tx_mode_config;
	tx_mode_config.long_range_mode = 1;
	tx_mode_config.mode = RF_OP_MODE_TX;

	if (!rf_spi_write_register(rf_handle, RegOpMode, tx_mode_config.op_mode))
		return 0;

	// Blocking

	rf_register_irq_flags_t tx_done_flags;
	if (!rf_spi_read_register(rf_handle, RegIrqFlags, &tx_done_flags.irq_flags))
		return 0;
	while (!tx_done_flags.tx_done) {
		HAL_Delay(1);
		if (!rf_spi_read_register(rf_handle, RegIrqFlags,
				&tx_done_flags.irq_flags))
			return 0;
	}

	// done sending
	// clear flags
	tx_done_flags.irq_flags = 0; // 0 out the rest
	tx_done_flags.tx_done = 1; // set the tx done bit to clear.

	// clear the tx done flag for next packet.
	if (!rf_spi_write_register(rf_handle, RegIrqFlags, tx_done_flags.irq_flags))
		return 0;

	return 1;
}

int rf_set_frequency(rf_handle_t *rf_handle, uint32_t rf_carrier_frequency) {

// FQ = (FRF * 32 Mhz) / (2 ^ 19)
	uint64_t frf = ((uint64_t) rf_carrier_frequency << 19) / 32000000;

	if (!rf_spi_write_register(rf_handle, RegFrfMsb, (uint8_t) (frf >> 16)))
		return 0;
	if (!rf_spi_write_register(rf_handle, RegFrfMid, (uint8_t) (frf >> 8)))
		return 0;
	if (!rf_spi_write_register(rf_handle, RegFrfLsb, (uint8_t) (frf >> 0)))
		return 0;

	return 1;

}

int rf_set_tx_power(rf_handle_t *rf_handle, uint8_t rf_power_dbm) {

	if (!((rf_power_dbm >= 2 && rf_power_dbm <= 17) || (rf_power_dbm == 20))) {
		printf(
				"\x1b[31;1;9m[RFlib] [%d] [ERROR] \x1b[0m\x1b[31m Unable to set power to %d out of range.\x1b[0m\n\r",
				rf_handle->rf_module_identifier, rf_power_dbm);
		return 0;
	}

	rf_register_pa_config_t reg_pa_config;
	uint8_t reg_pa_dac_config = 0;

	if (rf_power_dbm >= 2 || rf_power_dbm <= 17) {
		reg_pa_config.max_power = 7;
		reg_pa_config.pa_select = 1;
		reg_pa_config.output_power = (rf_power_dbm - 2);
		reg_pa_dac_config = RFM95_REGISTER_PA_DAC_LOW_POWER;
	} else if (rf_power_dbm == 20) {
		reg_pa_config.max_power = 7;
		reg_pa_config.pa_select = 1;
		reg_pa_config.output_power = 15;
		reg_pa_dac_config = RFM95_REGISTER_PA_DAC_HIGH_POWER;
	}

	if (!rf_spi_write_register(rf_handle, RegPaConfig,
			reg_pa_config.pa_config)) {
		printf(
				"\x1b[31;1;9m[RFlib] [%d] [ERROR] \x1b[0m\x1b[31m Unable to set power to %d SPI write error.\x1b[0m\n\r",
				rf_handle->rf_module_identifier, rf_power_dbm);
		return 0;
	}
	if (!rf_spi_write_register(rf_handle, RegPaDac, reg_pa_dac_config)) {
		printf(
				"\x1b[31;1;9m[RFlib] [%d] [ERROR] \x1b[0m\x1b[31m Unable to set power to %d SPI write error.\x1b[0m\n\r",
				rf_handle->rf_module_identifier, rf_power_dbm);
		return 0;
	}
	printf("\x1b[32;4;1;3m[RFlib] [%d] [ERROR] Set power to %d.\x1b[0m\n\r",
			rf_handle->rf_module_identifier, rf_power_dbm);
	return 1;

}

int rf_inturrupt_clbk(rf_handle_t *rf_handle, rf_interrupt_t rf_inturrupt) {

	return 1;

}

int rf_spi_read_register(rf_handle_t *rf_handle, uint8_t rf_register_address,
		uint8_t *rf_register_result) {

// 0 the MSB since that is the wnr bit. we are reading so it must be 0.
	uint8_t prep_register = rf_register_address & 0x7f;

	HAL_GPIO_WritePin(rf_handle->rf_nss_port, rf_handle->rf_nss_pin,
			GPIO_PIN_RESET);

	if (HAL_SPI_Transmit(rf_handle->rf_spi_handle, &prep_register, 1,
			rf_handle->rf_spi_timeout) != HAL_OK) {
		printf(
				"\x1b[31;1;4m[RFlib] [%d] [ERROR]\x1b[0m : Failed to read register \x1b[33;4;1;3m0x%x\x1b[0m\n\r",
				rf_handle->rf_module_identifier, rf_register_address);
		return 0;
	}

	if (HAL_SPI_Receive(rf_handle->rf_spi_handle, rf_register_result, 1,
			rf_handle->rf_spi_timeout) != HAL_OK) {
		printf(
				"\x1b[31;1;4m[RFlib] [%d] [ERROR]\x1b[0m : Failed to read register \x1b[33;4;1;3m0x%x\x1b[0m\n\r",
				rf_handle->rf_module_identifier, rf_register_address);
		return 0;
	}

	HAL_GPIO_WritePin(rf_handle->rf_nss_port, rf_handle->rf_nss_pin,
			GPIO_PIN_SET);
	return 1;
}

int rf_spi_write_register(rf_handle_t *rf_handle, uint8_t rf_register_address,
		uint8_t rf_register_value) {
// 1 the MSB since that is the wnr bit. we are writing so it must be 1.
	uint8_t prep_register_buffer[2] = { ((uint8_t) rf_register_address | 0x80u),
			rf_register_value };

	HAL_GPIO_WritePin(rf_handle->rf_nss_port, rf_handle->rf_nss_pin,
			GPIO_PIN_RESET);

	if (HAL_SPI_Transmit(rf_handle->rf_spi_handle, &prep_register_buffer, 1,
			rf_handle->rf_spi_timeout) != HAL_OK) {
		printf(
				"\x1b[31;1;4m[RFlib] [%d] [ERROR]\x1b[0m : Failed to set register \x1b[33;4;1;3m0x%x : 0x%x\x1b[0m\n\r",
				rf_handle->rf_module_identifier, rf_register_address,
				rf_register_value);
		return 0;
	}

	HAL_GPIO_WritePin(rf_handle->rf_nss_port, rf_handle->rf_nss_pin,
			GPIO_PIN_SET);

	return 1;

}

int rf_reset(rf_handle_t *rf_handle) {

	printf("\x1b[33;1;3;4m[RFlib] [%d] [INFO] : Reseting module\x1b[0m\n\r",
			rf_handle->rf_module_identifier);

	HAL_GPIO_WritePin(rf_handle->rf_nreset_port, rf_handle->rf_nreset_pin,
			GPIO_PIN_RESET);
	rf_handle->rf_delay_func(100);
	HAL_GPIO_WritePin(rf_handle->rf_nreset_port, rf_handle->rf_nreset_pin,
			GPIO_PIN_SET);
	rf_handle->rf_delay_func(100);

	return 1;

}
