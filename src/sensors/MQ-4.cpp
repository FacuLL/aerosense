/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file handles the initialization and data retrieval 
    from the MQ-4 methane sensor. The sensor provides raw analog 
    data that is scaled to obtain methane concentration levels.
   
*/


/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Includes the header for the MQ-4 sensor
#include "MQ-4.hpp"


/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the MQ-4 sensor
// @return: 1 if successful, 0 otherwise
int initMQ4()
{
    // Configure the pin for the MQ-4 sensor as input
    pinMode(P_MQ4, INPUT);
    return 1;
}


/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves data from the MQ-4 sensor
// @param newData: Pointer to structure where methane data will be stored
void getDataMQ4(t_dataMQ4 *newData)
{
    /* -------------------- LOCAL VARIABLES -------------------- */

    // Raw data from the sensor
    uint16_t rawData = analogRead(P_MQ4);

    /* --------------------- PROCESS DATA --------------------- */

    // Scale raw data to obtain methane concentration
    newData->methane = static_cast<uint16_t>(rawData);
}
