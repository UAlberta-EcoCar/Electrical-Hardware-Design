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
typedef struct {
	uint32_t id; // Default ID of the packet.
	uint8_t data[8]; //Pointer to the packet being transmitted.
} IntrimPacket;

/**
 * Just a test packet that determines
 */
struct Packet_Hello {
	uint32_t packet_id;
	char data[];
};

#endif /* INC_PACKETS_H_ */
