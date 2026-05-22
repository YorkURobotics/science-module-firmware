#ifndef PUMP_DRIVER_H
#define PUMP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"

// PUMP CONTROL FUNCTIONS

// Manual Mode: Turn the pump on at a specific speed and leave it on.
void Pump_SetSpeed(uint16_t pwm_speed);

// Auto Mode: Turn the pump on at a specific speed, but stop it after X milliseconds.
void Pump_Start_Timed(uint16_t pwm_speed, uint32_t duration_ms);

// Quick Start: Run the pump at a safe, slow speed (450) for X milliseconds.
void Pump_Start_Slow_Timed(uint32_t duration_ms);

// Emergency Stop: Instantly kill power to the pump.
void Pump_Stop(void);

// The Timer Check: Put this in the main while(1) loop so the pump knows when to turn itself off.
void Pump_Update(void);

// Continuous Slow Start: Turn the pump on at the safe speed (450) and leave it on.
void Pump_Start_Slow_Continuous(void);

#ifdef __cplusplus
}
#endif

#endif /* PUMP_DRIVER_H */