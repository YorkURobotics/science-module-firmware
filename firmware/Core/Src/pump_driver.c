#include "pump_driver.h"

extern TIM_HandleTypeDef htim1;

// Private Variables
static uint8_t pump_is_running = 0;
static uint32_t pump_start_time = 0;
static const uint32_t PUMP_DURATION_MS = 23010;

// Functions

void Pump_Request_500mL(void) {
    if (!pump_is_running) {
        pump_is_running = 1;
        pump_start_time = HAL_GetTick();

        // Turn pump ON (PWM to 450)
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 450);
    }
}

void Pump_Start_Continuous(void) {
    // 1. Turn pump ON (PWM to 450)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 450);

    // 2. Ensure the automatic timer watch ignores this run
    pump_is_running = 0;
}

void Pump_Stop(void) {
    // 1. Immediately kill power to the motor
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

    // 2. Reset the state
    pump_is_running = 0;
}

void Pump_Update(void) {
    // This only triggers if Pump_Request_500mL() started the pump
    if (pump_is_running) {
        if ((HAL_GetTick() - pump_start_time) >= PUMP_DURATION_MS) {

            // Time is up. Call stop function.
            Pump_Stop();

        }
    }
}
