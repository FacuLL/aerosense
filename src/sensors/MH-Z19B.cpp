/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file handles the initialization and data retrieval 
    from the MH-Z19B CO2 sensor. The sensor communicates via UART 
    and provides CO2 concentration in ppm.
   
*/


/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Includes the header for the MH-Z19B sensor
#include "MH-Z19B.hpp"

/* ---------------------- GLOBAL VARIABLES ---------------------- */

// HardwareSerial object for UART communication with MH-Z19B
HardwareSerial uartMHZ19B(2);

// Buffer for sensor response
uint8_t _response[9];

/* ------------------ PRIVATE FUNCTIONS PROTOTYPES ----------------- */

// Writes data to UART
// @param data: Array of bytes to send
// @param len: Length of the data array
void _writeUART(uint8_t data[], uint8_t len);

// Calculates CRC for the given data array
// @param data: Array of bytes for which CRC is calculated
// @return: Calculated CRC value
uint8_t _calcCRC(uint8_t data[]);

// Converts two bytes into an integer
// @param h: High byte
// @param l: Low byte
// @return: Combined integer value
int bytes2int(uint8_t h, uint8_t l);


/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the MH-Z19B sensor
// @return: 1 if successful, 0 otherwise
int initMHZ19B()
{
    // Begin UART communication with specified baud rate and pins
    uartMHZ19B.begin(BAUD, SERIAL_8N1, P_RX, P_TX);
    return 1;
}

/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves data from the MH-Z19B sensor
// @param newData: Pointer to structure where data will be stored
void getDataMHZ19B(t_dataMHZ19B *newData)
{
    /* ------------------- LOCAL VARIABLES ------------------- */

    // Command buffer to send request to the sensor
    uint8_t cmd[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    /* -------------------- SEND REQUEST -------------------- */

    // Calculate and append CRC to the command
    cmd[8] = _calcCRC(cmd);

    // Send command to the sensor
    _writeUART(cmd, 9);

    /* ----------------- WAIT FOR RESPONSE ----------------- */

    // Wait for response from the sensor
    while (uartMHZ19B.available() <= 0)
    {
        delay(10);
    }

    /* ------------------- READ RESPONSE ------------------- */

    // Clear the command buffer
    memset(cmd, 0, 9);

    // Read response into the buffer
    uartMHZ19B.readBytes(cmd, 9);

    /* ------------------- PARSE RESPONSE ------------------- */

    // Validate CRC of the response
    if (cmd[8] != _calcCRC(cmd))
    {
        // Indicate error in CRC validation
        newData->CO2 = -1;
        return;
    }

    // Extract CO2 data from the response
    newData->CO2 = bytes2int(cmd[2], cmd[3]);

    // Check if CO2 concentration is invalid or out of realistic range
    if (newData->CO2 < 400 || newData->CO2 > 5000)
    {
        // Indicate error or unrealistic value
        newData->CO2 = -1;
    }
}


/* *****************************************************************
    *                     WRITE UART FUNCTION                     *
   ***************************************************************** */

// Writes data to UART
// @param data: Array of bytes to send
// @param len: Length of the data array
void _writeUART(uint8_t data[], uint8_t len)
{
    // Clear UART buffer
    while (uartMHZ19B.available() > 0)
    {
        uartMHZ19B.read();
    }

    // Write data to UART
    uartMHZ19B.write(data, len);

    // Wait for transmission to complete
    uartMHZ19B.flush();
}


/* *****************************************************************
    *                      CALC CRC FUNCTION                      *
   ***************************************************************** */

// Calculates CRC for the given data array
// @param data: Array of bytes for which CRC is calculated
// @return: Calculated CRC value
uint8_t _calcCRC(uint8_t data[])
{
    /* ------------------- LOCAL VARIABLES ------------------- */

    // Index for iteration
    uint8_t i;

    // CRC value, initially set to zero
    uint8_t crc = 0;

    /* ------------------- CRC CALCULATION ------------------- */

    // Compute CRC by summing data bytes from index 1 to 7
    for (i = 1; i < 8; i++)
    {
        crc += data[i];
    }

    // Invert and increment CRC value
    crc = 255 - crc;
    crc++;

    return crc;
}


/* *****************************************************************
    *                    BYTES TO INT FUNCTION                    *
   ***************************************************************** */

// Converts two bytes into an integer
// @param h: High byte
// @param l: Low byte
// @return: Combined integer value
int bytes2int(uint8_t h, uint8_t l)
{
    /* ------------------- LOCAL VARIABLES ------------------- */

    // Variables for conversion
    int high;
    int low;

    /* ---------------- VALIDATE INPUT BYTES ---------------- */

    // Ensure high and low bytes are within valid range (0-255)
    if (h > 255 || l > 255)
    {
        // Indicate error in byte values
        return -1;
    }

    /* ------------------- BYTE COMBINATION ------------------ */

    // Convert bytes to integer using bitwise operations
    high = static_cast<int>(h);
    low = static_cast<int>(l);
    return (256 * high) + low;
}


