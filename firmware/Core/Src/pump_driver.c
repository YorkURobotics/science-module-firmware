#include "pump_driver.h"

// Safe speed for the pump so we don't splash water everywhere (about 45% power)
#define PUMP_DEFAULT_SLOW_SPEED 450

// Grab the timer we set up in main.c
extern TIM_HandleTypeDef htim1;

// Variables to keep track of time.
static uint8_t  pump_is_timed = 0;
static uint32_t pump_start_time = 0;
static uint32_t current_duration_ms = 0;

//IMPORTAN: try to keep pwm speed in a range of 450-1000. Anything below 450 is not enough to spin the motor. Use 0 to stop the pump.
void Pump_SetSpeed(uint16_t pwm_speed) {
    // Turn off the timer so it doesn't accidentally stop our manual run
    pump_is_timed = 0;

    // Set the motor speed
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_speed);
}


void Pump_Start_Timed(uint16_t pwm_speed, uint32_t duration_ms) {
    // Turn on the timer flag and save the current time from the board
    pump_is_timed = 1;
    pump_start_time = HAL_GetTick();
    current_duration_ms = duration_ms;

    // Start the motor at the requested speed
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_speed);
}


void Pump_Start_Slow_Timed(uint32_t duration_ms) {
    // Instead of copying and pasting code, just reuse our main function
    // and hand it the safe 450 default speed.
    Pump_Start_Timed(PUMP_DEFAULT_SLOW_SPEED, duration_ms);
}


void Pump_Stop(void) {
    // Turn off the timer flag
    pump_is_timed = 0;

    // Set speed to 0 to stop the motor
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}


void Pump_Update(void) {
    // Only check the clock if we actually started a timed run
    if (pump_is_timed) {

        // If the time that passed is greater than or equal to our requested time...
        if ((HAL_GetTick() - pump_start_time) >= current_duration_ms) {

            // Time's up. Stop the pump.
            Pump_Stop();
        }
    }
}