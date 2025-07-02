/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file manages the initialization and data retrieval 
    from the BME680 sensor, including temperature, humidity, 
    pressure, and gas resistance. VOC calculation has also been added.
   
*/


/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Includes the header file for the BME680 sensor class
#include "BME680.hpp"

/* ---------------------- GLOBAL VARIABLES ---------------------- */

// Object for interfacing with the BME680 sensor
BME680_Class BME680;

/* ----------------- PRIVATE FUNCTIONS PROTOTYPES ----------------- */

// Calculates VOC index from gas resistance
// @param gasResistance: The measured gas resistance
// @return: Calculated VOC index
static int calculateVOCIndex(uint32_t gasResistance);


/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the BME680 sensor
// @return: 1 if successful, 0 otherwise
int initBME680()
{
    /* ------------------ LOCAL VARIABLES ------------------ */

    // Number of connection attempts remaining
    uint8_t connectTry = 1;

    /* ------------------ INITIALIZATION ------------------ */

    // Attempt to initialize the sensor in I2C standard mode
    while (!BME680.begin(I2C_STANDARD_MODE) && connectTry != 0)
    {
        // Wait before retrying
        delay(2000);
        connectTry--;
    }

    // Check if initialization was unsuccessful
    if (!connectTry)
    {
        return 0;
    }

    // Configure sensor settings for oversampling and filters
    BME680.setOversampling(TemperatureSensor, Oversample16);
    BME680.setOversampling(HumiditySensor, Oversample16);
    BME680.setOversampling(PressureSensor, Oversample16);
    BME680.setIIRFilter(IIR4);

    // Configure gas settings for heater temperature and duration
    BME680.setGas(320, 150);

    return 1;
}


/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves data from the BME680 sensor
// @param newData: Pointer to structure where data will be stored
void getDataBME680(t_dataBME680 *newData)
{
    /* ----------------- LOCAL VARIABLES ----------------- */

    // Temporary variables for raw sensor data
    static int32_t temp, humidity, pressure, gas;

    /* ----------------- DATA RETRIEVAL ----------------- */

    // Retrieve sensor data
    BME680.getSensorData(temp, humidity, pressure, gas);

    // Temperature in degrees Celsius
    newData->temp = (int16_t)(temp / 100);

    // Relative humidity in %
    newData->humidity = (uint16_t)(humidity / 1000);

    // Pressure in hPa
    newData->pressure = (uint16_t)(pressure / 100);

    newData->vocIndex = calculateVOCIndex((uint32_t)gas);
}


/* *****************************************************************
    *                CALCULATE VOC INDEX FUNCTION                 *
   ***************************************************************** */

// Calculates VOC index from gas resistance
// @param gasResistance: The measured gas resistance
// @return: Calculated VOC index
static int calculateVOCIndex(uint32_t gasResistance)
{
    /* --------------- LOCAL VARIABLES --------------- */

    // VOC index, initially set to zero
    int vocIndex = 0;

    /* --------------- VOC CALCULATION ----------------- */

    // Example linear mapping of gas resistance to VOC index
    if (gasResistance > 0)
    {
        vocIndex = 500 - (int)(gasResistance / 1000);

        // Ensure the VOC index is within valid range
        if (vocIndex < 0)
        {
            vocIndex = 0;
        }

        else if (vocIndex > 500)
        {
            vocIndex = 500;
        }
    }

    return vocIndex;
}
