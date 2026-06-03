#ifndef LOAD_CELL_AMPLIFIER_H
#define LOAD_CELL_AMPLIFIER_H

#include "stm32f3xx_hal.h"

// Structure to hold the HX711 hardware pins and calibration data
typedef struct {
    GPIO_TypeDef* data_port;
    uint16_t      data_pin;
    GPIO_TypeDef* clock_port;
    uint16_t      clock_pin;
    int32_t       offset;
    float         scale;
} HX711_t;

// Function Prototypes
void HX711_Init(HX711_t* hx711, GPIO_TypeDef* data_port, uint16_t data_pin, GPIO_TypeDef* clock_port, uint16_t clock_pin);
int32_t HX711_ReadRaw(HX711_t* hx711);
void HX711_Tare(HX711_t* hx711, uint8_t times);
float HX711_GetWeight(HX711_t* hx711);

#endif /* LOAD_CELL_AMPLIFIER_H */