/*
 * packets.h
 *
 *  Created on: Dec 24, 2023
 *      Author: abina
 */

#ifndef INC_PACKETS_H_
#define INC_PACKETS_H_

#include <stdint.h>

/**
 * Used by serializer to encapsulate packets.
 */
typedef struct
{
	uint32_t id;	 // Default ID of the packet.
	uint8_t data[8]; // Pointer to the packet being transmitted.
} IntrimPacket;

/**
 * Just a test packet that determines
 */
struct Packet_Hello
{
	uint32_t packet_id;
	char data[];
};

enum packets
{
	H2_ALARM = 0x001,
	SHELL_EXT_STOP = 0x002,
	RELAY_CONF = 0x003,
	CAP_VOLT_CURR = 0x101,
	MTR_VOLT_CURR = 0x102,
	FC_VOLT_CURR = 0x103,
	INT_STACK_PRES_TEMP = 0x201,
	ACCEL_X_Y = 0x202,
	ACCEL_Z_SPEED = 0x203,
	H2_CONC_MV = 0x111,
	H2_TEMP = 0x221,
	H2_PRESSURE = 0x222,
	H2_HUMIDITY = 0x223
};

#endif /* INC_PACKETS_H_ */
