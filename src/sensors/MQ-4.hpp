/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensures the header is included only once
#ifndef MQ4_hpp
#define MQ4_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard library for integer types
#include <stdint.h>

// Arduino core definitions for pin constants live in Arduino.h
#include <Arduino.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// GPIO pin connected to the MQ-4 sensor
#define P_MQ4 33

/* ----------------- PUBLIC FUNCTIONS PROTOTYPES ----------------- */

// Structure to store methane data
typedef struct
{
    // Methane concentration
    int32_t methane; 
  
} t_dataMQ4;

// Initializes the MQ-4 sensor
int initMQ4();

// Retrieves methane data from the MQ-4 sensor
void getDataMQ4(t_dataMQ4 *newData);

#endif // MQ4_HPP
