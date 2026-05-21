/*
 * can_handler.h
 *
 *  Created on: May 15, 2026
 *      Author: Tirth Patel
 */

#include <stdint.h>
#include "main.h"

#ifndef INC_CAN_HANDLER_H_
#define INC_CAN_HANDLER_H_

// -----------------------------------------------------------------------
// Configuration (Define all CAN_IDs here)
// -----------------------------------------------------------------------

#define CAN_DEFAULT_ID 0x0F110000 // TODO: Change ID

// -----------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------

HAL_StatusTypeDef CAN_CONFIG(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef CAN_TRANSMIT(CAN_HandleTypeDef * hcan, uint32_t id, uint8_t *data, uint8_t size);

void CAN_Process_Incoming(uint32_t id, uint8_t *data, uint8_t len);

#endif /* INC_CAN_HANDLER_H_ */
