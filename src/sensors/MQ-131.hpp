/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensure the header is included only once
#ifndef MQ131_hpp
#define MQ131_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard integer types for portability
#include <stdint.h>

// GPIO functions for ESP32
#include <esp32-hal-gpio.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// Pin for O3 and NO2 measurement
#define P_MQ131 34

/* ----------------- PUBLIC FUNCTIONS PROTOTYPES ----------------- */

// Data structure for storing MQ-131 sensor data
typedef struct
{
    // Ozone level (O3) in parts per billion (ppb)
    int32_t ozone;

    // Nitrogen dioxide level (NO2) in parts per billion (ppb)
    int32_t no2;

} t_dataMQ131;

// Initializes the MQ-131 sensor
// @return: 1 if successful, 0 otherwise
int initMQ131();

// Retrieves data from the MQ-131 sensor
// @param newData: Pointer to structure where data will be stored
void getDataMQ131(t_dataMQ131 *newData);

#endif // MQ131_HPP
