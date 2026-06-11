/**
 ******************************************************************************
 * @file    SOIL_SENSOR.c
 * @brief   Driver implementation for NHK RS485 Modbus Soil Sensor
 ******************************************************************************
 */

#include "../Inc/SOIL_SENSOR.h"
#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * Private defines
 * ------------------------------------------------------------------------- */

#define TX_TIMEOUT      100     /* ms */
#define RX_TIMEOUT      2000    /* ms */
#define BUS_TURNAROUND  10      /* ms — wait after TX before RX */

#define SLAVE_ID        0x01
#define FC_READ         0x03
#define FC_WRITE        0x06

/* -------------------------------------------------------------------------
 * Private: CRC16 Modbus
 * ------------------------------------------------------------------------- */

static uint16_t CRC16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 0x0001) { crc >>= 1; crc ^= 0xA001; }
            else               { crc >>= 1; }
        }
    }
    return crc;
}

/* -------------------------------------------------------------------------
 * Private: send request and receive response
 * ------------------------------------------------------------------------- */

static Soil_Status SendRecv(UART_HandleTypeDef *huart,
                             uint8_t *msg,  uint8_t msg_len,
                             uint8_t *resp, uint8_t resp_len)
{
    /* Append CRC to last 2 bytes */
    uint16_t crc = CRC16(msg, msg_len - 2);
    msg[msg_len - 2] = crc & 0xFF;
    msg[msg_len - 1] = (crc >> 8) & 0xFF;

    /* Flush stale RX data */
    __HAL_UART_FLUSH_DRREGISTER(huart);

    if (HAL_UART_Transmit(huart, msg, msg_len, TX_TIMEOUT) != HAL_OK)
        return SOIL_ERR_TX;

    HAL_Delay(BUS_TURNAROUND);

    if (HAL_UART_Receive(huart, resp, resp_len, RX_TIMEOUT) != HAL_OK)
        return SOIL_ERR_RX;

    /* Verify response CRC */
    uint16_t recv_crc = ((uint16_t)resp[resp_len - 1] << 8) | resp[resp_len - 2];
    if (CRC16(resp, resp_len - 2) != recv_crc)
        return SOIL_ERR_CRC;

    return SOIL_OK;
}

/* -------------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------------- */

/**
 * @brief Read humidity, temperature, conductivity, pH, N, P, K
 *        Master sends:  01 03 00 00 00 07 04 08
 *        Sensor responds: 01 03 0E [14 data bytes] CRC CRC
 */
Soil_Status Soil_ReadAll(UART_HandleTypeDef *huart, Soil_Data *data)
{
    if (huart == NULL || data == NULL) return SOIL_ERR_TX;

    uint8_t msg[8]    = { SLAVE_ID, FC_READ, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00 };
    uint8_t resp[19]  = { 0 };  /* 3 header + 14 data + 2 CRC */

    Soil_Status status = SendRecv(huart, msg, sizeof(msg), resp, sizeof(resp));
    if (status != SOIL_OK) return status;

    /* Parse — data starts at resp[3], each register is 2 bytes big-endian */
    data->humidity     = ((uint16_t)(resp[3]  << 8) | resp[4])  * 0.1f;
    data->temperature  = (int16_t) ((resp[5]  << 8) | resp[6])  * 0.1f;
    data->conductivity =  (uint16_t)(resp[7]  << 8) | resp[8];
    data->ph           = ((uint16_t)(resp[9]  << 8) | resp[10]) * 0.1f;
    data->nitrogen     =  (uint16_t)(resp[11] << 8) | resp[12];
    data->phosphorus   =  (uint16_t)(resp[13] << 8) | resp[14];
    data->potassium    =  (uint16_t)(resp[15] << 8) | resp[16];

    return SOIL_OK;
}

/**
 * @brief Query slave ID using broadcast address
 *        Master sends:  FF 03 07 D0 00 01 91 59
 *        Sensor responds: FF 03 02 00 01 CRC CRC
 */
Soil_Status Soil_QuerySlaveID(UART_HandleTypeDef *huart, uint8_t *slave_id)
{
    if (huart == NULL || slave_id == NULL) return SOIL_ERR_TX;

    /* CRC pre-calculated for this fixed message */
    uint8_t msg[8]   = { 0xFF, FC_READ, 0x07, 0xD0, 0x00, 0x01, 0x91, 0x59 };
    uint8_t resp[7]  = { 0 };

    Soil_Status status = SendRecv(huart, msg, sizeof(msg), resp, sizeof(resp));
    if (status != SOIL_OK) return status;

    *slave_id = resp[4];
    return SOIL_OK;
}

/**
 * @brief Set a new slave ID on the sensor
 *        Master sends:  01 06 07 D0 00 <id> CRC CRC
 *        Sensor echoes: same bytes back
 *        Example (set ID=2): 01 06 07 D0 00 02 08 86
 */
Soil_Status Soil_SetSlaveID(UART_HandleTypeDef *huart, uint8_t new_id)
{
    if (huart == NULL) return SOIL_ERR_TX;

    uint8_t msg[8]  = { SLAVE_ID, FC_WRITE, 0x07, 0xD0, 0x00, new_id, 0x00, 0x00 };
    uint8_t resp[8] = { 0 };

    return SendRecv(huart, msg, sizeof(msg), resp, sizeof(resp));
}

/**
 * @brief Set sensor baud rate
 *        Master sends:  01 06 07 D1 00 <rate> CRC CRC
 *        Sensor echoes: same bytes back
 *        Example (set 9600): 01 06 07 D1 00 02 59 46
 *        rate: 0=2400, 1=4800, 2=9600
 */
Soil_Status Soil_SetBaudRate(UART_HandleTypeDef *huart, uint8_t rate)
{
    if (huart == NULL) return SOIL_ERR_TX;

    uint8_t msg[8]  = { SLAVE_ID, FC_WRITE, 0x07, 0xD1, 0x00, rate, 0x00, 0x00 };
    uint8_t resp[8] = { 0 };

    return SendRecv(huart, msg, sizeof(msg), resp, sizeof(resp));
}

/**
 * @brief Write nitrogen value to sensor register
 *        Master sends:  01 06 00 04 00 <value> CRC CRC
 *        Sensor echoes: same bytes back
 *        Example (write 32): 01 06 00 04 00 20 C9 D3
 */
Soil_Status Soil_WriteNitrogen(UART_HandleTypeDef *huart, uint16_t value)
{
    if (huart == NULL) return SOIL_ERR_TX;

    uint8_t msg[8]  = { SLAVE_ID, FC_WRITE, 0x00, 0x04,
                        (value >> 8) & 0xFF, value & 0xFF, 0x00, 0x00 };
    uint8_t resp[8] = { 0 };

    return SendRecv(huart, msg, sizeof(msg), resp, sizeof(resp));
}

/**
 * @brief Write phosphorus value to sensor register
 *        Master sends:  01 06 00 05 00 <value> CRC CRC
 *        Sensor echoes: same bytes back
 *        Example (write 88): 01 06 00 05 00 58 98 31
 */
Soil_Status Soil_WritePhosphorus(UART_HandleTypeDef *huart, uint16_t value)
{
    if (huart == NULL) return SOIL_ERR_TX;

    uint8_t msg[8]  = { SLAVE_ID, FC_WRITE, 0x00, 0x05,
                        (value >> 8) & 0xFF, value & 0xFF, 0x00, 0x00 };
    uint8_t resp[8] = { 0 };

    return SendRecv(huart, msg, sizeof(msg), resp, sizeof(resp));
}

/**
 * @brief Write potassium value to sensor register
 *        Master sends:  01 06 00 06 00 <value> CRC CRC
 *        Sensor echoes: same bytes back
 *        Example (write 104): 01 06 00 06 00 68 68 25
 */
Soil_Status Soil_WritePotassium(UART_HandleTypeDef *huart, uint16_t value)
{
    if (huart == NULL) return SOIL_ERR_TX;

    uint8_t msg[8]  = { SLAVE_ID, FC_WRITE, 0x00, 0x06,
                        (value >> 8) & 0xFF, value & 0xFF, 0x00, 0x00 };
    uint8_t resp[8] = { 0 };

    return SendRecv(huart, msg, sizeof(msg), resp, sizeof(resp));
}

/**
 * @brief Check if sensor is alive by querying its slave ID
 */
bool Soil_IsAlive(UART_HandleTypeDef *huart)
{
    uint8_t id = 0;
    return (Soil_QuerySlaveID(huart, &id) == SOIL_OK);
}

/**
 * @brief Format sensor data into a readable debug string
 */
void Soil_FormatData(const Soil_Data *data, char *buf, uint16_t buflen)
{
    if (data == NULL || buf == NULL) return;
    snprintf(buf, buflen,
            "--- Soil Report ---\r\n"
            "Temperature:  %.1f C\r\n"
            "Humidity:     %.1f %%RH\r\n"
            "pH:           %.1f\r\n"
            "Conductivity: %u us/cm\r\n"
            "Nitrogen:     %u mg/kg\r\n"
            "Phosphorus:   %u mg/kg\r\n"
            "Potassium:    %u mg/kg\r\n\r\n",
            data->temperature, data->humidity, data->ph,
            data->conductivity, data->nitrogen,
            data->phosphorus, data->potassium);
}

/**
 * @brief Convert status to string
 */
const char *Soil_StatusStr(Soil_Status status)
{
    switch (status) {
        case SOIL_OK:       return "OK";
        case SOIL_ERR_TX:   return "TX Error";
        case SOIL_ERR_RX:   return "RX Timeout";
        case SOIL_ERR_CRC:  return "CRC Mismatch";
        default:            return "Unknown";
    }
}
