/*
 * mq8calibration.c
 *
 *  Created on: Feb 28, 2024
 *      Author: abina
 */

#include "mq8calibration.h"

float H2Curve[3] = { 2.3, 0.93, -1.44 }; //two points are taken from the curve in datasheet.
//with these two points, a line is formed which is "approximately equivalent"
//to the original curve.
//data format:{ x, y, slope}; point1: (lg200, lg8.5), point2: (lg10000, lg0.03)

float Ro = 10;                  //Ro is initialized to 10 kilo ohms

int MQ8_GetPercentage(float rs_ro_ratio, float *pcurve) {
	return (pow(10, (((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0])));
}

int MQ8_GetH2Percentage(float rs_ro_ratio) {
	return MQ8_GetPercentage(rs_ro_ratio, H2Curve);
}

float MQ8_ResistanceCalculation(int real_adc) {
	return ((float) (RL_VALUE * real_adc));
	// 4095 is adc resolution origitan was 1023
//	return (((float) RL_VALUE * (4095 - real_adc) / real_adc));
}

float MQ8_Compensate(float real_adc) {
	int i;
	float rs = 0;

	for (i = 0; i < READ_SAMPLE_TIMES; i++) {
		rs += MQ8_ResistanceCalculation(real_adc);
		osDelay(READ_SAMPLE_INTERVAL);

	}

	rs = rs / READ_SAMPLE_TIMES;

	return rs;
}

float MQ8_Calibration() {
	int i;
	float val = 0;

	for (i = 0; i < CALIBARAION_SAMPLE_TIMES; i++) {     //take multiple samples
//		val += MQ8_ResistanceCalculation(adc_buf[ADC_BUF_LEN - 1] * 0.8);
		osDelay(CALIBRATION_SAMPLE_INTERVAL);
	}
	val = val / CALIBARAION_SAMPLE_TIMES;          //calculate the average value

	val = val / RO_CLEAN_AIR_FACTOR; //divided by RO_CLEAN_AIR_FACTOR yields the Ro
									 //according to the chart in the datasheet

	return val;
}
