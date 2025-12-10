/* *****************************************************************
 * HEADER CONFIGURATION MQ-131
 *************************************************************** */

#ifndef MQ131_hpp
#define MQ131_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

#include <stdint.h>
#include <Arduino.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// Pin for O3 and NO2 measurement
#define P_MQ131 34

/* ------------------- PUBLIC STRUCTURE TYPES ------------------- */

// Data structure for storing MQ-131 sensor data
typedef struct {
    // Ozone level (O3) in parts per million (ppm)
    int32_t ozone;
    // Nitrogen dioxide level (NO2) in ppm
    int32_t no2;
} t_dataMQ131;

/* ------------------- CALIBRATION STRUCTURE ------------------- */

// Structure to store calibration conditions
typedef struct {
    float R0;           // Sensor resistance in clean air
    float T_cal;        // Temperature during calibration (°C)
    float HR_cal;       // Humidity during calibration (%RH)
    float P_cal;        // Pressure during calibration (hPa)
} t_calMQ131;

/* ------------------- PUBLIC FUNCTIONS PROTOTYPES ------------------- */

// Initializes the MQ-131 sensor
// @return: 1 if successful, 0 otherwise
int initMQ131();

// Basic: Retrieves O3/NO2 data without environmental compensation
// @param newData: Pointer to structure where data will be stored
// @param adcRaw: Raw ADC value
// @param adcMax: Maximum ADC value (4095 for ESP32 12-bit)
void getDataMQ131_basic(t_dataMQ131 *newData, uint16_t adcRaw, uint16_t adcMax);

// Compensated: Retrieves O3/NO2 data with T, HR, P compensation
// @param newData: Pointer to structure where data will be stored
// @param adcRaw: Raw ADC value
// @param adcMax: Maximum ADC value
// @param temperatureC: Temperature in Celsius from BME680
// @param humidityRH: Humidity in %RH from BME680
// @param pressure_hPa: Pressure in hPa from BME680
void getDataMQ131_compensated(t_dataMQ131 *newData,
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
void calibrateMQ131_R0_real(uint16_t cleanAirADCValue, uint16_t adcMax,
                            float temperatureC, float humidityRH, float pressure_hPa);

// Get the calibration data structure
// @return: Pointer to calibration data
t_calMQ131* getMQ131_CalibrationData();

#endif // MQ131_hpp
