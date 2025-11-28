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

// HardwareSerial object for UART communication with MH-Z19B
SoftwareSerial mhz19bSerial(P_RX, P_TX); // RX, TX
MHZ19 myMHZ19;

/* ------------------ PRIVATE FUNCTIONS PROTOTYPES ----------------- */

int setRange(int range);



/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the MH-Z19B sensor
// @return: 1 if successful, 0 otherwise
int initMHZ19B()
{
    // Begin UART communication with specified baud rate and pins
    mhz19bSerial.begin(9600);
    delay(100);
    myMHZ19.begin(mhz19bSerial);                     // Set Range 2000
    myMHZ19.autoCalibration(false);

    if (myMHZ19.errorCode == RESULT_OK) {
        if (!setRange(2000)) return 0;
        myMHZ19.calibrate();                                // Calibrate
        return 1;
    }
    return 0;
}

int setRange(int range)
{
    myMHZ19.setRange(range);
    if ((myMHZ19.errorCode == RESULT_OK) && (myMHZ19.getRange() == range)) //RESULT_OK is an alias from the library,
        return 1;
    return 0;
}

/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves data from the MH-Z19B sensor
// @param newData: Pointer to structure where data will be stored
void getDataMHZ19B(t_dataMHZ19B *newData)
{
    newData->CO2 = myMHZ19.getCO2();
}