/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensure the header is included only once
#ifndef MHZ19B_hpp
#define MHZ19B_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard integer types for fixed-width integer definitions
#include <stdint.h>

// Provides the HardwareSerial class for UART communication
#include <SoftwareSerial.h>
#include <MHZ19.h>

/* -------------------- MACROS AND CONSTANTS ------------------------- */

// UART baud rate for MH-Z19B communication
#define BAUD 9600

// Pin definitions for RX and TX
#define P_RX 15
#define P_TX 18

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
