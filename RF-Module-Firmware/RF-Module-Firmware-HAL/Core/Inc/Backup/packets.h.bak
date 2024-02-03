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
struct Packet {
	uint32_t packet_id; // Default ID of the packet.
	void * packet; //Pointer to the packet being transmitted.
};

/**
 * Just a test packet that determines
 */
struct Packet_Hello {
	uint32_t packet_id;
	char data[];
};

#endif /* INC_PACKETS_H_ */
