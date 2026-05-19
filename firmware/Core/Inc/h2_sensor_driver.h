/*
 * h2_sensor_driver.h
 *
 *  Created on: 19th May 2026
 *      Author: Mykola Bohomaz
 *
 */

#include "main.h"
#include <stdint.h>
#ifndef H2_SENSOR_DRIVER_H

/*
 * ADC config
 */
#define ADC_MAX_VALUE 4095.0f
#define ADC_REF_VOLTAGE 3.3f

/*
 * Voltage divider config
 */
#define DIVIDER_R1 2000.0f
#define DEVIDER_R2 3000.0f

/*
 * read ADC data
 * @param hadc1  ADC handle variable
 * @return the reading of the ADC unconverted
 */
void read_ADC(ADC_HandleTypeDef hadc1);

/*
 * CAN transmit wraper
 */
void transmit_h2_sensor_values(CAN_HandleTypeDef *hcan);

#endif
