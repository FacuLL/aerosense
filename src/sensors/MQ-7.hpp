/* *****************************************************************
 * HEADER CONFIGURATION MQ-7
 *************************************************************** */

#ifndef MQ7_hpp
#define MQ7_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

#include <stdint.h>
#include <Arduino.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// Pin connected to the MQ-7 sensor
#define P_MQ7 35

/* ------------------- PUBLIC STRUCTURE TYPES ------------------- */

// Structure to hold sensor data
typedef struct {
    // Carbon monoxide level in ppm
    int32_t carbonMonoxyde;
} t_dataMQ7;

/* ------------------- CALIBRATION STRUCTURE ------------------- */

// Structure to store calibration conditions
typedef struct {
    float R0;           // Sensor resistance in clean air
    float T_cal;        // Temperature during calibration (°C)
    float HR_cal;       // Humidity during calibration (%RH)
    float P_cal;        // Pressure during calibration (hPa)
} t_calMQ7;

/* ----------------- PUBLIC FUNCTIONS PROTOTYPES ----------------- */

// Initializes the MQ-7 sensor
// @return: 1 if successful
int initMQ7();

// Basic: Retrieves CO data without environmental compensation
// @param newData: Pointer to structure where the data will be stored
// @param adcRaw: Raw ADC value
// @param adcMax: Maximum ADC value (4095 for ESP32 12-bit)
void getDataMQ7_basic(t_dataMQ7 *newData, uint16_t adcRaw, uint16_t adcMax);

// Compensated: Retrieves CO data with T, HR, P compensation
// @param newData: Pointer to structure where data will be stored
// @param adcRaw: Raw ADC value
// @param adcMax: Maximum ADC value
// @param temperatureC: Temperature in Celsius from BME680
// @param humidityRH: Humidity in %RH from BME680
// @param pressure_hPa: Pressure in hPa from BME680
void getDataMQ7_compensated(t_dataMQ7 *newData,
                            uint16_t adcRaw, uint16_t adcMax,
                            float temperatureC,
                            float humidityRH,
                            float pressure_hPa);

// Calibrate R0 in real environmental conditions
// @param cleanAirADCValue: ADC reading in clean air
// @param adcMax: Maximum ADC value
// @param temperatureC: Temperature during calibration
// @param humidityRH: Humidity during calibration
// @param pressure_hPa: Pressure during calibration
void calibrateMQ7_R0_real(uint16_t cleanAirADCValue, uint16_t adcMax,
                          float temperatureC, float humidityRH, float pressure_hPa);

// Get the calibration data structure
// @return: Pointer to calibration data
t_calMQ7* getMQ7_CalibrationData();

#endif // MQ7_hpp
