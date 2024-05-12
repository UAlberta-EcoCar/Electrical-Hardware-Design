/*
 * mq8calibration.h
 *
 *  Created on: Feb 28, 2024
 *      Author: abina
 */

#ifndef SRC_MQ8_MQ8CALIBRATION_H_
#define SRC_MQ8_MQ8CALIBRATION_H_

#include "cmsis_os.h"
#include "main.h"

#define         RL_VALUE                     (10)    //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.21)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (100)   //define the time interal(in milisecond) between each samples in the
//cablibration phase
#define         READ_SAMPLE_INTERVAL         (100)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (10)     //define the time interal(in milisecond) between each samples in
#define         GAS_H2                      (0)                                                     //normal operation                                             //which is derived from the chart in datasheet

extern float H2Curve[3];

extern float Ro;

/*****************************  MQGetPercentage **********************************
 Input:   rs_ro_ratio - Rs divided by Ro
 pcurve      - pointer to the curve of the target gas
 Output:  ppm of the target gas
 Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm)
 of the line could be derived if y(rs_ro_ratio) is provided. As it is a
 logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
 value.
 ************************************************************************************/
int MQ8_GetPercentage(float rs_ro_ratio, float *pcurve);

/*****************************  MQGetGasPercentage **********************************
 Input:   rs_ro_ratio - Rs divided by Ro
 gas_id      - target gas type
 Output:  ppm of the target gas
 Remarks: This function passes different curves to the MQGetPercentage function which
 calculates the ppm (parts per million) of the target gas.
 ************************************************************************************/
int MQ8_GetH2Percentage(float rs_ro_ratio);

/****************** MQResistanceCalculation ****************************************
 Input:   raw_adc - raw value read from adc, which represents the voltage
 Output:  the calculated sensor resistance
 Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
 across the load resistor and its resistance, the resistance of the sensor
 could be derived.
 ************************************************************************************/
float MQ8_ResistanceCalculation(float real_adc);

/*****************************  MQRead *********************************************
 Input:   mq_pin - analog channel
 Output:  Rs of the sensor
 Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
 The Rs changes as the sensor is in the different consentration of the target
 gas. The sample times and the time interval between samples could be configured
 by changing the definition of the macros.
 ************************************************************************************/
float MQ8_Compensate(float real_adc);

/***************************** MQCalibration ****************************************
 Input:   mq_pin - analog channel
 Output:  Ro of the sensor
 Remarks: This function assumes that the sensor is in clean air. It use
 MQResistanceCalculation to calculates the sensor resistance in clean air
 and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about
 10, which differs slightly between different sensors.
 ************************************************************************************/
float MQ8_Calibration();

#endif /* SRC_MQ8_MQ8CALIBRATION_H_ */
