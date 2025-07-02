/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file handles the initialization and data retrieval 
    from the MQ-131 sensor. The sensor measures ozone (O3) and 
    nitrogen dioxide (NO2) levels using analog inputs.
   
*/

/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Includes the header for the MQ-131 sensor
#include "MQ-131.hpp"


/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the MQ-131 sensor
// @return: 1 if successful, 0 otherwise
int initMQ131()
{
    // Set the sensor pin as input
    pinMode(P_MQ131, INPUT); 
    return 1;
}


/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves data from the MQ-131 sensor
// @param newData: Pointer to structure where data will be stored
void getDataMQ131(t_dataMQ131 *newData)
{
    /* -------------------- LOCAL VARIABLES -------------------- */

    // Raw data from the sensor
    uint16_t rawData = analogRead(P_MQ131);

    /* ------------------ PROCESS SENSOR DATA ------------------ */

    // Placeholder formula for Ozone (O3) calculation (in ppb)
    newData->ozone = (int32_t)((rawData - 100) * 0.5); 

    // Ensure no negative values
    if (newData->ozone < 0) newData->ozone = 0;       

    // Placeholder formula for Nitrogen Dioxide (NO2) calculation (in ppb)
    newData->no2 = (int32_t)((rawData - 50) * 0.3);

    // Ensure no negative values
    if (newData->no2 < 0) newData->no2 = 0;          
}
