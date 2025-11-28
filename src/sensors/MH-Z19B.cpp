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



/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the MH-Z19B sensor
// @return: 1 if successful, 0 otherwise
int initMHZ19B()
{
    // Begin UART communication with specified baud rate and pins
    pinMode(P_MH, INPUT);
    return 1;
}

/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves data from the MH-Z19B sensor
// @param newData: Pointer to structure where data will be stored
void getDataMHZ19B(t_dataMHZ19B *newData)
{
    newData->CO2 = analogRead(P_MH);
}