// Define
#ifndef PUMP_DRIVER_H
#define PUMP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"

// Function Prototypes

/**
 * @brief Triggers the pump to run for the calibrated 500mL duration.
 */
void Pump_Request_500mL(void);

/**
 * @brief Turns the pump ON.
 * Requires Pump_Stop() to turn off.
 */
void Pump_Start_Continuous(void);

/**
 * @brief Immediately halts the pump.
 */
void Pump_Stop(void);

/**
 * @brief Background watch for the timed pump.
 */
void Pump_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* PUMP_DRIVER_H */
