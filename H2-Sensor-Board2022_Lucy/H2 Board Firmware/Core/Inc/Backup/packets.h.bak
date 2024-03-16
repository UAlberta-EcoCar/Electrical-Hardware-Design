/*
 * packets.h
 *
 *  Created on: Dec 24, 2023
 *      Author: abina
 */

#ifndef INC_PACKETS_H_
#define INC_PACKETS_H_

#include <stdint.h>

// Incoming packet Identifier
// Anyone who wants this information sends a packet with this ID
typedef enum {
	GET_H2_CONC = 0x111,
	GET_TEMP = 0x221,
	GET_PRESSURE = 0x222,
	GET_HUMIDITY = 0x223
} H2_BOARD_CAN_HIGH_PRIORITY_ID;

// RTR Data
// anyone who wants this packet listens to this id.
typedef enum {
	RESPOND_EMERGENCY_JERMA_ALERT = 0x001,
	RESPOND_H2_CONC = 0x111,
	RESPOND_TEMP,
	RESPOND_PRESSURE,
	RESPOND_HUMIDITY,
} RESPONSE_PACKET_ID;

/**
 * Used by serializer to encapsulate packets.
 */
typedef struct {
	uint16_t id; // Default ID of the packet.
	uint8_t dlc;
	uint8_t rtr_type; // is it data for a request packet. Bit field here, we only need 1 bit.
	uint8_t data[8]; //Pointer to the packet being transmitted.
} __attribute__((packed)) IntrimPacket;

typedef struct {
	float h2_conc;
} __attribute__((packed)) EmergencyBroadCastPacket;

typedef struct {
	float float1;
	float float2;
} __attribute__((packed)) TwoFloatPacket;

typedef struct {
	float float1;
} __attribute__((packed)) OneFloatPacket;

/**
 * Just a test packet that determines
 */
struct Packet_Hello {
	uint32_t packet_id;
	char data[];
};

#endif /* INC_PACKETS_H_ */
