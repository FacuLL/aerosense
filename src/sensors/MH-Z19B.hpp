/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensure the header is included only once
#ifndef MHZ19B_hpp
#define MHZ19B_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard integer types for fixed-width integer definitions
#include <stdint.h>
#include <Arduino.h>

/* -------------------- MACROS AND CONSTANTS ------------------------- */

// Pin definitions for RX and TX
#define P_MH 15

/* ---------------------- DATA STRUCTURES ------------------------ */

// Structure to hold CO2 concentration data
typedef struct
{
    // CO2 concentration in ppm
    int32_t CO2;
    
} t_dataMHZ19B;

/* ---------------- PUBLIC FUNCTIONS PROTOTYPES ---------------- */

// Initializes the MH-Z19B sensor
// @return: 1 if successful, 0 otherwise
int initMHZ19B();

// Retrieves data from the MH-Z19B sensor
// @param newData: Pointer to structure where data will be stored
void getDataMHZ19B(t_dataMHZ19B *newData);

#endif // MHZ19B_HPP
