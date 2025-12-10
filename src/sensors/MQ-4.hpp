/* *****************************************************************
 * HEADER CONFIGURATION MQ-4
 *************************************************************** */

#ifndef MQ4_hpp
#define MQ4_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

#include <stdint.h>
#include <Arduino.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// GPIO pin connected to the MQ-4 sensor
#define P_MQ4 33

/* ------------------- PUBLIC STRUCTURE TYPES ------------------- */

// Structure to store methane data
typedef struct {
    // Methane concentration in ppm
    int32_t methane;
} t_dataMQ4;

/* ------------------- CALIBRATION STRUCTURE ------------------- */

// Structure to store calibration conditions
typedef struct {
    float R0;           // Sensor resistance in clean air
    float T_cal;        // Temperature during calibration (°C)
    float HR_cal;       // Humidity during calibration (%RH)
    float P_cal;        // Pressure during calibration (hPa)
} t_calMQ4;

/* ------------------- PUBLIC FUNCTIONS PROTOTYPES ------------------- */

// Initializes the MQ-4 sensor
int initMQ4();

// Basic: Retrieves methane data without environmental compensation
// @param newData: Pointer to structure where the data will be stored
// @param adcRaw: Raw ADC value
// @param adcMax: Maximum ADC value (4095 for ESP32 12-bit)
void getDataMQ4_basic(t_dataMQ4 *newData, uint16_t adcRaw, uint16_t adcMax);

// Compensated: Retrieves methane data with T, HR, P compensation
// @param newData: Pointer to structure where data will be stored
// @param adcRaw: Raw ADC value
// @param adcMax: Maximum ADC value
// @param temperatureC: Temperature in Celsius from BME680
// @param humidityRH: Humidity in %RH from BME680
// @param pressure_hPa: Pressure in hPa from BME680
void getDataMQ4_compensated(t_dataMQ4 *newData,
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
void calibrateMQ4_R0_real(uint16_t cleanAirADCValue, uint16_t adcMax,
                          float temperatureC, float humidityRH, float pressure_hPa);

// Get the calibration data structure
// @return: Pointer to calibration data
t_calMQ4* getMQ4_CalibrationData();

#endif // MQ4_hpp
