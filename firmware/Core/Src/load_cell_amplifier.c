#include "load_cell_amplifier.h"

// Initializes the struct variables and ensures the clock pin is low
void HX711_Init(HX711_t* hx711, GPIO_TypeDef* data_port, uint16_t data_pin, GPIO_TypeDef* clock_port, uint16_t clock_pin) {
    hx711->data_port = data_port;
    hx711->data_pin = data_pin;
    hx711->clock_port = clock_port;
    hx711->clock_pin = clock_pin;
    hx711->offset = 0;
    hx711->scale = 1.0f; // Safety default to prevent division by zero

    // HX711 requires the clock line to be low to remain in normal operation mode
    HAL_GPIO_WritePin(hx711->clock_port, hx711->clock_pin, GPIO_PIN_RESET);
}

// Reads the raw 24-bit value from the amplifier
int32_t HX711_ReadRaw(HX711_t* hx711) {
    uint32_t raw_value = 0;

    // Wait until DOUT goes low (data is ready)
    while (HAL_GPIO_ReadPin(hx711->data_port, hx711->data_pin) == GPIO_PIN_SET) {
        // Polling wait.
    }

    // Read the 24 bits
    for (int i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(hx711->clock_port, hx711->clock_pin, GPIO_PIN_SET);
        raw_value = raw_value << 1;
        HAL_GPIO_WritePin(hx711->clock_port, hx711->clock_pin, GPIO_PIN_RESET);

        if (HAL_GPIO_ReadPin(hx711->data_port, hx711->data_pin) == GPIO_PIN_SET) {
            raw_value++;
        }
    }

    // Send the 25th pulse for Channel A, Gain 128
    HAL_GPIO_WritePin(hx711->clock_port, hx711->clock_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hx711->clock_port, hx711->clock_pin, GPIO_PIN_RESET);

    // Convert 24-bit signed to 32-bit signed
    if (raw_value & 0x800000) {
        raw_value |= 0xFF000000;
    }

    return (int32_t)raw_value;
}

// Automatically calculates zero-point (offset)
void HX711_Tare(HX711_t* hx711, uint8_t times) {
    int64_t sum = 0;

    for (int i = 0; i < times; i++) {
        sum += HX711_ReadRaw(hx711);
        HAL_Delay(10);
    }

    hx711->offset = (int32_t)(sum / times);
}

// Returns the final calculated weight using the calibration scale
float HX711_GetWeight(HX711_t* hx711) {
    int32_t raw_val = 0;

    // Average 3 readings for a smooth output
    for (int i = 0; i < 3; i++) {
        raw_val += HX711_ReadRaw(hx711);
        HAL_Delay(5);
    }
    raw_val /= 3;

    return (float)(raw_val - hx711->offset) / hx711->scale;
}