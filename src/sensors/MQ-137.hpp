/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensure the header is included only once
#ifndef MQ137_hpp
#define MQ137_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard integer types for portability
#include <stdint.h>

// GPIO functions for ESP32
#include <esp32-hal-gpio.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// Pin for NH3 and CO measurement
#define P_MQ137 32

/* ----------------- PUBLIC FUNCTIONS PROTOTYPES ----------------- */

// Data structure for storing MQ-137 sensor data
typedef struct
{
    // Ammonia level (NH3) in parts per million (ppm)
    int32_t nh3;

    // Carbon monoxide level (CO) in parts per million (ppm)
    int32_t co;

} t_dataMQ137;

// Initializes the MQ-137 sensor
// @return: 1 if successful, 0 otherwise
int initMQ137();

// Retrieves data from the MQ-137 sensor
// @param newData: Pointer to structure where data will be stored
void getDataMQ137(t_dataMQ137 *newData);

#endif // MQ137_HPP