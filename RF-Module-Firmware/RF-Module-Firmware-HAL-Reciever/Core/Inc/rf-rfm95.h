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

#include "rf-rfm95-resgisters.h"

#define RFM95_INTERRUPT_COUNT 3

typedef void (*rf_delay_func_t)(uint32_t);

typedef void (*rf_post_init_func_t)();

/**
 * Do not modify directly. This struct is used by library functions.
 */
typedef struct {
	uint8_t rf_implicit_header_mode;
} rf_state_t;

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

	rf_state_t *rf_state;

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

/**
 * Listen for any incoming packets with explicit header.
 * @param *rf_handle Handle of module.
 */
int rf_listen(rf_handle_t *rf_handle);

/**
 * Listen for any incoming packets with implicit header; Needs size of packet.
 * @param *rf_handle Handle of module.
 * @param size Integer size of packet we are expecting.
 */
int rf_listen_implicit(rf_handle_t *rf_handle, uint8_t size);

/**
 * Check the number of bytes in the recieve FIFO.
 * @returns The number of bytes waiting in FIFO.
 */
int rf_available(rf_handle_t *rf_handle);

/**
 * Peeks the data stored in the RF FIFO.
 * @param rf_handle handle to the module being peeked.
 * @param rf_recieved_data pointer to a buffer for the data to be put in.
 */
int rf_peek(rf_handle_t *rf_handle, uint8_t *rf_recieved_data);

int rf_dump_registers(rf_handle_t *rf_handle);

/**
 * Sets opmode only in lora mode.
 */
int rf_set_op_mode(rf_handle_t *rf_handle, rf_op_mode_t rf_op_mode);

int rf_set_sync_word(rf_handle_t *rf_handle, uint8_t rf_sync_word);

int rf_set_spread_factor(rf_handle_t *rf_handle, uint8_t rf_spread_factor);

int rf_get_spread_factor(rf_handle_t *rf_handle, uint8_t *rf_spread_factor);

int rf_set_bandwidth(rf_handle_t *rf_handle, rf_bandwidth_t bandwidth);

int rf_get_bandwidth(rf_handle_t *rf_handle, long *bandwidth);

/**
 * Recieves the message packet in single rx mode. Usage must be accompanied by a check of @param rf_recieved_packet_length
 * value. If value is non zero the then the packet must be read out by the caller with rf_read_packet.
 * @param rf_handle handle to the rf module.
 * @param rf_recieved_packet_length number of bytes that were received.
 */
int rf_recieve_single(rf_handle_t *rf_handle,
		uint8_t *rf_recieved_packet_length);

/**
 * Reads single byte from FIFO and places in @param rf_recieved_byte
 */
int rf_read(rf_handle_t *rf_handle, uint8_t *rf_recieved_byte);

int rf_read_packet(rf_handle_t *rf_handle, uint8_t rf_recieved_packet_length,
		uint8_t *rf_recieved_buffer);

int rf_set_ocp(rf_handle_t *rf_handle, uint8_t rf_ocp_level);

int rf_packet_rssi(rf_handle_t *rf_handle, int *rf_packet_rssi);

int rf_packet_snr(rf_handle_t *rf_handle, int *rf_packet_snr);

int rf_set_coding_rate(rf_handle_t *rf_handle, int denominator);

#endif /* INC_RF_RFM95_H_ */
