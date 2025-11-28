/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file handles the initialization and data retrieval for the 
    MQ-7 sensor. The sensor measures carbon monoxide (CO) levels 
    and provides analog data which is scaled to ppm.
   
*/


/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Includes the header for the MQ-7 sensor
#include "MQ-7.hpp"


/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the MQ-7 sensor
// @return: 1 if successful
int initMQ7()
{
    // Configure the sensor pin as input
    pinMode(P_MQ7, INPUT);
    return 1;
}


/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves data from the MQ-7 sensor
// @param newData: Pointer to structure where the data will be stored
void getDataMQ7(t_dataMQ7 *newData)
{
    /* -------------------- LOCAL VARIABLES -------------------- */

    // Raw analog data from the sensor
    uint16_t rawData = analogRead(P_MQ7);

    /* ------------------ SCALING AND STORAGE ------------------ */

    // Scale raw data and store in the structure
    newData->carbonMonoxyde = (uint16_t) rawData;
}

