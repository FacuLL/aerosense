/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensures the header is included only once
#ifndef MQ7_hpp
#define MQ7_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Includes for standard integer types and ESP32 GPIO functions
#include <stdint.h>
#include <Arduino.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// Pin connected to the MQ-7 sensor
#define P_MQ7 35

/* ------------------- PUBLIC STRUCTURE TYPES ------------------- */

// Structure to hold sensor data
typedef struct
{
    // Carbon monoxide level in ppm
    int32_t carbonMonoxyde;

} t_dataMQ7;

/* ----------------- PUBLIC FUNCTIONS PROTOTYPES ----------------- */

// Initializes the MQ-7 sensor
// @return: 1 if successful
int initMQ7();

// Retrieves data from the MQ-7 sensor
// @param newData: Pointer to structure where the data will be stored
void getDataMQ7(t_dataMQ7 *newData);

#endif // MQ7_hpp