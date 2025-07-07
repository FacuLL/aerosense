/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file handles the initialization and data retrieval 
    from the MQ-137 sensor. The sensor measures ammonia (NH3) and 
    carbon monoxide (CO) levels using analog inputs.
   
*/

/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Includes the header for the MQ-137 sensor
#include "MQ-137.hpp"


/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the MQ-137 sensor
// @return: 1 if successful, 0 otherwise
int initMQ137()
{
    // Set the sensor pin as input
    pinMode(P_MQ137, INPUT); 
    return 1;
}


/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves data from the MQ-137 sensor
// @param newData: Pointer to structure where data will be stored
void getDataMQ137(t_dataMQ137 *newData)
{
    /* -------------------- LOCAL VARIABLES -------------------- */

    // Raw data from the sensor
    uint16_t rawData = analogRead(P_MQ137);

    /* ------------------ PROCESS SENSOR DATA ------------------ */

    // Formula for Ammonia (NH3) calculation (in ppm)
    // MQ-137 range: 5-500 ppm for NH3
    // Placeholder linear mapping - should be calibrated with actual sensor
    newData->nh3 = (int32_t)((rawData - 150) * 0.12); 

    // Ensure NH3 is within valid range
    if (newData->nh3 < 5) newData->nh3 = 5;       
    if (newData->nh3 > 500) newData->nh3 = 500;   

    // Formula for Carbon Monoxide (CO) calculation (in ppm)
    // Placeholder linear mapping - should be calibrated with actual sensor
    newData->co = (int32_t)((rawData - 100) * 0.08);

    // Ensure no negative CO values
    if (newData->co < 0) newData->co = 0;          
}
