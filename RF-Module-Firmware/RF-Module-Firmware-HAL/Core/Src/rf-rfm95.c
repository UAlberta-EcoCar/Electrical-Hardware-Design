/*
 * rf-rfm95.c
 *
 *  Created on: Dec 25, 2024
 *      Author: abina
 */
#include "rf-rfm95.h"

#include "ansi-codes.h"

#define LOG_ERROR(M, ...) printf("[Abi's RFlib]" SETFG_RED "[ERROR] (%s:%d) " M GR_RESET "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(M, ...) printf("[Abi's RFlib]" SETFG_YELLOW "[WARN] " M GR_RESET "\r\n", ##__VA_ARGS__)

#define LOG_INFO(M, ...) printf("[Abi's RFlib][INFO] " M "\r\n", ##__VA_ARGS__)

int rf_set_op_mode(rf_handle_t *rf_handle, rf_op_mode_t rf_op_mode) {

	rf_register_op_mode_config_t opmode = { 0 };

	if (!rf_spi_read_register(rf_handle, RegOpMode, &opmode.op_mode)) {
		LOG_ERROR("SPI ERROR Setting op mode");
		return 0;
	}

//	opmode.low_frequency_mode_on = 0;

//	opmode.access_shared_reg = 0;

//	opmode.long_range_mode = 1; // just assume we are long range mode only.

	opmode.mode = rf_op_mode;

	if (!rf_spi_write_register(rf_handle, RegOpMode, opmode.op_mode)) {
		LOG_ERROR("SPI ERROR Setting op mode");
		return 0;
	}

	opmode.op_mode = 0;

	if (!rf_spi_read_register(rf_handle, RegOpMode, &opmode.op_mode)) {
		LOG_ERROR("SPI ERROR Setting op mode");
		return 0;
	}

	return 1;

}

int rf_initialize_radio(rf_handle_t *rf_handle) {
	if (rf_handle == NULL)
		return 0;

	if (rf_handle->rf_spi_timeout == NULL)
		rf_handle->rf_spi_timeout = 100;

	if (rf_handle->rf_module_identifier == NULL)
		rf_handle->rf_module_identifier = -1;

	if (rf_handle->rf_carrier_frequency == NULL) {
		LOG_ERROR("Carrier Frequency not defined.");
		return 0;
	}

	printf("\r\n");

	LOG_INFO(SETFG_GREEN "Initializing module" GR_RESET,
			rf_handle->rf_module_identifier);

	rf_reset(rf_handle);

// check version
	uint8_t version;
	if (!rf_spi_read_register(rf_handle, RegVersion, &version)) {
		LOG_ERROR("Module did not return a version; SPI Error");
		return 0;
	}

	if (RFM9x_VER != version) {
		LOG_ERROR("Module did not return the correct version");
		return 0;
	}

	LOG_INFO("Module Available; \x1b[34;1;4mVersion: 0x%x\x1b[0m", version);

// Module must be placed in sleep mode before switching to lora.
	rf_register_op_mode_config_t opmode = { 0 };
	opmode.long_range_mode = 1;
	opmode.mode = RF_OP_MODE_SLEEP;
	if (!rf_spi_write_register(rf_handle, RegOpMode, opmode.op_mode))
		return 0;

// Default interrupt configuration, must be done to prevent DIO5 clock interrupts at 1Mhz
// Table 17
//	rf_register_dio_mapping_1_config_t diomapping1 = { 0 };
//	if (!rf_spi_write_register(rf_handle, RegDioMapping1,
//			diomapping1.dio_mapping_1))
//		return 0;
//
//	rf_register_dio_mapping_2_config_t diomapping2 = { 0 };
//	if (!rf_spi_write_register(rf_handle, RegDioMapping2,
//			diomapping2.dio_mapping_2))
//		return 0;

	if (rf_handle->rf_post_init_clbk != NULL) {
		rf_handle->rf_post_init_clbk();
	}

// Set up TX and RX FIFO base addresses.
	if (!rf_spi_write_register(rf_handle, RegFifoTxBaseAddr, 0x00)) // previously 0x80
		return 0;
	if (!rf_spi_write_register(rf_handle, RegFifoRxBaseAddr, 0x00))
		return 0;

// Maximum payload length of the RFM95 is 64.
	if (!rf_spi_write_register(rf_handle, RegMaxPayloadLength, 64))
		return 0;

// make sure this is defined
	rf_set_frequency(rf_handle, rf_handle->rf_carrier_frequency);

// Set LNA to the highest gain with 150% boost.
	rf_register_lna_config_t lnaboost = { 0 };
	if (!rf_spi_read_register(rf_handle, RegLna, &lnaboost.lna_config))
		return 0;
	lnaboost.LnaBoostHf = 0b11;
	if (!rf_spi_write_register(rf_handle, RegLna, lnaboost.lna_config))
		return 0;

// Set auto AGC
	rf_register_modem_config_3_t autoagc = { 0 };
	autoagc.agc_auto_on = 1;
	if (!rf_spi_write_register(rf_handle, RegModemConfig3,
			autoagc.modem_config_3))
		return 0;

// Set module power to 17dbm.
	if (!rf_set_tx_power(rf_handle, 10))
		return 0;

// IDLE
//	rf_register_op_mode_config_t idle = { 0 };
//	idle.long_range_mode = 1;
//	idle.mode = RF_OP_MODE_STDBY;
//	if (!rf_spi_write_register(rf_handle, RegOpMode, idle.op_mode))
//		return 0;

	if (!rf_set_op_mode(rf_handle, RF_OP_MODE_STDBY)) {
		LOG_ERROR("OP Mode Init error");
		return 0;
	}

	return 1;
}

int rf_send(rf_handle_t *rf_handle, uint8_t *buffer, uint8_t length_bytes) {

	rf_register_op_mode_config_t current_op_mode = { 0 };
	if (!rf_spi_read_register(rf_handle, RegOpMode, &current_op_mode.op_mode))
		return 0;

// check if in transmit mode.
	if (RF_OP_MODE_TX == current_op_mode.mode) {
		LOG_WARN("Module in TX unable to send.");
		return 0;
	}
// here, since the module is not transmitting lets clear the inturrpt flags for tx done.
	rf_register_irq_flags_t irq_flags = { 0 };
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
	rf_register_op_mode_config_t idle_op_mode = { 0 };
	idle_op_mode.long_range_mode = 1;
	idle_op_mode.mode = RF_OP_MODE_STDBY;
	if (!rf_spi_write_register(rf_handle, RegOpMode, idle_op_mode.op_mode))
		return 0;

// TODO: add a global that configures the implicit or explicit header mode.

	rf_register_modem_config_1_t modem_config_1 = { 0 };
	if (!rf_spi_read_register(rf_handle, RegModemConfig1,
			&modem_config_1.modem_config_1))
		return 0;

// Default to explicit header.
	modem_config_1.implicit_header_mode_on = 0;
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
				"\x1b[31;4;3;1m[Abi's RFlib] [%d] [ERROR] Max payload length exceeded with %d; max is %d.\x1b[0m\r\n",
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
				"\x1b[31;4;3;1m[Abi's RFlib] [%d] [ERROR] Old buffer not clear and adding current bytes exceeds max payload %d, prev %d, new %d.\x1b[0m\r\n",
				rf_handle->rf_module_identifier, max_payload_length,
				current_payload_length, length_bytes);
	}

// fill fifo
// write data to module
	for (int i = 0; i < length_bytes; i++) {
		if (!rf_spi_write_register(rf_handle, RegFifo, buffer[i])) {
			LOG_ERROR("FIFO Write ERROR");
			return 0;
		}
	}

// update the payload length
	if (!rf_spi_write_register(rf_handle, RegPayloadLength, length_bytes))
		return 0;

// end packet and send
	rf_register_dio_mapping_1_config_t dio_mapping_1 = { 0 };

// TODO: use dio0 inturrupt

// put in tx mode
	rf_register_op_mode_config_t tx_mode_config = { 0 };
	tx_mode_config.long_range_mode = 1;
	tx_mode_config.mode = RF_OP_MODE_TX;

	if (!rf_spi_write_register(rf_handle, RegOpMode, tx_mode_config.op_mode))
		return 0;

// Blocking

	rf_register_irq_flags_t tx_done_flags = { 0 };
	if (!rf_spi_read_register(rf_handle, RegIrqFlags, &tx_done_flags.irq_flags))
		return 0;

	while (!tx_done_flags.tx_done) {
		rf_handle->rf_delay_func(10);
		if (!rf_spi_read_register(rf_handle, RegIrqFlags,
				&tx_done_flags.irq_flags))
			return 0;
	}

// done sending
// clear flags
	tx_done_flags.irq_flags = 0; // 0 out the rest
	tx_done_flags.tx_done = 1;	 // set the tx done bit to clear.

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

		LOG_ERROR("Unable to set power to %d out of range.", rf_power_dbm);
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

		LOG_ERROR("Unable to set power to %d SPI write error.", rf_power_dbm);
		return 0;
	}
	if (!rf_spi_write_register(rf_handle, RegPaDac, reg_pa_dac_config)) {

		LOG_ERROR("Unable to set power to %d SPI write error.", rf_power_dbm);
		return 0;
	}

	rf_set_ocp(rf_handle, 240);

	LOG_INFO("Set power to %d.", rf_power_dbm);

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
		LOG_ERROR("Failed to read register 0x%x", rf_register_address);
		return 0;
	}

	if (HAL_SPI_Receive(rf_handle->rf_spi_handle, rf_register_result, 1,
			rf_handle->rf_spi_timeout) != HAL_OK) {
		LOG_ERROR("Failed to read register 0x%x", rf_register_address);
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

	if (HAL_SPI_Transmit(rf_handle->rf_spi_handle, prep_register_buffer, 2,
			rf_handle->rf_spi_timeout) != HAL_OK) {
		LOG_ERROR("Failed to set register 0x%x : 0x%x", rf_register_address,
				rf_register_value);
		return 0;
	}

	HAL_GPIO_WritePin(rf_handle->rf_nss_port, rf_handle->rf_nss_pin,
			GPIO_PIN_SET);

	return 1;
}

int rf_reset(rf_handle_t *rf_handle) {

//	printf(
//			"\x1b[33;1;3;4m[Abi's RFlib] [%d] [WARN] : Reseting module\x1b[0m\r\n",
//			rf_handle->rf_module_identifier);

	LOG_WARN("Reseting module");

	HAL_GPIO_WritePin(rf_handle->rf_nreset_port, rf_handle->rf_nreset_pin,
			GPIO_PIN_RESET);
	rf_handle->rf_delay_func(100);
	HAL_GPIO_WritePin(rf_handle->rf_nreset_port, rf_handle->rf_nreset_pin,
			GPIO_PIN_SET);
	rf_handle->rf_delay_func(100);

	return 1;
}

int rf_listen(rf_handle_t *rf_handle) {

	rf_register_dio_mapping_1_config_t rxdoneintrp = { 0 };

	rxdoneintrp.dio_0_mapping = 0b00; // Set RXDone inturrupt on.

	if (!rf_spi_write_register(rf_handle, RegDioMapping1,
			rxdoneintrp.dio_mapping_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

// explicit header mode

	rf_register_modem_config_1_t explicitheadermode = { 0 };

	if (!rf_spi_read_register(rf_handle, RegModemConfig1,
			&explicitheadermode.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	explicitheadermode.implicit_header_mode_on = 0;

	if (!rf_spi_write_register(rf_handle, RegModemConfig1,
			explicitheadermode.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	rf_register_op_mode_config_t rxcont = { 0 };

	rxcont.long_range_mode = 1;
	rxcont.mode = RF_OP_MODE_RXCONTINUOUS;

	if (!rf_spi_write_register(rf_handle, RegOpMode, rxcont.op_mode)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	LOG_INFO("Listening on %d Hz Explicit", rf_handle->rf_carrier_frequency);

	return 1;
}

int rf_listen_implicit(rf_handle_t *rf_handle, uint8_t size) {
	if (size <= 0) {
		LOG_ERROR("Listen size must be 1 byte or larger");
		return 0;
	}

	rf_register_dio_mapping_1_config_t diomapping1 = { 0 };

// get the current modem config then change to implicit.

	if (!rf_spi_read_register(rf_handle, RegModemConfig1,
			&diomapping1.dio_mapping_1)) {
		LOG_ERROR("SPI Error");
		return 0;
	}

	diomapping1.dio_0_mapping = 0b00;

	if (!rf_spi_write_register(rf_handle, RegDioMapping1,
			diomapping1.dio_mapping_1)) { // DIO0 => RXDONE
		LOG_ERROR("SPI Error");
		return 0;
	}

	rf_register_modem_config_1_t implicit_header_mode = { 0 };
	implicit_header_mode.implicit_header_mode_on = 1;

	if (!rf_spi_write_register(rf_handle, RegModemConfig1,
			implicit_header_mode.modem_config_1)) {
		LOG_ERROR("SPI Error");
		return 0;
	}

	if (!rf_spi_write_register(rf_handle, RegPayloadLength, size & 0xff)) {
		LOG_ERROR("SPI Error");
		return 0;
	}

	rf_register_op_mode_config_t recieve_lora_mode = { 0 };
	recieve_lora_mode.long_range_mode = 1;
	recieve_lora_mode.mode = RF_OP_MODE_RXCONTINUOUS;

	if (!rf_spi_write_register(rf_handle, RegOpMode,
			recieve_lora_mode.op_mode)) {
		LOG_ERROR("SPI Error");
		return 0;
	}

	return 1;
}

int rf_available(rf_handle_t *rf_handle) {

	int num_bytes_recieved = 0;

	if (!rf_spi_read_register(rf_handle, RegRxNbBytes, &num_bytes_recieved)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}
	return num_bytes_recieved;
}

int rf_peek(rf_handle_t *rf_handle, uint8_t *rf_recieved_data) {
	if (!rf_available(rf_handle)) {
		return 0;
	}

// store current FIFO address
	int currentAddress = 0;
	if (!rf_spi_read_register(rf_handle, RegFifoAddrPtr, &currentAddress)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}
// read
	if (!rf_spi_read_register(rf_handle, RegFifo, &rf_recieved_data)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}
// restore FIFO address
	if (!rf_spi_write_register(rf_handle, RegFifoAddrPtr, currentAddress)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	return 1;
}

int rf_dump_registers(rf_handle_t *rf_handle) {
	uint8_t read_reg = 0;
	for (int i = 0; i < 128; i++) {
		if (!rf_spi_read_register(rf_handle, i, &read_reg)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}
		LOG_INFO("0x%x: 0x%x", i, read_reg);
	}
	return 1;
}

int rf_set_sync_word(rf_handle_t *rf_handle, uint8_t rf_sync_word) {

	LOG_INFO("SETTING SYNC WORD: 0x%x", rf_sync_word);

	return 1;
}

int rf_set_spread_factor(rf_handle_t *rf_handle, uint8_t rf_spread_factor) {

	if (rf_spread_factor < 6) {
		rf_spread_factor = 6;
	} else if (rf_spread_factor > 12) {
		rf_spread_factor = 12;
	}

	rf_register_detect_optimize_config_t detect_optimize = { 0 };

	if (rf_spread_factor == 6) {
		detect_optimize.detection_optimize = 0x05;
		if (!rf_spi_write_register(rf_handle, RegDetectOptimize,
				detect_optimize.reg_detection_optimize)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}

		if (!rf_spi_write_register(rf_handle, RegDetectionThreshold, 0x0c)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}
	} else {

		detect_optimize.detection_optimize = 0x03;
		if (!rf_spi_write_register(rf_handle, RegDetectOptimize,
				detect_optimize.reg_detection_optimize)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}

		if (!rf_spi_write_register(rf_handle, RegDetectionThreshold, 0x0a)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}
	}

	rf_register_modem_config_2_t modem_config_2 = { 0 };

	if (!rf_spi_read_register(rf_handle, RegModemConfig2,
			&modem_config_2.modem_config_2)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	modem_config_2.spreading_factor = rf_spread_factor;

	if (!rf_spi_write_register(rf_handle, RegModemConfig2,
			modem_config_2.modem_config_2)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	long bw = 0;

	rf_get_bandwidth(rf_handle, &bw);

	int sf = 0;

	rf_register_modem_config_2_t sfconf = { 0 };

	if (!rf_spi_read_register(rf_handle, RegModemConfig2,
			&sfconf.modem_config_2)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	sf = sfconf.spreading_factor;

// Section 4.1.1.5
	long symbolDuration = 1000 / (bw / (1L << sf));

// Section 4.1.1.6
	int ldoOn = (symbolDuration > 16);

	rf_register_modem_config_3_t conf3 = { 0 };

	if (!rf_spi_read_register(rf_handle, RegModemConfig3,
			&conf3.modem_config_3)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	conf3.low_data_rate_optimize = ldoOn;

	if (!rf_spi_write_register(rf_handle, RegModemConfig3,
			conf3.modem_config_3)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	LOG_INFO("SETTING SPREAD FACTOR: %d", rf_spread_factor);

	return 1;
}

int rf_get_spread_factor(rf_handle_t *rf_handle, uint8_t *rf_spread_factor) {
	rf_register_modem_config_2_t sfconf = { 0 };

	if (!rf_spi_read_register(rf_handle, RegModemConfig2,
			&sfconf.modem_config_2)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	*rf_spread_factor = sfconf.spreading_factor;

	return 1;
}

int rf_set_bandwidth(rf_handle_t *rf_handle, rf_bandwidth_t bandwidth) {

	rf_register_modem_config_1_t bwconf = { 0 };

	uint8_t bw = 0;

	if (!rf_spi_read_register(rf_handle, RegModemConfig1,
			&bwconf.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	bw = bandwidth;

	bwconf.bandwidth = bandwidth;

	if (!rf_spi_write_register(rf_handle, RegModemConfig1,
			bwconf.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	uint8_t sf = 0;

	rf_get_spread_factor(rf_handle, &sf);

	long symbolDuration = 1000 / (bw / (1L << sf));

	// Section 4.1.1.6
	int ldoOn = (symbolDuration > 16);

	rf_register_modem_config_3_t conf3 = { 0 };

	if (!rf_spi_read_register(rf_handle, RegModemConfig3,
			&conf3.modem_config_3)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	conf3.low_data_rate_optimize = ldoOn;

	if (!rf_spi_write_register(rf_handle, RegModemConfig3,
			conf3.modem_config_3)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	LOG_INFO("Setting BW %d", bw);

	return 1;

}

int rf_get_bandwidth(rf_handle_t *rf_handle, long *bandwidth) {

	rf_register_modem_config_1_t bwconf = { 0 };

	uint8_t bw = 0;

	if (!rf_spi_read_register(rf_handle, RegModemConfig1,
			&bwconf.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	bw = bwconf.bandwidth;

	switch (bw) {
	case RF_BW_7K8:
		*bandwidth = 7800;
		break;
	case RF_BW_10K4:
		*bandwidth = 10400;
		break;
	case RF_BW_15K6:
		*bandwidth = 15600;
		break;
	case RF_BW_20K8:
		*bandwidth = 20800;
		break;
	case RF_BW_31K25:
		*bandwidth = 31250;
		break;
	case RF_BW_41K7:
		*bandwidth = 41700;
		break;
	case RF_BW_62K5:
		*bandwidth = 62500;
		break;
	case RF_BW_125K:
		*bandwidth = 125000;
		break;
	case RF_BW_250K:
		*bandwidth = 250000;
		break;
	case RF_BW_500K:
		*bandwidth = 500000;
		break;
	}

	return 1;
}

int rf_recieve_single(rf_handle_t *rf_handle,
		uint8_t *rf_recieved_packet_length) {

	uint8_t packet_length = 0;
	rf_register_irq_flags_t irq_flags = { 0 };

	if (!rf_spi_read_register(rf_handle, RegIrqFlags, &irq_flags.irq_flags)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	// TODO: add implicit mode
	rf_register_modem_config_1_t explicit_mode = { 0 };

	if (!rf_spi_read_register(rf_handle, RegModemConfig1,
			&explicit_mode.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	explicit_mode.implicit_header_mode_on = 0;

	if (!rf_spi_write_register(rf_handle, RegModemConfig1,
			explicit_mode.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	if ((irq_flags.rx_done && !irq_flags.payload_crc_error)) { // check if there is no error and rx is done.
		// clear irq flags. here we can just write the read irq register back to reset
		// any flags that have been raised since its a read reset register.
		if (!rf_spi_write_register(rf_handle, RegIrqFlags,
				irq_flags.irq_flags)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}
		// Since we only support Explicit header mode currently
		// TODO: add check for implicit header mode.
		if (!rf_spi_read_register(rf_handle, RegRxNbBytes, &packet_length)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}

		//*rf_recieved_packet_length = packet_length; // more verbose than just throwing pointer into read.
		// set fifo address to current rx address.

		uint8_t current_rx_address = 0;
		if (!rf_spi_read_register(rf_handle, RegFifoRxCurrentAddr,
				&current_rx_address)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}

		if (!rf_spi_write_register(rf_handle, RegFifoAddrPtr,
				current_rx_address)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}

		// stdby

		if (!rf_set_op_mode(rf_handle, RF_OP_MODE_STDBY)) {
			LOG_ERROR("OP MODE ERROR");
			return 0;
		}

		// put the packet length in return paramenter and the caller will read out the data.

	} else {

		rf_register_op_mode_config_t checkopmode = { 0 };
		if (!rf_spi_read_register(rf_handle, RegOpMode, &checkopmode.op_mode)) {
			LOG_ERROR("SPI ERROR");
			return 0;
		}

		if (!checkopmode.long_range_mode
				|| checkopmode.mode != RF_OP_MODE_RX_SINGLE) { // means we are not listening so listen

				// reset the fifo to the bottom
			if (!rf_spi_write_register(rf_handle, RegFifoAddrPtr, 0)) {
				LOG_ERROR("SPI ERROR");
				return 0;
			}
			// put in single rx mode.
			if (!rf_set_op_mode(rf_handle, RF_OP_MODE_RX_SINGLE)) {
				LOG_ERROR("OP MODE ERROR");
				return 0;
			}
			// set to 0 to tell the caller we havent recieved anything
			//*rf_recieved_packet_length = 0;
		}
	}

	rf_register_op_mode_config_t checkopmode = { 0 };
	if (!rf_spi_read_register(rf_handle, RegOpMode, &checkopmode.op_mode)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}
	// better to do it down here since packet length local is persistant throughout.
	*rf_recieved_packet_length = packet_length;
	return 1;
}

int rf_read(rf_handle_t *rf_handle, uint8_t *rf_recieved_byte) {

	// TODO: Implement rf_available check to see if we have enough bytes.
	// right now the only check is in the rf_read_packet.

	if (!rf_spi_read_register(rf_handle, RegFifo, rf_recieved_byte)) {
		LOG_ERROR("rf_read ERROR");
		return 0;
	}

	return 1;
}

int rf_read_packet(rf_handle_t *rf_handle, uint8_t rf_recieved_packet_length,
		uint8_t *rf_recieved_buffer) {

	if (rf_recieved_packet_length == 0) {
		LOG_ERROR("Read packet called with packet length 0");
		return 0;
	}

	uint8_t rec_byte = 0;

	for (int i = 0; i < rf_recieved_packet_length; i++) {
		if (!rf_read(rf_handle, &rec_byte)) {
			LOG_ERROR("FIFO READ ERROR");
			return 0;
		}
		rf_recieved_buffer[i] = rec_byte;
	}

	return 1;
}

int rf_set_ocp(rf_handle_t *rf_handle, uint8_t rf_ocp_level) {
	uint8_t ocpTrim = 27;

	if (rf_ocp_level <= 120) {
		ocpTrim = (rf_ocp_level - 45) / 5;
	} else if (rf_ocp_level <= 240) {
		ocpTrim = (rf_ocp_level + 30) / 10;
	}

	if (!rf_spi_write_register(rf_handle, RegOcp, 0x20 | (0x1F & ocpTrim))) {
		LOG_ERROR("OCP ERROR");
		return 0;
	}

	return 1;

}

int rf_packet_rssi(rf_handle_t *rf_handle, int *rf_packet_rssi) {

	int offset = -157;

	if (!rf_spi_read_register(rf_handle, RegPktRssiValue, rf_packet_rssi)) {
		LOG_ERROR("RSSI ERROR");
		return 0;
	}

	*rf_packet_rssi = *rf_packet_rssi + offset;

	return 1;
}

int rf_packet_snr(rf_handle_t *rf_handle, int *rf_packet_snr) {
	if (!rf_spi_read_register(rf_handle, RegPktSnrValue, rf_packet_snr)) {
		LOG_ERROR("RSSI ERROR");
		return 0;
	}

	*rf_packet_snr = (*rf_packet_snr) / 4;

	return 1;
}

int rf_set_coding_rate(rf_handle_t *rf_handle, int denominator) {
	if (denominator < 5) {
		denominator = 5;
	} else if (denominator > 8) {
		denominator = 8;
	}

	int cr = denominator - 4;

	rf_register_modem_config_1_t modemconf1 = { 0 };

	if (!rf_spi_read_register(rf_handle, RegModemConfig1,
			&modemconf1.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

	modemconf1.coding_rate = cr;

	if (!rf_spi_write_register(rf_handle, RegModemConfig1,
			modemconf1.modem_config_1)) {
		LOG_ERROR("SPI ERROR");
		return 0;
	}

}
