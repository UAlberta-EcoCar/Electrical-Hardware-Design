/*
 * lucy-can-ids.h
 *
 *  Created on: Jan 3, 2025
 *      Author: abina
 */

#ifndef INC_LUCY_CAN_IDS_H_
#define INC_LUCY_CAN_IDS_H_

#define CAN_LUCY_EMERGENCY 0x001
#define CAN_LUCY_SHELL_EXT_STOP 0x002
#define CAN_LUCY_RELAY_CONFIGURATION 0x003

#define CAN_LUCY_CAPACITOR_VI 0x101
#define CAN_LUCY_MOTOR_VI 0x102
#define CAN_LUCY_FC_VI 0x103

#define CAN_LUCY_INT_STACK_PT 0x201
#define CAN_LUCY_ACCEL_XY 0x202
#define CAN_LUCY_ACCEL_Z_SPEED 0x203

#define CAN_LUCY_H2_CONC 0x111
#define CAN_LUCY_H2_TEMP 0x211
#define CAN_LUCY_H2_HUMIDITY 0x223

#define CAN_LUCY_FC_FAN_TACH_12 0x301

typedef struct {
	union {
		struct {
			uint8_t cap_voltage :4;
			uint8_t cap_current :4;
		};
		uint8_t can_raw_lucy_capacitor_vi;
	};
} can_lucy_capacitor_vi;

typedef struct {
	union {
		struct {
			float mtr_voltage;
			float mtr_current;
		};
		uint8_t can_raw_lucy_motor_vi[8];
	};
} can_lucy_motor_vi;

typedef struct {
	union {
		struct {
			float fc_voltage;
			float fc_current;
		};
		uint8_t can_raw_lucy_fc_vi[8];
	};
} can_lucy_fc_vi;

typedef struct {
	union {
		struct {
			uint8_t int_stack_pressure;
			uint8_t int_stack_temprature;
		};
		uint8_t can_raw_lucy_int_stack_pt[8];
	};
} can_lucy_int_stack_pt;

typedef struct {
	union {
		struct {
			uint8_t x_accel :4;
			uint8_t y_accel :4;
		};
		uint8_t can_raw_lucy_accel_xy;
	};
} can_lucy_accel_xy;

typedef struct {
	union {
		struct {
			float z_accel;
			float speed_magnitude;
		};
		uint8_t can_raw_lucy_accel_z_speed[8];
	};
} can_lucy_accel_z_speed;

typedef struct {
	union {
		struct {
			float h2_sense_voltage;
		};
		uint8_t can_raw_lucy_h2_conc[8];
	};
} can_lucy_h2_conc;

typedef struct {
	union {
		struct {
			float h2_temp;
		};
		uint8_t can_raw_lucy_h2_temp[8];
	};
} can_lucy_h2_temp;

typedef struct {
	union {
		struct {
			float h2_pressure;
		};
		uint8_t can_raw_lucy_h2_pressure[8];
	};
} can_lucy_h2_pressure;

typedef struct {
	union {
		struct {
			float h2_humidity;
		};
		uint8_t can_raw_lucy_h2_humidity[8];
	};
} can_lucy_h2_humidity;

typedef struct {
	union {
		struct {
			uint8_t fc_tach_1_rpm :4;
			uint8_t fc_tach_2_rpm :4;
		};
		uint8_t can_raw_lucy_fc_tach_12;
	};
} can_lucy_fc_tach_12;

#endif /* INC_LUCY_CAN_IDS_H_ */
