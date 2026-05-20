/*
 * h2_sensor_driver.c
 *
 * Created on: 19th May 2026
 *     Author: Mykola Bohomaz
 */

#include "h2_sensor_driver.h"
#include <stdint.h>

uint16_t adc_raw = 0;
uint8_t adc_high = 0;
uint8_t adc_low = 0;

//===================================================================================
// HELPER FUNCTIONS
//===================================================================================

static void convert_adc(uint16_t adc_value) {
  adc_low = adc_value & 0xFF;
  adc_high = (adc_value >> 8) & 0xFF;
}

// TODO: potentially implement DMA to improve the MCU scheduling

//===================================================================================
// USER FUNCTIONS
//===================================================================================

/*
 * read ADC data
 * @param hadc1 ADC handle pointer
 * @return none
 */
void read_ADC(ADC_HandleTypeDef *hadc1) {
  if (HAL_ADC_Start(hadc1) == HAL_OK) {
    if (HAL_ADC_PollForConversion(hadc1, 10) == HAL_OK) {
      adc_raw = HAL_ADC_GetValue(hadc1);
      convert_adc(adc_raw);
    }

    HAL_ADC_Stop(hadc1);
  }
}

/*
 * CAN transmit wrapper
 */
void transmit_h2_sensor_values(CAN_HandleTypeDef *hcan) {
  uint8_t tx_data[8] = {0};

  tx_data[0] = adc_high;
  tx_data[1] = adc_low;

  CAN_TRANSMIT(hcan, CAN_H2_SENSOR_ID, tx_data, 2);
}
