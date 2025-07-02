/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensure the header is only included once
#ifndef BME680_hpp
#define BME680_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Library for interfacing with the BME680 sensor
#include <Zanshin_BME680.h>

// Library for standard integer types
#include <stdint.h>

/* ------------------ PUBLIC FUNCTIONS PROTOTYPES ------------------ */

// Structure to hold BME680 sensor data
typedef struct
{
    // Temperature in degrees Celsius, scaled by 100
    int32_t temp;

    // Relative humidity in percentage, scaled by 1000
    int32_t humidity;

    // Pressure in hPa, scaled by 1000
    int32_t pressure;

    // VOC index (calculated from gas resistance)
    int32_t vocIndex;

} t_dataBME680;

// Initializes the BME680 sensor
// @return: 1 if successful, 0 otherwise
int initBME680();

// Retrieves data from the BME680 sensor
// @param newData: Pointer to structure where data will be stored
void getDataBME680(t_dataBME680 *newData);

#endif // BME680_HPP
