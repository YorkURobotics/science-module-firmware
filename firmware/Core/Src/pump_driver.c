#include "pump_driver.h"

// Slowest safe speed for the pump
#define PUMP_DEFAULT_SLOW_SPEED 450

// timer we set up in main.c
extern TIM_HandleTypeDef htim1;

// Variables to keep track of time.
static uint8_t  pump_is_timed = 0;
static uint32_t pump_start_time = 0;
static uint32_t current_duration_ms = 0;

void Pump_SetSpeed(int16_t pwm_speed) {
    if (pwm_speed < 0) {
        pwm_speed = 0; // Negative values shut off the pump
    } else if (pwm_speed > 1000) {
        pwm_speed = 1000; // Cap at max speed
    } else if (pwm_speed > 0 && pwm_speed < PUMP_DEFAULT_SLOW_SPEED) {
        pwm_speed = PUMP_DEFAULT_SLOW_SPEED; // Clamp values (>0 and <450) to 450
    }

    pump_is_timed = 0; 
    
    // Cast it back to a uint32_t at the end to satisfy what STM expects
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)pwm_speed);
}


void Pump_Start_Timed(int16_t pwm_speed, uint32_t duration_ms) {
    // Catch negative numbers and force the pump to stop (0)
    if (pwm_speed < 0) {
        pwm_speed = 0; 
    } 
    // Cap the maximum speed at 1000
    else if (pwm_speed > 1000) {
        pwm_speed = 1000;
    } 
    // If it's on, but too weak to spin, clamp it to safe speed (450)
    else if (pwm_speed > 0 && pwm_speed < PUMP_DEFAULT_SLOW_SPEED) {
        pwm_speed = PUMP_DEFAULT_SLOW_SPEED; 
    }

    // Turn on the timer and save the start time
    pump_is_timed = 1;
    pump_start_time = HAL_GetTick();
    current_duration_ms = duration_ms;

    // Send the speed to the hardware (cast it to uint32_t to satisfy HAL)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)pwm_speed);
}


void Pump_Start_Slow_Timed(uint32_t duration_ms) {
    // Instead of copying and pasting code, just reuse our main function
    // and hand it the safe 450 default speed.
    Pump_Start_Timed(PUMP_DEFAULT_SLOW_SPEED, duration_ms);
}


void Pump_Stop(void) {
    // Turn off the timer
    pump_is_timed = 0;

    // Set speed to 0 to stop the motor
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}


void Pump_Update(void) {
    // Only check the clock if we actually started a timed run
    if (pump_is_timed) {

        // If the time that passed is greater than or equal to our requested time
        if ((HAL_GetTick() - pump_start_time) >= current_duration_ms) {

            // Time's up. Stop the pump.
            Pump_Stop();
        }
    }
}

void Pump_Start_Slow_Continuous(void) {
    // Just reuse the manual speed function and hand it the safe 450 speed.
    Pump_SetSpeed(PUMP_DEFAULT_SLOW_SPEED);
}