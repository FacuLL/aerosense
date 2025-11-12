// Ensure the header is included only once
#ifndef PMS5003_hpp
#define PMS5003_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard integer types for portability
#include <stdint.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// UART configuration for the PMS5003 sensor
#define PMS5003_BAUD 9600
#define PMS5003_SERIAL_INDEX 2

// Pin definitions (can be overridden at build time)
#define P_PMS5003_RX 16
#define P_PMS5003_TX 17

#define P_PMS5003_RESET -1
#define P_PMS5003_SLEEP -1

/* ---------------------- DATA STRUCTURES ---------------------- */

// Structure to hold PMS5003 particulate data
typedef struct
{
    // PM1.0 concentration in micrograms per cubic meter
    int32_t pm1_0;

    // PM2.5 concentration in micrograms per cubic meter
    int32_t pm2_5;

    // PM10 concentration in micrograms per cubic meter
    int32_t pm10;

} t_dataPMS5003;

/* ---------------- PUBLIC FUNCTIONS PROTOTYPES ---------------- */

// Initializes the PMS5003 sensor
// @return: 1 if successful, 0 otherwise
int initPMS5003();

// Retrieves data from the PMS5003 sensor
// @param newData: Pointer to structure where data will be stored
void getDataPMS5003(t_dataPMS5003 *newData);

#endif // PMS5003_hpp
